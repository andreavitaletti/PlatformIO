# ESP32 BLE → Phone Gateway → Server / MQTT Broker

A smartphone acts as a mobile gateway for an ESP32 sensor node: it receives
readings over BLE (Web Bluetooth), enriches them with phone-side data, and
forwards them upstream. Two gateway flavours are provided:

| Gateway | PlatformIO env | Firmware | Web app | Uplink | Phone enrichment |
|---------|----------------|----------|---------|--------|------------------|
| **HTTP** | `blegwweb`  | `src/main-blegwweb.cpp`  | `web/gatewayweb.html`  | HTTP POST (`fetch`) to any endpoint | GPS + phone timestamp |
| **MQTT** | `blegwmqtt` | `src/main-blegwmqtt.cpp` | `web/gatewaymqtt.html` | MQTT over WebSocket (MQTT.js) | Accelerometer + receive timestamp |

Each web app is paired with its own firmware: the BLE UUIDs and JSON format
differ, so **flash the firmware that matches the page you open on the phone**.

## Architecture

### HTTP gateway

```
┌─────────────────┐        BLE Notify        ┌──────────────────────┐
│   ESP32 Node    │ ────────────────────────► │   Chrome / Android   │
│                 │                           │   (gatewayweb.html)  │
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

### MQTT gateway

```
┌─────────────────┐        BLE Notify        ┌──────────────────────┐
│   ESP32 Node    │ ────────────────────────► │   Chrome / Android   │
│                 │  JSON: {id, seq, temp,    │   (gatewaymqtt.html) │
│  Sensor stubs   │   humidity, uptime_ms}    │                      │
│  BLE GATT       │                           │  + accelerometer     │
│  server         │                           │  + receive timestamp │
└─────────────────┘                           └──────────┬───────────┘
                                                         │
                                       MQTT over WSS     │  topic: data/enriched
                                       (MQTT.js, QoS 1)  │
                                                         ▼
                                              ┌──────────────────────┐
                                              │   MQTT Broker        │
                                              │   broker.emqx.io     │
                                              └──────────┬───────────┘
                                                         │
                                                         ▼
                                                  any subscriber
```

## Files

| File | Description |
|------|-------------|
| `src/main-blegwweb.cpp`  | ESP32 firmware for the HTTP gateway (sensor + command characteristics) |
| `src/main-blegwmqtt.cpp` | ESP32 firmware for the MQTT gateway (single notify characteristic) |
| `src/main-remotexy.cpp`  | RemoteXY example (unrelated to the gateways) |
| `web/gatewayweb.html`    | Self-contained phone web app — HTTP gateway |
| `web/gatewaymqtt.html`   | Self-contained phone web app — MQTT gateway |
| `platformio.ini`         | One environment per `src/main-<env>.cpp` |

## ESP32 Setup

The project uses PlatformIO; `build_src_filter` compiles only
`src/main-${PIOENV}.cpp`, so pick the environment that matches the gateway:

```bash
pio run -e blegwweb  -t upload -t monitor   # HTTP gateway firmware
pio run -e blegwmqtt -t upload -t monitor   # MQTT gateway firmware
```

Replace the sensor stubs with your real sensor code (DHT22, BME280…):
`readSensors()` in `main-blegwweb.cpp`, `readTemperature()` / `readHumidity()`
in `main-blegwmqtt.cpp`.

No extra libraries needed — BLE support ships with the ESP32 Arduino core.

## Web App Setup

Both pages are served the same way.

### Option A — local file (simplest)
Copy `gatewayweb.html` or `gatewaymqtt.html` to your phone and open it in Chrome.
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

Then open `https://<your-pc-ip>:8443/gatewayweb.html` (or `gatewaymqtt.html`) on your phone.

### Option C — GitHub Pages / Netlify (zero-config HTTPS)
Just push the `web/` folder to a repo and enable Pages — instant HTTPS, no server needed.

## Using the HTTP gateway

1. Flash `blegwweb`.
2. Open `gatewayweb.html`, connect to the ESP32 over BLE and start GPS.
3. Enter the server URL and send manually or enable auto-send.

Payload sent with `POST` (`Content-Type: application/json`):

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

## Using the MQTT gateway

1. Flash `blegwmqtt`.
2. Open `gatewaymqtt.html` and press, in order:
   1. **Connect to ESP32 (BLE)**
   2. **Enable smartphone accelerometer**
   3. **Connect to MQTT broker**
   4. **Start forwarding**
3. Verify from any machine:

   ```bash
   mosquitto_sub -h broker.emqx.io -p 1883 -t "data/enriched" -v
   ```

   or use the MQTT web client embedded at the bottom of the page.

Broker and topic are set at the top of the page script:

```js
const MQTT_URL   = "wss://broker.emqx.io:8084/mqtt";
const MQTT_TOPIC = "data/enriched";
```

The browser can only speak MQTT over WebSockets, so the broker must expose a
`ws://`/`wss://` listener (use `wss://` when the page is served over HTTPS).
`broker.emqx.io` is a public broker: anyone can read the topic, so do not
publish sensitive data there.

Payload published on `data/enriched` (QoS 1, not retained), one per BLE notification:

```json
{
  "device_id": "esp32-001",
  "sequence": 42,
  "esp32_uptime_ms": 215034,
  "esp32_timestamp_received_ms": 1779201121123,
  "sensors": {
    "temperature": 23.5,
    "humidity": 55.0
  },
  "smartphone": {
    "accelerometer": {
      "x": 0.12,
      "y": 9.78,
      "z": 0.31,
      "timestamp_ms": 1779201121090
    }
  }
}
```

`accelerometer` is `null` until the accelerometer has been enabled.

## UUIDs

The UUIDs in each firmware must match its web app:

| Gateway | Service | Characteristics |
|---------|---------|-----------------|
| HTTP | `…-1234567890ab` | sensor `…-1234567890ac` (notify), command `…-1234567890ad` (write) |
| MQTT | `…-123456789001` | sensor `…-123456789002` (read + notify) |

Generate new ones at https://www.uuidgenerator.net/ if deploying multiple nodes.

## Limitations

| Constraint | Detail |
|------------|--------|
| Browser support | Chrome on Android only (no iOS Safari, no Firefox) |
| HTTPS required | When serving remotely; `file://` and `localhost` are exempt |
| BLE MTU | Default 20 bytes; ESP32 negotiates up to 512 (MQTT firmware requests 185) — JSON payload must fit |
| Background tab | Chrome may throttle/suspend the page if minimised — keep screen on |
| One client | A BLE peripheral can only connect to one central at a time |
| MQTT ports | Some networks (e.g. **eduroam**) block the broker's WebSocket port — use mobile data or another network |
| No buffering | Both gateways drop readings while the uplink is down |

## Extending

- **Multiple sensors**: add more GATT characteristics (one UUID per sensor type)
- **Buffering**: store readings in `localStorage` when offline, flush on reconnect
- **Auth (HTTP)**: add an `Authorization: Bearer <token>` header in the `fetch()` call
- **Auth (MQTT)**: pass `username` / `password` in the `mqtt.connect()` options and use a private broker
- **Commands (HTTP firmware)**: use `sendCommand('INTERVAL:10000')` to reconfigure the ESP32 remotely
- **Both uplinks**: the two pages share the same BLE → enrich → forward structure, so `publishMQTT()` and `sendNow()` can be combined in a single page
