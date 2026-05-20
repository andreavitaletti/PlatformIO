# ESP32 BLE → Web Bluetooth → Server Gateway

## Architecture

```
┌─────────────────┐        BLE Notify        ┌──────────────────────┐
│   ESP32 Node    │ ────────────────────────► │   Chrome / Android   │
│                 │                           │   (gateway.html)     │
│  DHT22 sensor   │   JSON: {t, h, ms}        │                      │
│  BLE GATT       │                           │  + GPS coords        │
│  server         │ ◄──────────────────────── │  + phone timestamp   │
│                 │   optional CMD writes      │                      │
└─────────────────┘                           └──────────┬───────────┘
                                                         │
                                              HTTP POST  │  JSON: {sensor, gps,
                                             (fetch API) │   phone_ts, device_id}
                                                         ▼
                                              ┌──────────────────────┐
                                              │   Your Server        │
                                              │   (any HTTP endpoint)│
                                              └──────────────────────┘
```

## Files

| File | Description |
|------|-------------|
| `esp32/esp32_ble_sensor.ino` | Arduino sketch for the ESP32 |
| `web/gateway.html`           | Self-contained phone web app |

## ESP32 Setup

1. Open `esp32_ble_sensor.ino` in Arduino IDE
2. Install board: **ESP32 by Espressif** via Boards Manager
3. Replace the `readSensors()` stub with your real sensor code (DHT22, BME280…)
4. Flash to your board

No extra libraries needed — BLE support ships with the ESP32 Arduino core.

## Web App Setup

### Option A — local file (simplest)
Copy `gateway.html` to your phone and open it in Chrome.
Web Bluetooth works from `file://` on Android.

### Option B — HTTPS server (recommended for production)
Web Bluetooth **requires HTTPS** (or localhost) when served remotely.

```bash
# Python quick server with self-signed cert
openssl req -x509 -newkey rsa:2048 -keyout key.pem -out cert.pem -days 365 -nodes -subj "/CN=localhost"
python3 -c "
import ssl, http.server
ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
ctx.load_cert_chain('cert.pem','key.pem')
srv = http.server.HTTPServer(('0.0.0.0', 8443), http.server.SimpleHTTPRequestHandler)
srv.socket = ctx.wrap_socket(srv.socket)
srv.serve_forever()
"
```

Then open `https://<your-pc-ip>:8443/gateway.html` on your phone.

### Option C — GitHub Pages / Netlify (zero-config HTTPS)
Just push `gateway.html` to a repo and enable Pages — instant HTTPS, no server needed.

## JSON Payload sent to server

```json
{
  "device_id": "CSS-SensorNode",
  "phone_ts":  "2026-05-19T14:32:01.123Z",
  "sensor": {
    "t":  -18.24,
    "h":  14.87,
    "ms": 123456
  },
  "gps": {
    "lat": 41.90278,
    "lon": 12.49636,
    "acc": 12.5
  }
}
```

## UUIDs

The UUIDs in the `.ino` and `gateway.html` must match.
Generate new ones at https://www.uuidgenerator.net/ if deploying multiple nodes.

## Limitations

| Constraint | Detail |
|------------|--------|
| Browser support | Chrome on Android only (no iOS Safari, no Firefox) |
| HTTPS required | When serving remotely; `file://` and `localhost` are exempt |
| BLE MTU | Default 20 bytes; ESP32 negotiates up to 512 — JSON payload must fit |
| Background tab | Chrome may throttle/suspend the page if minimised — keep screen on |
| One client | A BLE peripheral can only connect to one central at a time |

## Extending

- **Multiple sensors**: add more GATT characteristics (one UUID per sensor type)
- **Buffering**: store readings in `localStorage` when offline, flush on reconnect
- **Auth**: add an `Authorization: Bearer <token>` header in the `fetch()` call
- **Commands**: use `sendCommand('INTERVAL:10000')` to reconfigure the ESP32 remotely
