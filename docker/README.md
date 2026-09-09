# Local MQTT Broker

This setup runs an Eclipse Mosquitto MQTT broker locally with Docker Compose.

## Requirements

* Docker Engine
* Docker Compose

## Start the broker

Run from the repository root:

```bash
docker compose up -d
```

## Check the broker status

```bash
docker compose ps
```

The broker is available to local applications at:

```text
localhost:1883
```

## Subscribe to test messages

Open a terminal and run:

```bash
docker exec -it microhydros-mosquitto \
  mosquitto_sub -h localhost -t microhydros/test -v
```

Keep this terminal open.

## Publish a test message

Open another terminal and run:

```bash
docker exec microhydros-mosquitto \
  mosquitto_pub -h localhost \
  -t microhydros/test \
  -m '{"sensor_id":"inside_air","measurement":"temperature","value":23.5,"unit":"C"}'
```

The subscriber should display:

```text
microhydros/test {"sensor_id":"inside_air","measurement":"temperature","value":23.5,"unit":"C"}
```

## View broker logs

```bash
docker compose logs mosquitto
```

## Stop the broker

```bash
docker compose down
```

The persistent Docker volumes are retained when the broker is stopped.

## Current security limitation

Anonymous MQTT access is enabled for local development. Authentication and encrypted communication are not yet configured and must be considered before using the system outside a controlled development environment.

