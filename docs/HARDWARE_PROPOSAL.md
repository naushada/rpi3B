# Proposal: Minimalistic Battery-Powered IoT Gateway + WiFi AP

A hardware/software proposal for a minimal, battery-powered platform that runs
the `device-iot` LwM2M gateway (the [`naushada/iot`](https://github.com/naushada/iot)
stack with the vendored `bcm2837` I²C driver from this repo) **and** acts as a
WiFi access point that re-shares an internet uplink to a connected client.

---

## 1. What the device must do

| #  | Requirement                                              | Implication |
|----|----------------------------------------------------------|-------------|
| R1 | Run `device-iot` (LwM2M gateway + vendored `bcm2837` I²C driver) | Linux-capable SoC, I²C exposed to sensors (mangOH Yellow), enough RAM/flash for a C++ userspace daemon |
| R2 | Battery powered                                          | Low idle draw, PMIC/charger, fuel gauge, safe shutdown |
| R3 | Custom Linux distro                                      | Buildroot/Yocto target with good mainline support |
| R4 | Built-in WiFi **AP**                                     | WiFi chip + driver supporting **AP mode** (`hostapd`) |
| R5 | Client joins the AP → reaches the **internet**          | The gateway needs an **upstream** path (cellular/second radio/Ethernet) + NAT/forwarding |

The critical, often-missed point in R5: an AP alone gives clients a *LAN*, not
the internet. The box must own a separate **WAN uplink** and route between the
two. That requirement dominates the hardware choice.

---

## 2. Topology

```
   [ Sensors ]                         Battery + PMIC
   I2C / mangOH ──┐                          │
                  ▼                          ▼
  Client ⇢ WiFi AP (wlan0, hostapd) ─►  [ device-iot SoC ]  ─► WAN uplink ─► Internet
   (phone/laptop)        10.0.0.0/24       Linux + nftables NAT      (LTE / 2nd radio / Eth)
```

Software glue (all mainline Linux, no custom kernel work):

- `hostapd` on `wlan0` → the AP (R4)
- `dnsmasq` → DHCP + DNS for clients
- Upstream `wwan0`/`eth0`/`wlan1` → WAN (R5)
- `nftables` masquerade + `net.ipv4.ip_forward=1` → the "route/forwarding rule"

---

## 3. The uplink decision (pick before the SoC)

"Access the internet" needs a WAN. On a battery device the cleanest options are:

| Uplink | How | Pros | Cons |
|--------|-----|------|------|
| **LTE/4G modem** (USB or mPCIe, e.g. Quectel EG25-G) | `wwan0`/PPP | Truly mobile, single self-contained box | +0.3–2 W, SIM/data plan, RF design, certification |
| **Second WiFi radio** (station mode) | `wlan1` joins existing WiFi, `wlan0` re-shares | Cheap, no SIM | Needs an existing WiFi to leech; two radios = more power/coexistence |
| **Ethernet/USB tether** | `eth0`/`usb0` | Simplest, lowest power | Defeats "battery/portable" except at a desk |

**Recommendation:** design for **LTE as primary**, keep a USB-Ethernet as a
fallback/bench option. A single WiFi chip that cannot do AP+STA simultaneously is
the most common failure here — if you go the "second WiFi" route, budget for
**two** radios.

---

## 4. Hardware options (minimal → most integrated)

### Option A — Raspberry Pi **Zero 2 W** (recommended starting point)

Quad-core Cortex-A53 (same BCM family as the `bcm2837` driver), onboard
AP-capable WiFi, I²C on the header.

- **Pros:** Reuses the BCM/I²C driver work almost verbatim; large mainline +
  community support; tiny; cheapest path to a prototype; Buildroot/Yocto BSP
  exists.
- **Cons:** Onboard WiFi AP+STA simultaneous is flaky → you'll want LTE or a 2nd
  USB-WiFi for the uplink; no PMIC/charger/fuel-gauge (you add those); single
  USB-OTG port (hub needed for modem + extras); weak deep-sleep story.

### Option B — Pi **3B** (this repo's current target)

- **Pros:** Exactly the silicon the driver targets; Ethernet onboard (easy WAN at
  the bench); most familiar.
- **Cons:** Power-hungry for a battery device (~2–3 W idle, higher spikes);
  physically large; same single-radio AP+STA limitation. Good as a **dev board**,
  poor as the shipped battery unit.

### Option C — **mangOH Yellow / WP76xx** (Sierra Wireless)

The repo already integrates mangOH Yellow sensors, so this is the "native" fit.

- **Pros:** **Integrated cellular modem** (solves R5 cleanly), designed for
  battery/IoT, built-in WiFi, sensors and Legato/Yocto BSP, certified RF module.
  Closest to a product, not a prototype.
- **Cons:** WP76 is a lower-power Cortex-A7-class part — tighter CPU/RAM budget;
  Sierra's Legato/Yocto toolchain is more specialized (steeper learning curve,
  smaller community); higher unit cost; the `bcm2837` MMIO driver does **not**
  apply (different SoC) — I²C would go through the WP's Linux `i2c-dev` instead.

### Option D — Other low-power Linux SoMs (NXP i.MX 7 / 8M-Mini, Allwinner H3/H5, RK3308)

- **Pros:** Built for low-power/battery; mainline Linux; spec exactly the
  RAM/flash/radios you need; PMIC usually integrated.
- **Cons:** Most board bring-up effort; you re-do the I²C/sensor layer for a new
  SoC; no Pi ecosystem to lean on.

---

## 5. Recommendation

- **Prototype now:** **Pi Zero 2 W** (Option A) + USB **LTE modem** for WAN + the
  mangOH/I²C sensors on the header. Fastest route that reuses the existing
  `bcm2837` driver and proves the AP→NAT→LTE path end-to-end.
- **Product path:** **mangOH Yellow / WP76** (Option C) — collapses modem, WiFi,
  sensors, and battery management into one certified board, at the cost of
  dropping the BCM driver and adopting Sierra's toolchain.

Decide the uplink (Section 3) first; it determines whether you need one radio or
two and dominates the power budget.

---

## 6. Minimal Bill of Materials (Option A prototype)

| Block       | Part (example)                          | Notes |
|-------------|-----------------------------------------|-------|
| Compute     | Pi Zero 2 W                             | I²C + WiFi AP |
| WAN         | Quectel EG25-G USB LTE                   | `wwan0`, needs SIM |
| Power       | LiPo 3.7 V (3000–5000 mAh) + TP4056 / PiSugar | Add fuel gauge (MAX17048) over I²C |
| Regulation  | 5 V boost (e.g. TPS61xx)                 | Clean rail for SoC + modem peaks |
| Sensors     | mangOH Yellow / I²C bus                  | Existing driver |
| Storage     | A-class microSD or eMMC                  | Custom distro image |

---

## 7. Custom distro (R3) — minimal stack

- **Builder:** Buildroot (smaller/faster to learn) or Yocto (better for
  productization).
- **Kernel:** mainline + `cfg80211`/`mac80211`, the AP-capable WiFi driver,
  USB-serial / `qmi_wwan` for the modem.
- **Userspace:** `hostapd`, `dnsmasq`, `nftables`, `ModemManager`/`qmicli` (or
  PPP), and the `device-iot` daemon as the LwM2M gateway. Ship the gtest suite as
  a boot self-test (as this repo already does).
- **Routing (R5), the "forwarding rule":**

  ```sh
  sysctl -w net.ipv4.ip_forward=1
  nft add table ip nat
  nft add chain ip nat post '{ type nat hook postrouting priority 100; }'
  nft add rule ip nat post oifname "wwan0" masquerade
  ```

### 7.1 `hostapd` config example (`/etc/hostapd/hostapd.conf`)

WPA2 AP on `wlan0`. Adjust `channel`/`hw_mode` for 2.4 GHz (`g`) vs 5 GHz (`a`).

```ini
interface=wlan0
driver=nl80211
ssid=device-iot
country_code=US

# 2.4 GHz, channel 6
hw_mode=g
channel=6
ieee80211n=1
wmm_enabled=1

# WPA2-PSK (CCMP)
auth_algs=1
wpa=2
wpa_key_mgmt=WPA-PSK
rsn_pairwise=CCMP
wpa_passphrase=change-me-please

# Hardening: isolate clients from each other, hide nothing else
ap_isolate=1
ignore_broadcast_ssid=0
```

Bring the interface up with a static gateway IP before starting `hostapd`:

```sh
ip addr add 10.0.0.1/24 dev wlan0
ip link set wlan0 up
hostapd -B /etc/hostapd/hostapd.conf
```

### 7.2 `dnsmasq` config example (`/etc/dnsmasq.conf`)

DHCP + DNS for the AP subnet only (do not serve the WAN side):

```ini
# Serve only the AP interface
interface=wlan0
bind-interfaces
except-interface=wwan0

# DHCP pool on the 10.0.0.0/24 AP subnet
dhcp-range=10.0.0.50,10.0.0.150,255.255.255.0,12h
dhcp-option=option:router,10.0.0.1
dhcp-option=option:dns-server,10.0.0.1

# Resolve upstream via the modem-provided servers in /etc/resolv.conf
domain-needed
bogus-priv
cache-size=1000
```

Start it after `wlan0` has its `10.0.0.1` address:

```sh
dnsmasq -C /etc/dnsmasq.conf
```

With `hostapd` (AP), `dnsmasq` (DHCP/DNS), and the `nft` masquerade rule above all
running, a client that joins SSID `device-iot` gets a `10.0.0.x` lease and reaches
the internet through the `wwan0` uplink.

---

## 8. Power budget & battery life (rough)

| State                       | Pi Zero 2 W | + LTE      | Total      |
|-----------------------------|-------------|------------|------------|
| Idle, AP up, no client      | ~0.7 W      | ~0.3 W     | ~1.0 W     |
| Active client + LTE TX      | ~1.2 W      | ~1.5 W peak| ~2–3 W peak|

A 5000 mAh / 3.7 V (~18.5 Wh) pack ⇒ **~8–12 h** typical, less under sustained
LTE traffic. Add a low-power MCU or RTC wake if you need multi-day standby — the
Pi family does not deep-sleep well, which is the main argument for Option C/D in a
real product.

---

## 9. Memory budget (RAM + storage)

### 9.1 RAM (resident, steady state)

| Component                          | Typical RSS | Notes |
|------------------------------------|-------------|-------|
| Linux kernel + base userspace      | 30–60 MB    | Buildroot minimal, BusyBox init, no desktop |
| `hostapd` (AP)                     | 2–5 MB      | Single SSID |
| `dnsmasq` (DHCP + DNS)             | 2–4 MB      | A handful of leases |
| `nftables` (NAT/forwarding)        | <1 MB       | Rules live in-kernel; userspace tool only at config |
| Modem mgmt (`ModemManager`/`qmicli`) | 5–25 MB   | `ModemManager` is heavy; raw `qmicli` + a script is far lighter |
| `device-iot` daemon (C++ LwM2M)    | 10–30 MB    | Depends on object/instance count + TLS (mbedTLS/OpenSSL) buffers |
| Buffers/cache + headroom           | ~50 MB      | Page cache, network buffers, fragmentation slack |
| **Working set total**              | **~110–175 MB** | |

- **256 MB RAM is workable**, **512 MB is comfortable** and the value pick.
- Pi Zero 2 W ships **512 MB** → fits with room for page cache and traffic spikes.
- A WP76-class part (typically 256 MB) fits the **minimal** profile only if you
  drop `ModemManager` for `qmicli`, keep TLS buffers modest, and avoid running the
  gtest self-test concurrently with the live workload.
- Add **swap/zram (~128 MB)** to absorb peaks and the boot-time gtest run cleanly.

### 9.2 Storage (flash / rootfs)

| Component                              | Size      |
|----------------------------------------|-----------|
| Kernel + DTB + modules                 | 10–25 MB  |
| Buildroot rootfs (BusyBox + libs)      | 20–50 MB  |
| `hostapd`/`dnsmasq`/`nftables`/modem tools | 5–10 MB |
| `device-iot` binary + gtest + deps     | 5–20 MB   |
| Config, logs, persistent LwM2M state   | 5–20 MB   |
| **Image total (minimal)**              | **~50–125 MB** |

- A **256 MB–512 MB** SD/eMMC holds the image with margin; **1–4 GB** gives ample
  space for A/B update slots and log retention.
- Prefer an **A/B (dual-bank) layout** for safe OTA updates if this becomes a
  product — roughly doubles the rootfs footprint but still fits a 1 GB device.
- Use a read-only rootfs + small read-write overlay (`overlayfs`) to protect flash
  on a battery device that may lose power abruptly.

**Bottom line:** target **512 MB RAM** and a **≥512 MB (ideally 1 GB)** rootfs.
The Pi Zero 2 W meets this out of the box; constrained parts (WP76) require the
lean software choices noted above.

---

## 10. Global pros & cons of this architecture

**Pros**

- Reuses the existing `bcm2837` I²C/sensor driver investment (Options A/B).
- All routing/AP/DHCP done with stock, well-understood Linux daemons — no custom
  networking code.
- Clear prototype→product migration path (Zero 2 W → mangOH).
- Single box: sensor gateway **and** internet-sharing AP.

**Cons / risks**

- **Single-radio AP+STA** is the #1 trap — budget for LTE or a second radio up
  front.
- Pi-class SoCs have a **weak battery/sleep story**; real battery life is hours,
  not days.
- LTE uplink adds **RF design, SIM/data-plan, and certification** overhead.
- Acting as an open AP-to-internet router is a **security surface** (firewall
  rules, client isolation, captive concerns) you must own.
- The product-grade choice (mangOH/WP76) **abandons the BCM driver** and adopts a
  niche toolchain.
