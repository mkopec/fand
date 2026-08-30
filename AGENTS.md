# AGENTS.md

fand is a small Linux fan-control daemon written in C. It polls hwmon temperature
sensors from `/sys`, maps them to PWM duty cycles via piecewise-linear curves, and
writes the results to hwmon PWM devices. Config is YAML; the only external
dependency is libyaml.

## Build & test

```sh
make          # builds ./main (installed as fand)
make test     # builds ./test_runner and runs it; all assertions must pass
make install  # installs binary + systemd unit (run as root)
```

- Toolchain: `gcc -Wall -Wpedantic -g`, links `-lyaml -lm` (order matters: libs
  after sources). Requires C11 (uses `__VA_OPT__`).
- CI (`.github/workflows/test.yml`) runs exactly `make test` on every push/PR —
  keep it green. Tests run unprivileged and must never touch real hwmon
  devices (they use in-memory objects and temp files under `/tmp`).
- Run `make test` before committing anything C.

## Code layout

| File | Responsibility |
|---|---|
| `daemon.c` | `main()`, signal handling, main loop, config validation |
| `config.c` | libyaml event-based parser; builds the full object tree |
| `hwmon.c` | `hwmon_resolve_path()` — stable paths (`.../hwmon`), glob expansion |
| `sensor.c` | `tempN_input` reader (millidegrees ÷ 1000 + offset; NAN on error) |
| `curve.c` | Piecewise-linear curve, clamps 0–255 |
| `fan.c` | `pwmN` writer with hysteresis, `pwmN_enable` manual/auto switching |
| `zone.c` | Zone = sensors + fans; max sensor value drives all zone fans |
| `common.h` | Constants (`MAX_ZONES`, `MAX_ZONE_SIZE`, `MAX_PATH`), version, log macros |
| `test.c` | Hand-rolled test harness (ASSERT/ASSERT_EQ), no framework |

## Invariants (do not break)

- `pwmN_enable` semantics: `1` = manual control (fand owns the fan), `5` =
  hardware auto. `fand_config_enable()` sets 1 at start, `fand_config_disable()`
  must restore 5 on every exit path (shutdown, SIGHUP reload, fatal config
  error) so fans are never left in manual control by a dead daemon.
- Curve inputs must be strictly increasing; `curve_create()` enforces this and
  returns NULL. Keep that validation if you touch `curve.c`.
- Zones with zero fans or zero sensors are silently skipped by the parser.
- Config parse failures must degrade gracefully: skip the invalid object with a
  `DBG()` message, don't crash. `fand_config_load()` returns NULL only when the
  file is unreadable/unparseable or no zones survive.
- `hwmon_resolve_path()` must keep working for: explicit `hwmonN` paths
  (backward compat), paths ending in `/hwmon` (scan for first `hwmonN` child),
  and glob patterns. Callers own the returned allocation.
- All sensor values are thousandths of a degree (e.g. 52300 = 52.3°C).

## Conventions

- C11, 4-space indent, `snake_case` functions, `static` for file-local
  functions, `struct`-per-subsystem headers.
- Diagnostics go through `DBG()` (stderr). There is no log-level system;
  `DEBUG 1` in `common.h` is hardcoded.
- No new runtime dependencies. Keep it libyaml + libc + libm.
- Tests: `static void test_xxx(void)` in `test.c` using `ASSERT`/`ASSERT_EQ`,
  registered at the end of `main()`. Follow the existing pattern — no
  frameworks.
- Bump `FAND_VERSION` in `common.h` and `pkgver` in `PKGBUILD` together when
  releasing (they have drifted: 0.1.1 vs 1.3).

## Runtime facts

- Default config path is `fand.conf` in the CWD; pass the path as argv[1].
- Signals: `SIGTERM`/`SIGINT` = graceful shutdown (restores hardware control);
  `SIGHUP` = reload config (bad new config → keep the old one running).
- Main loop polls every `poll_interval` seconds (default 1); per-fan
  `hysteresis` skips PWM writes within the threshold to reduce sysfs churn.
- systemd unit `fand.service` expects `/usr/local/bin/fand /etc/fand.conf`.
