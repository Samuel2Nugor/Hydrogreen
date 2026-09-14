# Node-RED InfluxDB Storage Flow

This flow stores validated MicroHydros measurements in InfluxDB. It follows the validated measurement structure defined in `docs/data-contract.md`.

## Flow file

```text
docker/node-red/flows/influxdb-storage.json
```

The exported flow contains:

```text
Test injection
    → Create four test measurements
    → Build InfluxDB write
    → InfluxDB HTTP write
    → Debug response
```

The test nodes demonstrate the storage path without requiring sensors, ESP32 firmware or the complete MQTT validation flow.

## Requirements

Start the Docker backend from the repository root:

```bash
docker compose up -d
```

Confirm that Node-RED and InfluxDB are running:

```bash
docker compose ps node-red influxdb
```

Node-RED receives these settings from the local `.env` file through Docker Compose:

```text
INFLUXDB_URL
INFLUXDB_ORG
INFLUXDB_BUCKET
INFLUXDB_TOKEN
```

Real secret values must not be stored in this flow or committed to Git.

## Import the flow

1. Open Node-RED at `http://localhost:1880`.
2. Open the menu in the upper-right corner.
3. Select **Import**.
4. Select `docker/node-red/flows/influxdb-storage.json`.
5. Import the flow into a new tab.
6. Click **Deploy**.

## Storage schema

All validated environmental readings are stored under one InfluxDB measurement:

```text
sensor_reading
```

### Tags

| Tag                | Source                  |
| ------------------ | ----------------------- |
| `device_id`        | Validated `device_id`   |
| `measurement_type` | Validated `measurement` |
| `sensor_id`        | Validated `sensor_id`   |
| `unit`             | Validated `unit`        |

Tags allow measurements to be filtered by device, measurement type, sensor and unit.

### Fields

| Field            | Type    | Source                     |
| ---------------- | ------- | -------------------------- |
| `value`          | Float   | Validated `value`          |
| `schema_version` | Integer | Validated `schema_version` |
| `sequence`       | Integer | Validated `sequence`       |
| `uptime_ms`      | Integer | Validated `uptime_ms`      |
| `boot_id`        | String  | Validated `boot_id`        |

### Timestamp

The InfluxDB point time is taken from the validated `timestamp` assigned by Node-RED. The HTTP write uses millisecond precision.

## Supported measurement types

| Measurement type       | Sensor           | Unit         |
| ---------------------- | ---------------- | ------------ |
| `internal_temperature` | `internal_sht31` | `celsius`    |
| `internal_humidity`    | `internal_sht31` | `percent_rh` |
| `external_temperature` | `external_sht31` | `celsius`    |
| `water_temperature`    | `water_ds18b20`  | `celsius`    |

## Test the storage flow

Press the button on the test Inject node once.

The flow sends one example of each supported measurement type. The Debug panel should show four HTTP responses with:

```text
statusCode: 204
```

HTTP status `204` means InfluxDB accepted the measurement.

## Query stored values

Run from the repository root:

```bash
docker compose exec influxdb sh -c '
influx query \
"from(bucket: \"${DOCKER_INFLUXDB_INIT_BUCKET}\")
  |> range(start: -15m)
  |> filter(fn: (r) => r._measurement == \"sensor_reading\")
  |> filter(fn: (r) => r._field == \"value\")
  |> keep(columns: [\"_time\", \"_value\", \"device_id\", \"measurement_type\", \"sensor_id\", \"unit\"])" \
--org "$DOCKER_INFLUXDB_INIT_ORG" \
--token "$DOCKER_INFLUXDB_INIT_ADMIN_TOKEN"
'
```

The result should contain all four measurement types and their timestamps.

Increase the range, for example to `-24h`, when querying older test data.

## Integration with validated MQTT data

The test-data nodes are only for independent development.

The included MQTT input subscribes to `microhydros/v1/devices/+/telemetry/validated/+` using the Mosquitto service at `mosquitto:1883`. Rejected topics are not subscribed to and therefore cannot enter the normal `sensor_reading` measurement.

## Current limitations

* The flow currently uses the InfluxDB administrator token supplied through `.env`.
* A scoped write-only token should replace the administrator token in a later security improvement.
* The included Inject and test-generation nodes are development tools.
* The storage flow currently accepts only the four measurement types defined in the contract.

