#!/usr/bin/env python3
"""
MicroHydros telemetry simulator.

Stands in for the ESP32-S3 + SHT31 + DS18B20 firmware described in
docs/technical-solutions.md until real hardware is wired up. Publishes
one combined MQTT message per read cycle containing the 4 project-plan
measurements:

    - temp_internal      SHT31, growing environment air temperature (C)
    - humidity_internal  SHT31, growing environment relative humidity (%RH)
    - temp_external      outside air temperature (C) - sensor TBD, see README
    - temp_water         DS18B20, nutrient solution temperature (C)

Topic:   microhydros/<device_id>/telemetry
Payload: JSON, one object per read cycle (see README.md for the schema
and the reasoning for one combined message vs. one topic per metric).

This payload shape is a proposal, not an approved contract -
docs/data-contract.md is still a placeholder. Treat this as the draft
to bring to the team, not a settled spec.
"""

import argparse
import json
import logging
import math
import random
import signal
import sys
import time
from dataclasses import dataclass
from datetime import datetime, timezone

import paho.mqtt.client as mqtt

log = logging.getLogger("microhydros.simulator")


@dataclass
class DriftingMetric:
    """
    A slowly-drifting value with sensor noise on top, optionally following
    a daily sinusoidal baseline (for things like outdoor temperature).

    value = baseline(t) + drift + noise

    drift is a bounded random walk (mean-reverts toward 0) so the signal
    wanders realistically instead of trending off to infinity.
    """

    mean: float
    daily_amplitude: float = 0.0
    noise_sigma: float = 0.1
    drift_sigma: float = 0.02
    drift_bound: float = 1.5
    value_min: float | None = None
    value_max: float | None = None
    _drift: float = 0.0

    def sample(self, now: datetime) -> float:
        seconds_of_day = now.hour * 3600 + now.minute * 60 + now.second
        daily_phase = (seconds_of_day / 86400.0) * 2 * math.pi
        # Peak in mid-afternoon rather than at midnight.
        baseline = self.mean + self.daily_amplitude * math.sin(daily_phase - math.pi / 2)

        self._drift += random.gauss(0, self.drift_sigma)
        self._drift = max(-self.drift_bound, min(self.drift_bound, self._drift))

        value = baseline + self._drift + random.gauss(0, self.noise_sigma)

        if self.value_min is not None:
            value = max(self.value_min, value)
        if self.value_max is not None:
            value = min(self.value_max, value)
        return round(value, 2)


class EquipmentSpikeEvent:
    """
    project-plan.md calls out that supporting electrical equipment can
    quickly push internal temp/humidity out of range. Simulate that as
    occasional, short-lived spikes rather than pretending the environment
    is always calm - that's the actual failure mode the prototype needs
    to catch.
    """

    def __init__(self, chance_per_tick: float = 0.01, duration_ticks_range=(3, 8)):
        self.chance_per_tick = chance_per_tick
        self.duration_ticks_range = duration_ticks_range
        self.remaining_ticks = 0
        self.temp_boost = 0.0
        self.humidity_drop = 0.0

    def tick(self) -> None:
        if self.remaining_ticks > 0:
            self.remaining_ticks -= 1
            if self.remaining_ticks == 0:
                self.temp_boost = 0.0
                self.humidity_drop = 0.0
            return

        if random.random() < self.chance_per_tick:
            self.remaining_ticks = random.randint(*self.duration_ticks_range)
            self.temp_boost = random.uniform(2.0, 6.0)
            self.humidity_drop = random.uniform(5.0, 15.0)
            log.warning(
                "simulated equipment event: +%.1fC / -%.1f%%RH for %d ticks",
                self.temp_boost,
                self.humidity_drop,
                self.remaining_ticks,
            )

    @property
    def active(self) -> bool:
        return self.remaining_ticks > 0


def build_metrics() -> dict[str, DriftingMetric]:
    return {
        "temp_internal": DriftingMetric(
            mean=24.0, daily_amplitude=1.0, noise_sigma=0.15,
            value_min=10.0, value_max=45.0,
        ),
        "humidity_internal": DriftingMetric(
            mean=65.0, daily_amplitude=3.0, noise_sigma=0.8,
            value_min=0.0, value_max=100.0,
        ),
        "temp_external": DriftingMetric(
            mean=13.0, daily_amplitude=5.0, noise_sigma=0.3,
            value_min=-20.0, value_max=45.0,
        ),
        "temp_water": DriftingMetric(
            mean=20.0, daily_amplitude=0.3, noise_sigma=0.05,
            drift_sigma=0.01, drift_bound=0.5,
            value_min=0.0, value_max=40.0,
        ),
    }


def make_reading(metrics: dict[str, DriftingMetric], spike: EquipmentSpikeEvent) -> dict:
    spike.tick()
    now = datetime.now(timezone.utc)

    temp_internal = metrics["temp_internal"].sample(now)
    humidity_internal = metrics["humidity_internal"].sample(now)
    temp_external = metrics["temp_external"].sample(now)
    temp_water = metrics["temp_water"].sample(now)

    if spike.active:
        temp_internal = round(temp_internal + spike.temp_boost, 2)
        humidity_internal = round(max(0.0, humidity_internal - spike.humidity_drop), 2)

    return {
        "temp_internal": temp_internal,
        "humidity_internal": humidity_internal,
        "temp_external": temp_external,
        "temp_water": temp_water,
        "unit": {
            "temp_internal": "C", "humidity_internal": "%RH",
            "temp_external": "C", "temp_water": "C",
        },
        "ts": now.strftime("%Y-%m-%dT%H:%M:%SZ"),
    }


def maybe_corrupt(reading: dict, anomaly_rate: float) -> dict:
    """
    Optionally mangle a reading to exercise the backend's validation path
    (dropped field, out-of-range value, wrong type). Off by default -
    only used with --inject-anomalies.
    """
    if random.random() >= anomaly_rate:
        return reading

    kind = random.choice(["drop_field", "out_of_range", "wrong_type", "null_value"])
    metric = random.choice(["temp_internal", "humidity_internal", "temp_external", "temp_water"])
    reading = dict(reading)

    if kind == "drop_field":
        reading.pop(metric, None)
        log.info("anomaly injected: dropped %s", metric)
    elif kind == "out_of_range":
        reading[metric] = 9999.0
        log.info("anomaly injected: %s out of range", metric)
    elif kind == "wrong_type":
        reading[metric] = "not-a-number"
        log.info("anomaly injected: %s wrong type", metric)
    elif kind == "null_value":
        reading[metric] = None
        log.info("anomaly injected: %s null", metric)

    return reading


def parse_args(argv=None):
    p = argparse.ArgumentParser(description="MicroHydros MQTT telemetry simulator")
    p.add_argument("--broker-host", default="localhost")
    p.add_argument("--broker-port", type=int, default=1883)
    p.add_argument("--device-id", default="microhydros-sim-01")
    p.add_argument("--interval", type=float, default=10.0, help="seconds between reads")
    p.add_argument("--qos", type=int, default=1, choices=[0, 1, 2])
    p.add_argument("--count", type=int, default=0, help="number of readings, 0 = run forever")
    p.add_argument("--inject-anomalies", type=float, default=0.0,
                    help="probability (0-1) per reading of publishing a malformed payload")
    p.add_argument("--seed", type=int, default=None, help="RNG seed, for reproducible runs")
    p.add_argument("-v", "--verbose", action="store_true")
    return p.parse_args(argv)


def main(argv=None) -> int:
    args = parse_args(argv)
    logging.basicConfig(
        level=logging.DEBUG if args.verbose else logging.INFO,
        format="%(asctime)s %(levelname)s %(message)s",
    )
    if args.seed is not None:
        random.seed(args.seed)

    topic = f"microhydros/{args.device_id}/telemetry"

    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id=f"sim-{args.device_id}")
    client.on_connect = lambda c, u, f, rc, props=None: log.info(
        "connected to %s:%d (rc=%s)", args.broker_host, args.broker_port, rc
    )
    client.on_disconnect = lambda c, u, dc, rc=None, props=None: log.warning("disconnected (rc=%s)", rc)

    stop = {"flag": False}

    def handle_sigint(signum, frame):
        stop["flag"] = True

    signal.signal(signal.SIGINT, handle_sigint)
    signal.signal(signal.SIGTERM, handle_sigint)

    try:
        client.connect(args.broker_host, args.broker_port, keepalive=30)
    except Exception as exc:
        log.error("could not connect to broker at %s:%d - %s", args.broker_host, args.broker_port, exc)
        return 1

    client.loop_start()

    metrics = build_metrics()
    spike = EquipmentSpikeEvent()

    published = 0
    try:
        while not stop["flag"]:
            reading = make_reading(metrics, spike)
            reading["device_id"] = args.device_id
            payload_dict = maybe_corrupt(reading, args.inject_anomalies)
            payload = json.dumps(payload_dict)

            result = client.publish(topic, payload, qos=args.qos)
            result.wait_for_publish(timeout=5)
            log.info("published to %s: %s", topic, payload)

            published += 1
            if args.count and published >= args.count:
                break
            time.sleep(args.interval)
    finally:
        client.loop_stop()
        client.disconnect()

    log.info("stopped after %d reading(s)", published)
    return 0


if __name__ == "__main__":
    sys.exit(main())
