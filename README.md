# fand - A simple, flexible fan control daemon

## Introduction

fand is a daemon that controls the fans (and other pwm-controlled devices)
in your system. It's main feature is its configurability, using a concept of
thermal zones.

# Features

- Configurability
- Fan curve interpolation
- Supports stable hwmon paths
- Lightweight, written in C with minimum dependencies

## Zones

A zone is a collection of fans and sensors. Each sensor in a zone is polled,
optionally has an offset applied, and the maximum sensor value in a zone is
used to control the fans in the zone. Each fan has a curve, defined as a list
of temperatures and the respective PWM values to be applied. For temperatures
inbetween the defined curve points, the PWM values are interpolated automatically.

This allows you to:

- have your case fans follow both CPU temperatures and GPU temperatures
- set your AIO cooler fan speeds to follow the coolant temperatures, but also
  ramp up when the GPU gets hot
- have your pump speed follow the CPU temperatures but stay high when the
  coolant is warm
- all of the above, simultaneously!

## Example

A simple config with one sensor and 2 fans:

```yaml
zones:
  - name: Zone #1
    sensors:
      - path: /sys/class/hwmon/hwmon7  # NZXT Kraken coolant temperature
        index: 1
        offset: 0
    fans:
      - path: /sys/class/hwmon/hwmon5  # NCT6798 fan header #1
        index: 1
        curve:
          temperatures:
            - 25
            - 30
            - 37
            - 40
          speeds:
            - 55
            - 80
            - 220
            - 255
      - path: /sys/class/hwmon/hwmon5  # NCT6798 fan header #2
        index: 2
        curve:
          temperatures:
            - 25
            - 30
            - 37
            - 40
          speeds:
            - 55
            - 80
            - 220
            - 255
```

## Configuration

### Global options

| Option | Default | Description |
|--------|---------|-------------|
| `poll_interval` | `1` | Polling interval in seconds |

### Fan options

| Option | Default | Description |
|--------|---------|-------------|
| `hysteresis` | `0` | Minimum PWM change to trigger a write (prevents flutter) |

## Signal handling

| Signal | Effect |
|--------|--------|
| `SIGTERM` / `SIGINT` | Graceful shutdown, restores automatic fan control |
| `SIGHUP` | Reload configuration without restarting |

## Usage

```
fand [config_file]
```

If no config file is specified, `fand.conf` in the current directory is used.

## Dependencies

- libyaml

## Limitations

- Only hwmon fans and sensors are supported
- Each zone has to have at least one fan and at least one sensor
- A fan can't be used in more than one zone (well, it can, it's just that only
  the last setting will be applied)
