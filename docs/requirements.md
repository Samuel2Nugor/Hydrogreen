# MicroHydros Requirements

> **Status:** Draft
>
> **Source:** MicroHydros project specification and HydroGreen Fingers customer brief
>
> **Approval:** Requirements must be reviewed and agreed upon by the team.

## Purpose

This document translates the customer’s needs and the course specification into clear and testable project requirements.

It defines what the prototype must demonstrate. Technical implementation decisions will be documented separately.

## Priority definitions

| Priority | Meaning |
|----------|---------|
| **Must** | Required for the prototype or course delivery to be accepted |
| **Should** | Important and should be included when reasonably possible |
| **Could** | Optional and only considered after all Must requirements are stable |

## Requirement categories

| Prefix | Category |
|--------|----------|
| `FR` | Functional requirement |
| `CR` | Technical constraint |
| `QR` | Quality and reliability requirement |
| `DR` | Documentation and delivery requirement |

## Requirement format

Each requirement should contain:

- A unique ID
- A priority
- A clear requirement
- An acceptance criterion describing how the team can verify it
- A reference to its source

## Functional requirements

### FR-01 — Measure internal air temperature

- **Priority:** Must
- **Requirement:** The prototype shall measure the air temperature inside the hydroponic growing environment.
- **Acceptance criteria:**
  - A sensor is positioned inside the growing environment.
  - The embedded system successfully reads its temperature value.
  - The reading is distinguishable from the external-air and water-temperature readings.
- **Source:** Project specification, section 3.1, item 1.

### FR-02 — Measure external air temperature

- **Priority:** Must
- **Requirement:** The prototype shall measure the air temperature outside the hydroponic growing environment.
- **Acceptance criteria:**
  - A sensor is positioned outside the growing environment.
  - The embedded system successfully reads its temperature value.
  - The reading is distinguishable from the internal-air temperature.
- **Source:** Project specification, section 3.1, item 2.

### FR-03 — Measure water temperature

- **Priority:** Must
- **Requirement:** The prototype shall measure the temperature of the water or nutrient solution.
- **Acceptance criteria:**
  - A suitable sensor measures the water or nutrient solution.
  - The embedded system successfully reads its temperature value.
  - The sensor is suitable for contact with, or protected from, the measured liquid.
- **Source:** Project specification, section 3.1, item 3.

### FR-04 — Measure internal relative humidity

- **Priority:** Must
- **Requirement:** The prototype shall measure relative humidity inside the growing environment.
- **Acceptance criteria:**
  - A humidity sensor is positioned inside the growing environment.
  - The embedded system successfully reads a relative-humidity value.
  - The value is identifiable as an internal humidity measurement.
- **Source:** Project specification, section 3.1, item 4.

### FR-05 — Perform recurring measurements

- **Priority:** Must
- **Requirement:** The prototype shall perform measurements repeatedly so that multiple readings can be observed over time.
- **Acceptance criteria:**
  - The system completes multiple measurement cycles without manual restarting.
  - Each cycle includes the four mandatory measurement points.
  - The order or timing of the readings makes it possible to distinguish separate measurement cycles.
- **Source:** Project specification, sections 3.1 and 9; customer brief, pages 1–2.

### FR-06 — Process measurements using an embedded system

- **Priority:** Must
- **Requirement:** Measurement data shall be read and processed by an embedded system.
- **Acceptance criteria:**
  - The selected embedded device reads the connected sensors.
  - The device converts the readings into a structured form suitable for communication.
  - The team can explain the embedded system’s role in the solution.
- **Source:** Project specification, section 9, item 3.

### FR-07 — Communicate data to an external system

- **Priority:** Must
- **Requirement:** The prototype shall communicate measurement data from the embedded system to an external system.
- **Acceptance criteria:**
  - Measurement data successfully leaves the local embedded device.
  - Another computer, gateway, server, broker or equivalent system receives the data.
  - The team can demonstrate the communication during testing or the final demonstration.
- **Source:** Project specification, sections 5 and 9; customer brief, page 3.

### FR-08 — Handle incorrect or unreasonable situations

- **Priority:** Must
- **Requirement:** The prototype shall include handling for incorrect, failed or unreasonable situations.
- **Acceptance criteria:**
  - At least one relevant failure or invalid-reading scenario is identified.
  - The system detects or handles the selected scenario instead of silently treating it as valid data.
  - The behaviour and result are documented and tested.
- **Source:** Project specification, section 9, item 5; customer brief, pages 2–3.

## Technical constraints

### CR-01 — Prohibited sensors

- **Priority:** Must
- **Requirement:** DHT11 and DHT22 sensors shall not be used in the prototype.
- **Acceptance criteria:**
  - Neither sensor family appears in the implemented hardware.
  - Neither sensor family appears as a selected component in the hardware documentation.
- **Source:** Project specification, section 3.1; customer brief, page 2.

### CR-02 — Research and justify sensor choices

- **Priority:** Must
- **Requirement:** The team shall research possible sensors and justify the selected main sensors using criteria relevant to the project.
- **Acceptance criteria:**
  - More than one reasonable option is considered for the main sensor functions.
  - The comparison uses relevant factors such as accuracy, range, resolution, reliability, response time, interface, cost, availability, power consumption or environmental suitability.
  - The final choices and trade-offs are documented.
- **Source:** Project specification, section 4; customer brief, page 2.

### CR-03 — Select and justify the communication solution

- **Priority:** Must
- **Requirement:** The team shall select and justify a suitable method for communicating measurement data to an external system.
- **Acceptance criteria:**
  - The selected communication method is documented.
  - At least one reasonable alternative is considered.
  - The decision explains why the selected method fits the prototype’s needs and limitations.
- **Source:** Project specification, section 5; customer brief, page 3.

## Quality requirements

### QR-01 — Prefer stability over unnecessary scope

- **Priority:** Must
- **Requirement:** The team shall prioritize a small, stable and well-motivated prototype over additional unfinished functionality.
- **Acceptance criteria:**
  - All Must requirements are prioritized before optional features.
  - Optional features do not prevent completion or testing of the core prototype.
  - Known stability problems and limitations are documented.
- **Source:** Project specification, section 9; customer brief, page 3.

### QR-02 — Consider future historical measurements

- **Priority:** Should
- **Requirement:** The system design should consider that historical measurement data may become important in a future product.
- **Acceptance criteria:**
  - The data representation supports distinguishing measurements taken at different times.
  - The documentation explains how measurements could later be stored or analysed historically.
  - A full historical-analysis system is not required for this prototype.
- **Source:** Customer brief, page 2.

## Explicitly out of scope

The following features are not mandatory and shall only be considered after the core requirements are stable:

- A production-ready commercial cloud platform
- A mobile application
- An advanced web interface
- Machine learning
- Automatic watering control
- Automatic lighting control
- Advanced prediction algorithms
- Finished physical product design

**Source:** Project specification, sections 5 and 14.
