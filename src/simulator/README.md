# Telemetry simulator

Publishes fake sensor readings over MQTT so the rest of the pipeline
(Mosquitto → Node-RED → InfluxDB) can be built and tested before the
ESP32-S3 + SHT31 + DS18B20 hardware is wired up. Once real firmware
exists, this becomes a stand-in for testing/CI rather than the primary
data source.

Implements the 3 measurements from `docs/project-plan.md` as 4 values
(SHT31 gives both temperature and humidity from one internal reading):

| Field                | Sensor (planned)      | Unit  |
|-----------------------|------------------------|-------|
| `temp_internal`       | SHT31                 | °C    |
| `humidity_internal`   | SHT31                 | %RH   |
| `temp_external`       | *not yet chosen*      | °C    |
| `temp_water`          | DS18B20               | °C    |

**`temp_external`'s sensor isn't decided in `docs/technical-solutions.md` yet** — the simulator just generates a plausible outdoor value. Worth resolving before hardware work starts.

## Quick start

1. Start a local broker:

   ```bash
   cd infra/mosquitto
   docker compose up -d
   ```

2. Install and run the simulator:

   ```bash
   cd src/simulator
   python -m venv .venv && source .venv/bin/activate   # Windows: .venv\Scripts\activate
   pip install -r requirements.txt
   python simulate.py -v
   ```

3. Watch it (from any machine with `mosquitto_clients` installed, or Node-RED's MQTT-in node):

   ```bash
   mosquitto_sub -h localhost -t 'microhydros/#' -v
   ```

Stop the simulator with Ctrl+C. Stop the broker with `docker compose down` (add `-v` to also drop its persisted retained state).

## Data shape (proposal, not yet ratified)

`docs/data-contract.md` is still a placeholder — this is what the simulator emits today, meant as a starting point for the team to review, not a finished spec.

**Topic:** `microhydros/<device_id>/telemetry`

**Payload:** one JSON object per read cycle, all 4 values together:

```json
{
  "temp_internal": 24.54,
  "humidity_internal": 67.81,
  "temp_external": 14.94,
  "temp_water": 20.19,
  "unit": {"temp_internal": "C", "humidity_internal": "%RH", "temp_external": "C", "temp_water": "C"},
  "ts": "2026-09-09T08:01:18Z",
  "device_id": "microhydros-sim-01"
}
```

**Why one topic/message per cycle instead of 4 separate topics:** all 4 values come from one read of the physical unit at the same instant, and Node-RED → InfluxDB naturally wants that as a single point with 4 fields at one timestamp, not 4 separate writes to reconcile. If the team decides per-metric topics fit the architecture better (e.g. to let Node-RED subscribe to individual sensors), that's a straightforward change to `make_reading()` — flag it in `docs/decision-log.md` either way once decided.

## What it simulates, and why

- **Slow drift + sensor noise per metric** — real sensors don't return a flat line; each value random-walks around a baseline with independent noise on top.
- **A daily cycle on `temp_external`** (and a smaller one on the internal values) — outdoor temperature isn't flat over 24h, and greenhouses feel some of that.
- **Occasional equipment-driven spikes** — `docs/project-plan.md` explicitly calls out that "supporting electrical equipment can quickly change the temperature and humidity" and that unsuitable conditions need to be caught before they cause harm. The simulator randomly triggers short internal temp-up/humidity-down events so the pipeline actually has something realistic to detect, instead of only ever seeing calm data.
- **Optional malformed payloads** (`--inject-anomalies`) — dropped fields, out-of-range values, wrong types, nulls. Off by default; turn it on to test that Node-RED/InfluxDB (or whatever validates the data) actually rejects or flags bad input instead of silently accepting it.

## CLI options

```
python simulate.py [options]

  --broker-host HOST       default: localhost
  --broker-port PORT       default: 1883
  --device-id ID           default: microhydros-sim-01
  --interval SECONDS       default: 10.0 (seconds between reads)
  --qos {0,1,2}            default: 1
  --count N                default: 0 (run forever; N = stop after N reads)
  --inject-anomalies P     default: 0.0 (probability 0-1 per reading of a malformed payload)
  --seed N                 fix the RNG for a reproducible run
  -v, --verbose
```

Example — fast run for a demo, with occasional bad data to show validation working:

```bash
python simulate.py --interval 2 --inject-anomalies 0.1 -v
```

## Tested

Verified locally against a real Mosquitto broker (native binary and via this `docker-compose.yml`): connects, publishes at the configured interval, `mosquitto_sub` receives well-formed JSON on the expected topic, `--inject-anomalies` produces each of the 4 malformed shapes, and a bad broker address fails with a clear error instead of hanging silently.
