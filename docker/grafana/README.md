# MicroHydros Grafana Dashboard

This dashboard displays current and historical MicroHydros measurements stored in InfluxDB.

## Dashboard file

```text
docker/grafana/dashboards/microhydros-measurements.json
```

## Included measurements

The dashboard contains current-value and historical-trend panels for:

* Internal temperature
* Internal humidity
* External temperature
* Water temperature

## Start the backend

Run from the repository root:

```bash
docker compose up -d
```

Confirm that Grafana and InfluxDB are running:

```bash
docker compose ps grafana influxdb
```

Open Grafana at:

```text
http://localhost:3000
```

## Configure the InfluxDB data source

Create an InfluxDB data source in Grafana using:

```text
Name: MicroHydros InfluxDB
Query language: Flux
URL: http://influxdb:8086
```

Use the organization, token and bucket values from the local `.env` file. Secret values must not be committed to Git.

Select **Save & test**. Grafana should confirm that the data source is working.

## Import the dashboard

1. Open Grafana.
2. Select **Dashboards**.
3. Select **New** and then **Import**.
4. Upload `docker/grafana/dashboards/microhydros-measurements.json`.
5. Select the `MicroHydros InfluxDB` data source if prompted.
6. Complete the import.

## Test the dashboard

Generate all four test measurements using the Node-RED storage flow described in:

```text
docker/node-red/README.md
```

Refresh the Grafana dashboard. The four current-value panels should display the latest measurements, and the historical panels should display the values over time.

Use the Grafana time-range selector to inspect measurements recorded at earlier times.

## Data source

The dashboard queries the InfluxDB measurement:

```text
sensor_reading
```

It separates the four measurements using the `measurement_type` tag.

## Security

InfluxDB credentials are configured locally and are not included in the exported dashboard.

