# Hydrogreen

## Utveckling av en IoT- och embeddedprototyp

MicroHydros is an IoT prototype that monitors a small growing environment (e.g. hydroponics). An ESP32-S3 sensor node reads internal air temperature/humidity (SHT31) and water and external air temperature (two DS18B20 probes), and publishes one raw MQTT telemetry message every 5 seconds. A Docker-based backend (Mosquitto, Node-RED, InfluxDB, Grafana) validates, stores and displays the measurements. See [`docs/architecture.md`](docs/architecture.md) for the full system design and [`docs/data-contract.md`](docs/data-contract.md) for the exact MQTT/JSON contract.

## Repository structure

| Path | Purpose |
|------|---------|
| `docs/` | Project planning and technical documentation |
| `firmware/` | ESP32-S3 firmware (ESP-IDF, C) |
| `hardware/` | Bill of materials, pinout and wiring documentation |
| `docker/` | Backend services (Mosquitto, Node-RED, InfluxDB, Grafana) via Docker Compose |
| `src/` | Source code for the prototype (e.g. the telemetry simulator) |
| `tests/` | Tests and supporting test code — see [`docs/test-plan.md`](docs/test-plan.md) |
| `data/` | Small sample measurements and example data formats |
| `CONTRIBUTING.md` | Rules for branches, commits, reviews and pull requests |

The structure may change as the system architecture becomes clearer.

## Dependencies

* Docker Engine + Docker Compose — runs the backend (see [`docker/README.md`](docker/README.md))
* [ESP-IDF v5.1+](https://docs.espressif.com/projects/esp-idf/en/v5.1.2/esp32s3/get-started/index.html) — builds and flashes the firmware (see [`firmware/esp32s3/README.md`](firmware/esp32s3/README.md))
* Python 3 — for the telemetry simulator (see [`src/simulator/README.md`](src/simulator/README.md)) and tooling bundled with ESP-IDF

## Quick start

1. **Start the backend.** `cd docker && docker compose up -d` — see [`docker/README.md`](docker/README.md) for `.env` setup and health checks.
2. **Get telemetry flowing**, either:
   * Run the simulator (no hardware needed): see [`src/simulator/README.md`](src/simulator/README.md), or
   * Flash the real ESP32-S3: see [`hardware/esp32s3/README.md`](hardware/esp32s3/README.md) for wiring and [`firmware/esp32s3/README.md`](firmware/esp32s3/README.md) for build/flash/menuconfig.
3. **Import the Node-RED flows** (`docker/node-red/flows/`) if not already provisioned — see [`docker/README.md`](docker/README.md).
4. **Check it's working:**
   * Node-RED editor: `http://localhost:1880` (or the deployment's mapped port)
   * Grafana dashboard: `http://localhost:3000` — shows current and historical measurements once valid data has been written to InfluxDB
   * Raw MQTT: subscribe to `microhydros/v1/devices/+/telemetry/raw` to see incoming device messages; `.../telemetry/validated/+` for accepted measurements and `.../telemetry/rejected` for anything Node-RED rejected (see [`docs/architecture.md`](docs/architecture.md) § MQTT topics)

For the team's demonstration deployment (Hetzner, not a laptop), see [`docs/architecture.md`](docs/architecture.md) § Demonstration environment for the actual addresses and ports.
