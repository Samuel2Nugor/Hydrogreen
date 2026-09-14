import json
import paho.mqtt.client as mqtt

BROKER_HOST = "localhost"
BROKER_PORT = 1883
DEVICE_ID = "simulator-01"
TOPIC = f"microhydros/v1/devices/{DEVICE_ID}/telemetry/raw"

client = mqtt.Client()
client.connect(BROKER_HOST, BROKER_PORT)


payload = {
    "schema_version": 1,                # vilken version av kontraktet ni följer (alltid 1 just nu)
    "device_id": DEVICE_ID,             # "simulator-01" - redan definierad ovanför
    "boot_id": "sim-boot-1",            # ett ID för denna "uppstart" av simulatorn - hittepå-sträng duger
    "sequence": 0,                      # meddelande nummer 0 (första meddelandet efter start)
    "uptime_ms": 0,                     # hur länge simulatorn "varit igång" i millisekunder - 0 vid start
    "measurements": {
        "internal_temperature_c": 23.5,        # innetemp i Celsius
        "internal_humidity_percent": 60.0,     # inne-luftfuktighet i %
        "external_temperature_c": 15.0,        # utetemp i Celsius
        "water_temperature_c": 20.0            # vattentemp i Celsius
    },
    "sensor_status": {
        "internal_sht31": "ok",         # status för inne-sensorn - "ok" betyder sensorn funkade
        "external_sht31": "ok",         # status för ute-sensorn
        "water_ds18b20": "ok"           # status för vattensensorn
    }
}

client.publish(TOPIC, json.dumps(payload), qos=1)
client.disconnect()
print("Skickat!")