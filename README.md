# phoenix-flight-computer
Flight computer sensor board firmware for Rocket Propulsion Lab's flagship liquid-propellant rocket **Phoenix**.

This system:
* Samples pressure transducers over I2C ADCs (and other sensors in a modular fashion).
* Streams telemetry frames back to the ground over raw Ethernet.
* Is structured to allow easy/clean addition of new sensors as self-contained modules.

Valve actuation, thermocouples and load cells have been removed from this board's firmware (valve commands are no longer received). The firmware targets an ESP32.

## Repo Structure 
```text
phoenix-fc-sensor-board/
├── phoenix-fc-sensor-board.ino     # Main entry: setup(), loop(), telemetry frame build & Ethernet TX
├── src/
│   ├── constants.h                 # Telemetry timing (and legacy valve constants)
│   ├── w5500/                      # MACRAW driver wrapper for Wiznet W5500
│   └── pt/                         # Pressure transducer abstraction & configuration
├── docs/
│   └── diagrams/                   # Valve / plumbing / system diagrams
├── LICENSE
└── README.md
```

## High Level Operation
On boot:
* Initialize serial (115200 baud) for debugging.
* Initialize W5500 Ethernet Chip in MACRAW mode with the sensor board MAC address.
* Initialize I2C and the ADCs for all pressure transducers.

On loop:
* Every `TELEMETRY_DELAY` ms (200 ms), read all PTs and transmit a telemetry frame to the ground station.

## Ethernet Communication Protocol
The Phoenix FC and ground station communicate using custom raw Ethernet frames over the W5500.

### MAC Addressing
Each device is assigned a unique unicast MAC:
```c
// Ground station
const uint8_t MAC_GROUND_STATION[6] = {0x02, 0x47, 0x53, 0x00, 0x00, 0x01}; // "GS"
// Phoenix sensor/flight computer (this repo)
const uint8_t MAC_SENSOR_GIGA[6]    = {0x02, 0x53, 0x49, 0x00, 0x00, 0x04}; // "SI, Sensor Interface"
```
### Frame Type
Telemetry uses a custom EtherType:
* Telemetry frames (SI to GS): `0x8889`

(Command frames, `0x63e4`, are not currently handled by this firmware.)
### Generic Frame Layout
```text
+------------+------------+-----------+-----------------+
| Dst MAC(6) | Src MAC(6) | Type (2)  | Payload (N ...) |
+------------+------------+-----------+-----------------+
```

### Telemetry Frames (SI to GS)
**EtherType**: `0x8889`

Telemetry is currently serialized as simple CSV ASCII string in payload.

**Payload format**:
```text
Payload: "PT0,PT1,...,PTn"
```
Where:
* `PT[i]` = engineering-value pressure in PSI, in the order listed in `src/pt/pt.cpp` (a reading of `0.0` indicates an error)

## Pressure Transducer System
Pressure transducers are encapsulated in `src/pt/`. Conceptually:
* Each PT has a name, an ADC channel, an engineering pressure range and an output voltage range.
* PTs are read through two ADS1015 I2C ADCs (addresses `0x48` and `0x49`) on SDA = GPIO21, SCL = GPIO22.
* The module holds static configurations of all PTs (GN2, LOX/LNG upstream, downstream and dome).

To add/modify PTs, edit the table in `src/pt/pt.cpp`; the main loop automatically aggregates all readings.

## Launch Control (Ground Station Reference)
The following launch control modes are implemented on the ground side; they are kept here for reference only and are not part of this board's firmware.

### Dev Mode (Risk: High)
* This mode is intended for testing valve openings before fueling
* All valves are free to be switched on and off
* Be very sure of what you are doing when using this mode

![My Image](./docs/diagrams/phoenix-fc-launch-control-dev.svg)

### Fueling Mode (Risk: Medium)
* In fueling mode, all flow valves are locked in OFF mode
* all vent valves can be controlled by individual switch

![My Image](./docs/diagrams/phoenix-fc-launch-control-fueling.svg)

### Launch Mode (Risk: Low)

### Buttons/Switches
* Abort (button): has highest priority, open vent valves for all
* Arm (switch): open/close pressurant valves
* Launch (button): open flow valves

![My Image](./docs/diagrams/phoenix-fc-launch-control-launch.svg)

## Physical Specification
### MCU
ESP32-MINI-1 (ESP32-U4WDH)

It's worth noting that this is an Xtensa core, not a RISC-V one
### Ethernet
[W5500 Ethernet LAN Network Module](https://www.amazon.com/HiLetgo-Ethernet-Network-Support-Microcontroller/dp/B0CDWX9VQ5)
