# Time.Manager

`PiSubmarine.Time.Manager` drives a fixed-period single-threaded tick loop for
`Time::ITickable` objects.

## Responsibility

This module owns:

- registration of tickable objects
- fixed-period tick pacing
- uptime and delta-time calculation
- exposing current runtime time telemetry through `Time.Telemetry.Api`
- a blocking `Run()` loop

It does not own:

- thread pools
- dynamic composition while running
- business logic inside tick handlers

## Design

The manager is intended for deterministic single-threaded runtime composition.

- `AddTickable()` and `RemoveTickable()` are expected to be used before `Run()`
- `Run()` blocks and repeatedly calls `Tick(uptime, deltaTime)` on the
  registered tickables
- `Stop()` requests loop termination
- pacing uses a fixed target period and sleeps to the next scheduled deadline
  to avoid drift accumulation
