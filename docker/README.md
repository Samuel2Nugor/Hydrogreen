# MicroHydros Docker Backend

This Docker Compose environment runs the backend services required by the MicroHydros prototype.

## Services

| Service   | Responsibility                              | Local address           | Docker address         |
| --------- | ------------------------------------------- | ----------------------- | ---------------------- |
| Mosquitto | MQTT message broker                         | `localhost:1883`        | `mosquitto:1883`       |
| Node-RED  | Receive, validate and process telemetry     | `http://localhost:1880` | `http://node-red:1880` |
| InfluxDB  | Store historical measurements               | `http://localhost:8086` | `http://influxdb:8086` |
| Grafana   | Display current and historical measurements | `http://localhost:3000` | `http://grafana:3000`  |

Applications running directly on the laptop use `localhost`. Containers communicate using their Docker service names. Docker-assigned IP addresses must not be placed in configuration because they can change.

## Requirements

* Docker Engine
* Docker Compose
* Available ports `1883`, `1880`, `8086` and `3000`

## Tested container versions

| Service | Docker image |
| ------- | ------------ |
| Mosquitto | `eclipse-mosquitto:2` |
| Node-RED | `nodered/node-red:5.0.7` |
| InfluxDB | `influxdb:2.9.1` |
| Grafana | `grafana/grafana:13.2.1` |

Node-RED, InfluxDB and Grafana use exact tested versions. Mosquitto uses the supported major-version tag because the running program version does not have a matching Docker image tag.

## Local environment configuration

Create a private `.env` file from the committed template:

```bash
cp .env.example .env
vim .env
```

Replace the example passwords and token with private local values.

```dotenv
INFLUXDB_USERNAME=microhydros
INFLUXDB_PASSWORD=replace-with-a-private-password
INFLUXDB_ORG=microhydros
INFLUXDB_BUCKET=microhydros
INFLUXDB_TOKEN=replace-with-a-long-random-token

GRAFANA_USERNAME=admin
GRAFANA_PASSWORD=replace-with-a-private-password
```

InfluxDB passwords must contain between 8 and 72 characters.

A random InfluxDB token can be generated with:

```bash
openssl rand -hex 32
```

The `.env` file contains secrets and must not be committed. Only `.env.example` belongs in Git.

Team members should keep these shared names:

```text
Organisation: microhydros
Bucket: microhydros
```

Passwords and tokens may be different in each local development environment.

## Start the backend

Run from the repository root:

```bash
docker compose up -d
```

Docker downloads missing images, creates the internal network and persistent volumes, and starts all four services.

## Check service status

```bash
docker compose ps
```

The expected containers are:

```text
microhydros-mosquitto
microhydros-node-red
microhydros-influxdb
microhydros-grafana
```

All containers should show an `Up` status.

## Open the web interfaces

Open these addresses in a browser:

* Node-RED: `http://localhost:1880`
* InfluxDB: `http://localhost:8086`
* Grafana: `http://localhost:3000`

Use the credentials defined in the local `.env` file when signing in to InfluxDB or Grafana.

## Verify service health

### InfluxDB

```bash
curl -fsS http://localhost:8086/health
```

InfluxDB should report a passing status.

### Grafana

```bash
curl -fsS http://localhost:3000/api/health
```

Grafana should report that its database is `ok`.

### Node-RED

```bash
curl -s -o /dev/null -w '%{http_code}\n' http://localhost:1880
```

Node-RED should return:

```text
200
```

## Verify internal Docker networking

Node-RED must be able to resolve Mosquitto and InfluxDB using their service names:

```bash
docker exec microhydros-node-red getent hosts mosquitto influxdb
```

The command should return Docker-network addresses for both services.

Configuration inside Node-RED must use:

```text
MQTT broker: mosquitto
MQTT port: 1883
InfluxDB URL: http://influxdb:8086
```

Do not use `localhost` inside Node-RED. Inside its container, `localhost` refers to Node-RED itself.

## Test Mosquitto

Open one terminal and subscribe:

```bash
docker exec -it microhydros-mosquitto \
  mosquitto_sub -h localhost -t "microhydros/#" -v
```

Keep the terminal open.

In another terminal, publish a test message:

```bash
docker exec microhydros-mosquitto \
  mosquitto_pub -h localhost \
  -t microhydros/test \
  -m '{"sensor_id":"inside_air","measurement":"temperature","value":23.5,"unit":"C"}'
```

The subscriber should display the topic and message.

The `microhydros/test` topic is only a basic broker test. The telemetry simulator and ESP32-S3 must use the topics defined in `docs/data-contract.md`.

## View logs

View logs from every service:

```bash
docker compose logs
```

Follow logs continuously:

```bash
docker compose logs -f
```

View one service:

```bash
docker compose logs --tail=100 node-red
docker compose logs --tail=100 influxdb
docker compose logs --tail=100 grafana
docker compose logs --tail=100 mosquitto
```

Press `Ctrl+C` to stop following logs. This does not stop the containers.

## Restart services

Restart the complete backend:

```bash
docker compose restart
```

Restart one service:

```bash
docker compose restart node-red
```

Recreate a service after changing its Compose environment configuration:

```bash
docker compose up -d --force-recreate influxdb
```

## Stop the backend

Stop and remove the containers and network:

```bash
docker compose down
```

Named volumes are retained, so Node-RED flows, InfluxDB measurements and Grafana configuration remain available the next time the environment starts.

## Persistent volumes

| Volume            | Stored data                          |
| ----------------- | ------------------------------------ |
| `mosquitto_data`  | Persistent Mosquitto data            |
| `mosquitto_log`   | Mosquitto logs                       |
| `node_red_data`   | Node-RED flows and configuration     |
| `influxdb_data`   | InfluxDB measurements                |
| `influxdb_config` | InfluxDB configuration               |
| `grafana_data`    | Grafana dashboards and configuration |

Do not use `docker compose down -v` during normal development. The `-v` option deletes these volumes and therefore removes stored measurements, flows and dashboards.

## Initial InfluxDB setup

InfluxDB uses the `.env` values only when its persistent volume is initialized for the first time.

Changing the initialization values later does not automatically change an existing InfluxDB account, password, organisation, bucket or token.

During the initial setup, InfluxDB will fail to start if its password is shorter than 8 characters. Check the error with:

```bash
docker compose logs --tail=100 influxdb
```

## Development responsibilities

This repository provides the shared backend infrastructure. Team members can extend it through separate branches and pull requests.

Expected additions include:

* Node-RED validation and storage flows (exported to `docker/node-red/flows/` and version-controlled; both must be imported into Node-RED at `http://localhost:1880` for the full telemetry pipeline)
* InfluxDB measurement structure
* Grafana data-source provisioning
* Grafana dashboard provisioning
* Telemetry simulator
* Integration and failure tests

Changes must follow the approved architecture and `docs/data-contract.md`.

## Current limitations

* MQTT anonymous access is enabled for local development.
* MQTT authentication and TLS are not configured.
* Node-RED editor authentication is not configured.
* The Docker setup is intended for a controlled development or demonstration network.
* Secrets and tokens must never be committed.

These limitations must be reviewed before exposing the system outside a controlled local network.
