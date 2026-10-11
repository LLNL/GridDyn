# Changelog

All notable changes to this project after the 0.9.0 release will be documented here

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/).
This project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [unreleased]

## [0.13.0] - 2026-10-10

### Added

- Added native renewable and wind-generation models, including renewable plant
  controllers, grid-forming and distributed converters, and WT3/WT4 model
  families.
- Added and expanded generator, governor, exciter, stabilizer, voltage-limiter,
  and voltage-compensator models, plus bus/COI measurement sensors and
  protection relays.
- Added composite, CIM, electronic, and motor-load models, with improved load
  templates and DYR/DYD mappings.
- Added PSLF SQLite `.sav` input support and expanded PSS/E RAW and DYR, PSLF
  DYD, and EPC import coverage.
- Added in-memory Python recorder results through `as_dicts()` and
  `as_dataframe()`.
- Added installed CMake package config files that export the `Griddyn::gridDynMain`
  executable target and `Griddyn::griddynServer` when that executable is built.

### Changed

- Consolidated Python output into `sim.write_file(path, type=...)`; the output
  extension selects the writer unless `type` is provided.
- Organized Python solving and time-domain execution on their domain objects:
  use `sim.power_flow.run()` for power flow, `sim.optimization.opf()` for OPF,
  and `sim.time_domain.initialize()`, `sim.time_domain.run()`,
  `sim.time_domain.run_until()`, and `sim.time_domain.step()` for dynamic
  simulation. General actions remain available through `sim.execute(action)`.
- Standardized Python network collections on lowercase plural names such as
  `sim.buses` and `sim.generators`. Callers upgrading from 0.12 should replace
  `sim.powerflow()`/`sim.PFlow.run()` with `sim.power_flow.run()`, move
  `sim.initialize()`, `sim.run()`, `sim.run_until()`, and `sim.step()` to
  `sim.time_domain`, and use `sim.optimization.opf()` instead of
  `solve_opf()`. Replace `sim.TDS`, capitalized singular collection aliases,
  and format-specific output methods with `sim.time_domain`, lowercase plural
  collections, and `sim.write_file()`.
- Clarified that `.sav` denotes the PSLF SQLite reader; legacy binary PSS/E
  saved cases are not supported by that reader.

### Improved

- Expanded model-import diagnostics and validation for unsupported or malformed
  input, including checks that reject invalid PSLF `.sav` files before creating
  model objects.
- Expanded area AGC, reserve dispatch, scheduling, and controller signal-routing
  support.
- Added Ruff naming-convention checks to the Python library CI workflow.
- Expanded initialization, solver, and regression coverage for renewable,
  composite-load, protection, partitioned, DAE, and larger-network workflows.

### Removed

- Retired the FSKIT federation integration and its associated build and source
  files.

## [0.12.0] - 2026-09-25

### Added

- Added a compiled Python package with APIs for loading networks, power flow,
  dynamic simulation, parameter edits, and MATPOWER/PYPOWER export, plus runnable
  examples.
- Added continuous DC optimal power flow for supported polynomial costs, using
  the native solver or the optional HiGHS backend.
- Added `GENSAE` and expanded generator controls, exciters, and governors, along
  with broader PSS/E DYR mappings and EPC+DYD input support.
- Added native PSS/E `GENROE` and `IEEEX1` dynamic models with exact DYR-reader
  mappings, initialization/residual/Jacobian coverage, and component tests.
  `GENROE` uses its exponential saturation curve, while `IEEEX1` supports both
  the zero-`TB` direct path and the lead-lag path. Independent external
  trajectory comparisons remain follow-up validation work.

### Improved

- Expanded PSS/E RAW and MATPOWER compatibility, including transformer records,
  remote voltage control, tap data, and generator cost handling.
- Improved CVODE/SUNDIALS integration and expanded transient simulation
  validation for larger networks and dynamic model behavior.

## Earlier project notes (release version and date unspecified)

- Made major CMake and build-system changes.
- Added Braid solvers.
- Upgraded SUNDIALS to 3.1.
- Added HELICS integration.
