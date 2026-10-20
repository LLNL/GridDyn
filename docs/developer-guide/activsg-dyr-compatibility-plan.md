# ACTIVSg PSS/E compatibility plan

For the current ACTIVSg25k renewable parameter and format audit, see
[ACTIVSg25k dynamic-model audit](activsg25k-dynamics-audit.md). The renewable
"Missing" labels in the historical inventory below predate the current
REGCA1/REECA1 implementation; the audit describes the remaining case-specific
gaps and is the source of truth for ACTIVSg25k renewable status.
The latest whole-case initialization diagnosis and reproducible method are in
[the ACTIVSg25k dynamic investigation](activsg25k-dynamic-investigation.md).

## Scope

This is the single compatibility plan for the supplied ACTIVSg500,
ACTIVSg2000, ACTIVSg10k, ACTIVSg25k, and ACTIVSg70k PSS/E RAW/DYR pairs. It combines their
inventories so that implementation work is prioritized by total impact rather
than by case-file size.

The source cases remain outside the repository:

| Case        | RAW source                                                      | DYR source                                                               | DYR records |
| ----------- | --------------------------------------------------------------- | ------------------------------------------------------------------------ | ----------: |
| ACTIVSg500  | `C:\Users\phlpt\Downloads\ACTIVSg500\ACTIVSg500.RAW`            | `C:\Users\phlpt\Downloads\ACTIVSg500\ACTIVSg500_dynamics.dyr`            |         272 |
| ACTIVSg2000 | `C:\Users\phlpt\Downloads\ACTIVSg2000\ACTIVSg2000.RAW`          | `C:\Users\phlpt\Downloads\ACTIVSg2000\ACTIVSg2000_dynamics.dyr`          |       1,739 |
| ACTIVSg10k  | `C:\Users\phlpt\Downloads\ACTIVSg10k\ACTIVSg10k.RAW`            | `C:\Users\phlpt\Downloads\ACTIVSg10k\ACTIVSg10k_dynamics.dyr`            |       7,404 |
| ACTIVSg25k  | `C:\Users\phlpt\Downloads\ACTIVSg25k\ACTIVSg25k.RAW`            | `C:\Users\phlpt\Downloads\ACTIVSg25k\ACTIVSg25k.dyr`                     |      18,108 |
| ACTIVSg70k  | `C:\Users\phlpt\Documents\test_cases\ACTIVSg70k\ACTIVSg70k.RAW` | `C:\Users\phlpt\Documents\test_cases\ACTIVSg70k\ACTIVSg70k_dynamics.dyr` |      40,418 |

The five files contain 67,941 DYR records. Based on current DYR dispatch,
56,260 records are recognized and 11,681 remain unsupported. This count
includes the provisional `WT3P1` and `WT3T1` adapters; recognition does not
establish equation equivalence or dynamic validation. A model is complete only
after import, initialization, limits, and disturbed trajectories agree with
an external reference.

### Governor initialization compatibility policy

Dynamic initialization is permissive by default for supported governor models:
when an initialized governor output is above its configured upper limit,
GridDyn raises the upper limit to the initialized value and emits a warning.
Set `strict_governor_limits` (or the alias `strictgovernorlimits`) through
`-f/--flags` or `flags=...` to reject that initial upper-limit mismatch instead.
This policy applies only to the initial upper-limit mismatch; invalid model
parameters and lower-limit violations remain errors.

The same compatibility policy is used for supported exciter initialization:
when an initialized upper-bound signal exceeds its configured limit, GridDyn
raises that limit to the initialized value and emits a warning by default.
This covers DC1A/DC2A regulator limits, IEEE Type-1/Type-2 regulator limits,
SCRX EMAX, ESAC6A regulator limits, ESST4B regulator/inner limits, and the
EXAC1/ESAC1A/EXAC2 regulator limits. Set `strict_exciter_limits` (or the
alias `strictexciterlimits`) through `-f/--flags` or `flags=...` to reject
that initial upper-limit mismatch instead. Invalid model parameters and
lower-limit violations remain errors.

### Initial-limit consistency audit

For every model using the automatic policy, the raised parameter is the same
parameter used by the dynamic limiter: its root surface detects that adjusted
boundary, the limiter holds the associated state only while its drive points
outward, and the state is released when the drive becomes inward. DAE hold
residuals use the corresponding `-cj` diagonal when the residual is
`-dx/dt`; IEEE Type-1 and the base IEEE-simple governor were corrected to
meet that convention. IEEE Type-2 uses `+dVr/dt` in its own residual and
therefore correctly retains `+cj`.

IEEE Type-2 previously bypassed the shared initialization policy. It now
raises `VRMAX` by default (or rejects the point under
`strict_exciter_limits`) before entering its normal runtime limiter.

The following exciters still deliberately reject an inconsistent initial
point rather than applying the generic policy: AC7B, AC8B, ESST1A, ESST2A,
ESST3A, and IEEET3. They each have multiple or scaled bounds, so direct
assignment of an initialized signal to `VRMAX` would be dimensionally wrong.
For example, ESST1A's field ceiling is `VRMAX*Vt - KC*Ifd`, ESST3A has
independent `VRMAX` and `VMMAX` states, and AC7B/AC8B have nested PID,
regulator, and exciter limits. Any extension of the permissive policy must
derive and update the particular runtime parameter for each violated bound,
then add an active-limit residual/Jacobian regression; it should not reuse
the one-dimensional helper blindly.

## Dynamic execution results

The ACTIVSg10k load-step measurements below are from 2026-09-22. The
2026-10-14 flat-run checks and the remote-swing-bus importer correction are
recorded after those results.

The merged branch has moved ACTIVSg10k beyond import-only status. The supplied
RAW/DYR pair now initializes in the Release build with the supported
conventional-machine and controller set, including the previously blocking
EXAC1 `TA=0` algebraic regulator path and valid GENSAL records with
`Xdp == Xdpp`. EXAC2 limiter roots also have regression coverage for state
projection, stale root directions, and rearming after the solver advances in
time.

A no-disturbance short equilibrium check remains stationary. A diagnostic
IDA run with a 10 MW step on `BUS$50000::LOAD#0` at `t=1.0 s` reaches
`t=1.03 s` in approximately 76 seconds. The old same-time EXAC2 root loop is
no longer reproduced, but the run still encounters a system-wide cascade of
IEEE Type-1 exciter and IEEEG1 governor limiter roots. This is a usable smoke
result, not full dynamic validation: a 9-second post-disturbance run, an
independent trajectory comparison, and a performance fix remain outstanding.

### ACTIVSg10k remote swing-terminal dynamic reference (2026-10-14)

A flat full-case DAE and partitioned run initially drifted because a RAW swing
bus whose generator remotely regulates another bus was converted from static
SLK to AFIX, while its dynamic bus type remained NORMAL. That left its dynamic
angle reference free. A controlled one-generator case isolated the issue:
adding `dyntype=fixangle` to that swing bus reduced the partitioned 100 ms
maximum absolute state-vector change from `1.528` to `3.8e-5`; the DAE 10 ms
change fell from `4.55` to `6.3e-5`.

The RAW importer now sets both static AFIX and dynamic FIX_ANGLE for this case.
The focused RAW regression fixture
`test/test_files/input_tests/raw_remote_voltage_swing_terminal.raw` verifies
both bus types. The focused single-controller and grouped remote-voltage
import tests pass.

With the original ACTIVSg10k RAW/DYR pair, roots enabled, switched-shunt and
tap adjustments disabled, and no disturbance, both Release solver modes
completed to `1.03 s`. Capturing states every 10 ms produced 104 snapshots
through the endpoint. The largest absolute state-vector displacement from
initialization was `7.97e-5` for DAE and `8.20e-5` for partitioned; the endpoint
displacements were `7.93e-5` and `8.13e-5`, respectively. These are
flat-equilibrium smoke runs, not a disturbed trajectory comparison; the
9-second post-disturbance validation above remains open.

Reproduce either flat run from PowerShell by setting `$method` to `dae` or
`partitioned`:

```powershell
$case = 'C:\Users\phlpt\Documents\test_cases\ACTIVSg10k'
$method = 'dae' # or 'partitioned'
& .\build-ninja-release\bin\gridDynMain.exe `
  "--input=$case\ACTIVSg10k.RAW" `
  "--import=$case\ACTIVSg10k_dynamics.dyr" `
  "--param=dynamicsolvermethod=$method" '--param=timestop=1.03' `
  '--flags=sparse,no_tap_changer,no_shunt_adjustments' `
  --state-output="activsg10k-$method-1030ms.bin" --save-state-period=10
```

ACTIVSg25k's standalone RAW power flow has a converged baseline with switched-
shunt adjustments disabled, and the full RAW+DYR file has been loaded into the
dynamic path. Earlier Release runs stopped at time zero with REECA1 or ESST4B
initialization-limit errors. After rebuilding with ESST4B failure context, a
flat DAE attempt remained active for 30 minutes and a partitioned CVODE attempt
for one hour without a terminal status; both were stopped without a completed
trajectory. A 0.1-second partitioned run with state saves every 10 ms stayed
active for 15 minutes without its first snapshot. No full DAE or partitioned
run has passed, and the five-second stability check remains open. The four-
record Type-3 wind adapter's fidelity remains a separate limitation.

## Power-flow readiness

| Case        | MATPOWER                                               | RAW                                                                                                                     | EPC                                                                       | Current conclusion                                                                                                                                                                                                                                                                                                                                                |
| ----------- | ------------------------------------------------------ | ----------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| ACTIVSg500  | Solves                                                 | Solves                                                                                                                  | Solves                                                                    | Outputs agree within the established tolerance.                                                                                                                                                                                                                                                                                                                   |
| ACTIVSg2000 | `case_ACTIVSg2000.m` solves                            | Solves but differs from EPC by up to 0.03746 pu V / 0.00589 rad                                                         | Solves                                                                    | Control-device interpretation still needs parity work.                                                                                                                                                                                                                                                                                                            |
| ACTIVSg10k  | `case_ACTIVSg10k.m` solves: 10,000 buses, 12,706 links | Solves: 10,000 buses / 12,706 links; max source-state V error 0.000743 pu                                               | Solves: 10,000 buses / 12,706 links; max source-state V error 0.002317 pu | Three-winding topology is validated across all 300 units; RAW convergence defect was negative fixed phase-shifter `COD` interpretation.                                                                                                                                                                                                                           |
| ACTIVSg25k  | `case_ACTIVSg25k.m` provides a 25,000-bus solved state | Solves: 25,000 buses / 32,230 links; stays within 0.000101 pu V and 0.000124 rad of the RAW-supplied terminal-bus state | Not assessed                                                              | RAW and MATPOWER source states differ by up to 0.07851 pu V / 0.02221 rad on 24,250 common terminal buses, so cross-format parity is not yet established.                                                                                                                                                                                                         |
| ACTIVSg70k  | Solves: 70,000 buses / 88,207 links                    | Solves: 70,000 buses / 88,207 links                                                                                     | Solves: 70,000 buses / 88,207 links                                       | Max bus P residual is at most 0.00001 MW and max Q residual at most 0.08494 MVAr across the three formats. RAW+DYR DAE initialization and a near-flat 1 s run pass; full EPC+DYD remains blocked by the `WLWSCC` active-current load curve. The diagnostic EPC+DYD overlay is solver-consistent between DAE and partitioned runs, but is not the unmodified pair. |

### MATPOWER generator step-up-transformer limitation

The standard MATPOWER `mpc.gen` table does not carry the PSS/E RAW generator
step-up-transformer fields `RT`, `XT`, or `GTAP`. When `RT` or `XT` is
nonzero, the RAW reader models that impedance as a generator-side AC link and
generated bus; a MATPOWER import cannot reconstruct that portion of the
network.

Consequently, corresponding RAW and MATPOWER cases can both load, converge,
and retain identical supplied bus voltages, but converge to different solved
operating points. This was observed in ACTIVSg10k: the supplied terminal-bus
states agree to about `1e-7` pu/rad, whereas the solved RAW/MATPOWER states
differ by as much as `0.02324` pu in voltage magnitude and `0.00226` rad in
angle. For example, the generator at bus 30342 has nonzero RAW `RT` and its
MATPOWER record has no equivalent field.

Use RAW (and a format that preserves the equivalent generator transformer)
when exact network-model parity is required. Do not use a standard MATPOWER
export as an exact solution-equivalence reference for a case that has
generator step-up impedances unless those impedances have been represented
consistently in both models.

## Dynamic-model inventory

| PSS/E model | ACTIVSg500 | ACTIVSg2000 | ACTIVSg10k | ACTIVSg25k | ACTIVSg70k |  Total | Current status / required work                                                                                                                                                                                                                                |
| ----------- | ---------: | ----------: | ---------: | ---------: | ---------: | -----: | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `GENROU`    |         90 |         410 |      1,136 |      2,857 |      6,937 | 11,430 | Reader supported; continue initialization and trajectory validation.                                                                                                                                                                                          |
| `SEXS`      |         90 |           0 |          0 |          0 |          0 |     90 | Reader supported; validate case configurations.                                                                                                                                                                                                               |
| `TGOV1`     |         21 |           0 |          0 |          0 |          0 |     21 | Reader supported; validate case configurations.                                                                                                                                                                                                               |
| `IEEEST`    |          2 |         434 |      1,851 |      4,101 |      9,243 | 15,631 | Reader supported for local-input configurations; validate variants used.                                                                                                                                                                                      |
| `IEEEG1`    |          0 |          43 |        162 |      1,115 |      3,518 |  4,838 | Reader supported; validate case configurations.                                                                                                                                                                                                               |
| `ESDC1A`    |          0 |          12 |        163 |        123 |        427 |    725 | Reader supported; validate case configurations.                                                                                                                                                                                                               |
| `EXAC2`     |          0 |          38 |        106 |        239 |        486 |    869 | **Implemented and merged; external trajectory pending.** Zero-`TR` and bypassed-state paths, initialization, active limits, and root rearming have regression coverage.                                                                                       |
| `GGOV1`     |          0 |         367 |        974 |      1,742 |      3,419 |  6,502 | **Reader/model implemented.** Equation, initialization, DYR-order, coupling, residual, and analytic-Jacobian tests exist. Nonzero `TENG` is rejected.                                                                                                         |
| `ESST4B`    |          0 |         278 |        745 |      1,396 |      3,187 |  5,606 | **Reader/model implemented.** Exact DYR mapping and GENSAL-coupled residual/Jacobian tests exist; external UEL/OEL inputs remain unrouted.                                                                                                                    |
| `GENSAL`    |          0 |          25 |        715 |      1,244 |      2,306 |  4,290 | **Implemented and merged.** Salient-pole equations, quadratic saturation, initialization, controller signals, Jacobians, and the valid `Xdp == Xdpp` degenerate case are covered.                                                                             |
| `HYGOV`     |         39 |          25 |        715 |      1,244 |      2,306 |  4,329 | Dedicated `GovernorHygov` and DYR mapping implemented; validate case initialization, limits, and trajectories.                                                                                                                                                |
| `SCRX`      |          0 |           5 |        312 |        446 |      1,053 |  1,816 | **Reader/model implemented; external trajectory open.** OpenIPSL/PowerDynamics equations, source selection, crowbar behavior, and DYR mapping are covered.                                                                                                    |
| `IEEET1`    |          0 |          23 |        214 |        942 |      1,907 |  3,086 | **Implemented; external trajectory open.** Native IEEE Type 1 model and exact DYR mapping are available; validate representative case configurations and trajectories.                                                                                        |
| `EXPIC1`    |          0 |          61 |        153 |        287 |        569 |  1,070 | **Implemented; external trajectory open.** GridKit-specified equations, exact DYR schema, full block tests, and whole-case residual/Jacobian coverage are present.                                                                                            |
| `ESDC2A`    |          0 |           1 |         87 |        202 |        104 |    394 | **Implemented and merged; external trajectory pending.** Shares DC2A/EXDC2 equations and DYR schema; nonzero unsupported `Switch` is rejected.                                                                                                                |
| `ESAC6A`    |          0 |           7 |         26 |        194 |        583 |    810 | **Reader/model implemented; external trajectory open.** GridKit AC6A equations, saturation/rectifier behavior, initialization, and DYR mapping are covered.                                                                                                   |
| `EXAC1`     |          0 |           6 |         22 |        130 |        381 |    539 | **Implemented and merged; external trajectory pending.** `TA=0` algebraic-regulator and bypassed-state paths, limits, and root behavior have regression coverage.                                                                                             |
| `ESAC1A`    |          0 |           4 |         23 |        142 |        546 |    715 | **Implemented and merged; external trajectory pending.** Dedicated AC1A DYR mapping and control-element limits reuse the AC-exciter core; UEL/OEL routing remains open.                                                                                       |
| `GAST`      |         30 |           0 |          0 |          0 |          0 |     30 | **Implemented; external trajectory open.** OpenIPSL/ANDES/GridKit equations, DYR mapping, initialization, selector/limit, and Jacobian tests are present.                                                                                                     |
| `REECA1`    |          0 |           0 |          0 |        614 |        571 |  1,185 | **Implemented; validation pending.** Native renewable electrical control and DYR mapping are available with the associated `REGCA1` converter; complete external and whole-case trajectory checks.                                                            |
| `REGCA1`    |          0 |           0 |          0 |        614 |        571 |  1,185 | **Implemented; validation pending.** Native renewable converter and DYR mapping are available with `REECA1`; complete external and whole-case trajectory checks.                                                                                              |
| `WT3E1`     |          0 |           0 |          0 |        119 |        576 |    695 | **Dedicated model; validation pending.** Electrical controller, DYR mapping, initialization, and host-Jacobian coverage are present; whole-plant trajectories remain open.                                                                                    |
| `WT3G1`     |          0 |           0 |          0 |        119 |        576 |    695 | **Dedicated model; validation pending.** Electrical interface, DYR mapping, initialization, and host-Jacobian coverage are present; whole-plant trajectories remain open.                                                                                     |
| `WT3P1`     |          0 |           0 |          0 |        119 |        576 |    695 | **Provisional adapter.** The DYR field order now follows PowerWorld's documented `Tp, Kpp, Kip, Kpc, Kic, ThetaMin, ThetaMax, RThetaMax, Pset` list when translating to WTPTA1. Equation, initialization, base, and trajectory equivalence remain unverified. |
| `WT3T1`     |          0 |           0 |          0 |        119 |        576 |    695 | **Provisional adapter.** DYR records translate to `WTARA1` plus `WTDS`/`WTDTA1`; source field semantics and equation/trajectory equivalence remain unverified.                                                                                                |

The broader model-by-model mapping is maintained in the
[ANDES compatibility roadmap](andes-compatibility.md); this table is the
authoritative ACTIVSg demand inventory.

### GridKit equation and case overlay

GridKit at `C:\data\Documents\codeProjects\GridKit` commit `de7bea18`
provides a second C++-oriented source of model equations and JSON validation
artifacts. It materially improves the source position for `GAST` and
`EXPIC1`: GridKit implements `GASTPTI` and supplies ACTIVSg500/WECC240 case
paths, while its EXPIC1 documentation contains the complete PI, filter,
feedback, source/rectifier, saturation, and initialization equations.

| GridKit artifact                         | Models/case coverage                                                                                                 | How GridDyn should use it                                                                                                        |
| ---------------------------------------- | -------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------- |
| Native model sources                     | `GENROU`, `GENSAL`, `GASTPTI`, `HYGOV`, `TGOV1`, `IEEET1`, `ESDC1A`, `SEXS-PTI`, `IEEEST`, `REGCA`, `REECB`, `REPCA` | Compare model equations and, where the interfaces align, capture initialization and trajectory references.                       |
| Documented but not GridKit-native models | `EXPIC1`, `SCRX`, `ESAC6A`, `ESDC2A`, `ESST4B`, `EXAC1`, `EXAC2`, `EXDC1`, `GGOV1`, `IEEEG1`, `PSS1A`                | Use as equation specifications only; do not claim GridKit trajectory validation until a runnable implementation/case is present. |
| Self-contained regression examples       | Two-bus `IEEET1`, `TGOV1`, and `GENSAL`, with `.case.json`, `.solver.json`, and `.ref.csv`                           | Port as small GridDyn trajectory fixtures before relying on a large-case comparison.                                             |
| Large JSON cases                         | ACTIVSg200, ACTIVSg500, ACTIVSg10k, and WECC240                                                                      | ACTIVSg500 has supplied validation traces; WECC240 additionally exercises GAST/HYGOV/TGOV1 plus `REGCA`/`REECB`/`REPCA`.         |

The local GridKit ACTIVSg2000 directory is **not** a runnable dynamic input:
it contains its README and image only, and the README lists EXPIC1 among its
unimplemented models. Therefore the final ACTIVSg2000 integration remains
dependent on the separate RAW/DYR delivery; GridKit cannot substitute for it.

### ACTIVSg25k static DYR assessment

`ACTIVSg25k.dyr` has 18,108 records in 22 model families. It has now been
loaded for whole-case dynamic initialization. Earlier binaries stopped at
renewable reference-limit or ESST4B lower-limit errors; the current no-shunt
Release attempts remained active but did not produce completed trajectories.
DYR dispatch recognizes all 22 names,
including provisional `WT3P1` and `WT3T1` adapters that translate to existing
wind components. Those translations have not been validated against the
legacy model equations, so the case remains uncertified for full Type-3
dynamics.

The largest remaining gap is now the renewable groups. `REECA1` and `REGCA1` must be initialized and
validated together; the four `WT3*` models are likewise one Type-3 wind-turbine
system, not four independent optional models. Existing recognition is not by
itself case validation: every supported family still needs applicable
case-configuration, initialization, limiter, and disturbance-trajectory
coverage.

### ACTIVSg70k RAW, DYR, and initialization assessment

The supplied files in `C:\Users\phlpt\Documents\test_cases\ACTIVSg70k` were
run with the rebuilt Release `gridDynMain.exe`. All three static inputs
returned exit code 0 and reported 70,000 buses and 88,207 links. Following the
local PV Q redispatch correction, the bus-44359 results are:

| Input               | Exit | Voltage (pu) | Angle (deg) | Qgen (MVAr) | Qlink (MVAr) | Q residual (MVAr) |
| ------------------- | ---: | -----------: | ----------: | ----------: | -----------: | ----------------: |
| `case_ACTIVSg70k.m` |    0 |     1.044000 |   -112.7940 |   -71.42619 |     71.42619 |           0.00000 |
| `ACTIVSg70k.RAW`    |    0 |     1.044000 |   -112.7471 |   -71.40720 |     71.40720 |           0.00000 |
| `ACTIVSg70k.EPC`    |    0 |     1.044000 |   -112.8664 |   -71.40098 |     71.40098 |           0.00000 |

All three outputs report 70,000 buses, 88,207 links, and 10,390 generators.
The MATPOWER import reports 34,449 load objects; RAW and EPC report 42,619.
The prior MATPOWER mismatch was traced to the local PV control path. The PV
Newton equations omit the bus reactive-balance equation, but when a bus returned
from PQ to PV after a Q-limit release, its local generators remained pinned at
the old Q limit. The solver then had no equation that could refresh their Q
outputs, even after the required aggregate Q moved back inside the unit limits.
GridDyn now redispatches local voltage-controlling generators to the bus's
required Q whenever a PV or slack bus is within its aggregate limits. This
preserves each unit's Q bounds and its configured sharing fraction. The
MATPOWER Q residual at bus 44359 is now zero to the CSV precision.

Across all 70,000 bus rows, the largest absolute reactive residual is 0.00001
MVAr for MATPOWER, 0.00498 MVAr for RAW, and 0.08494 MVAr for EPC; none exceeds
1 MVAr. The largest real-power residual is 0 MW for MATPOWER and RAW and
0.00001 MW for EPC. These results support accepting the three static
power-flow runs; they do not certify dynamic behavior.

The three results have voltage magnitudes from 0.9378 to 1.0900 pu.
Their power-flow-only solves and file imports are now exercised, but this is
not an external reference comparison for three-winding transformer behavior.

The paired `ACTIVSg70k.RAW` and `ACTIVSg70k_dynamics.dyr` import and dynamic
initialization now complete on the rebuilt executable. The DYR has 40,418
records in 22 model families, and the current DYR dispatch recognizes all 22
names. The initial-condition diagnostic reports `IDA_SUCCESS` with a maximum
absolute DAE residual of `6.44e-9` and a maximum algebraic residual of
`7.75e-11` when run with `--param=tolerance=1e-10`; this is below the selected
`1e-8` residual acceptance threshold. The run advanced to `1e-6 s` and exited
successfully. At the default `1e-6` tolerance the residual was `7.74e-5`, so a
tighter solver setting is needed to meet that threshold.

Before WT3P1 setpoint alignment, the initialized physical derivative reached
`-5.01 deg/s` on `WTPTA1.powerIntegral`: the imported `WT3P1.Pset` was `1.0 pu`
while the corresponding `WT3E1.PORD` was about `0.833 pu`. IDA made the DAE
residual small by assigning a matching derivative, but the operating point was
not stationary. In a no-disturbance run, that integrator moved by `3.00` state
units by `0.6 s`; the diagnostic was stopped as the solve slowed near the pitch
clamp at `0.7 s`.

To isolate that mismatch, a temporary diagnostic DYR copy changed only the
576 `WT3P1.Pset` values to the corresponding RAW generator `PG/MBASE` values
(all 576 bus/machine pairs matched). This is not a permanent input edit or a
validated WT3P1 interpretation. The one-second IDA run completed with exit
code 0: all ten 0.1-second solves returned normally, with zero error-test and
nonlinear-convergence failures. The largest state deviation was `1.53e-3` at
`0.1 s` and decreased to `8.55e-4` at `1 s` (largest final change: an
`ESAC6A.xll` state). IDA evaluated root functions 547 times, but no root was
returned and no limit activity was logged after integration began. This is a
near-flat diagnostic run, not a zero-derivative proof: the initial derivative
probe still reported `max_abs_yp=0.1662`.

The WT3P1 adapter now aligns its fixed Pset to the initialized PORD during
dynamic initialization. A configured Pset of `1.0` is treated as a flat-start
placeholder and adjusted silently; shifting any other configured value logs a
warning with both values. An already matched Pset is left unchanged. The
native WTPTA1 reader is unaffected. All 576 WT3P1 records in this DYR specify
`Pset=1.0`. With the original RAW and DYR unchanged, the one-second IDA run
now exits successfully: all ten 0.1-second solves return 0, the maximum state
change is `1.53e-3` at `0.1 s` and `8.55e-4` at `1 s`, and no root is returned
or limit activity logged after integration begins. IDA evaluates root
functions 547 times as part of normal monitoring. The initial residual remains
`6.43e-9`; the largest initial derivative is `0.1662` in an exciter. This is a
near-flat start, not a proof of zero derivatives or WT3P1 equation fidelity.

Both the supplied and diagnostic DYR runs widened 796 controller upper bounds
during initialization to include their starting outputs. That initialization
adjustment is distinct from a dynamic limiter event; it remains a caveat if
the acceptance criterion forbids any limit-parameter adjustment at all.

The DYR contains generator and plant-controller records only; it has no
composite dynamic-load records. The 42,619 RAW load objects therefore remain
static load representations in this test. `WT3P1` and `WT3T1` still use
provisional translations; their equation and parameter fidelity and the full
Type-3 trajectory remain unverified. The case-specific audit also confirms
the supported selectors: all 3,419 `GGOV1` records use `TENG=0`; all 104
`ESDC2A` records use `Switch=0`; all 9,243 `IEEEST` records use `MODE=1`,
`BUSR=0`; all 3,518 `IEEEG1` records have no secondary generator; and all 571
`REECA1` records use local measurements (`BUSR=0`).

The initialization diagnostic is reproducible with:

```powershell
build-ninja-release\bin\gridDynMain.exe `
  C:\Users\phlpt\Documents\test_cases\ACTIVSg70k\ACTIVSg70k.RAW `
  --import C:\Users\phlpt\Documents\test_cases\ACTIVSg70k\ACTIVSg70k_dynamics.dyr `
  --param=dynamicsolvermethod=dae --param=timestop=0.000001 --param=tolerance=1e-10 `
  --flags=sparse,ida_ic_diagnostics,ida_ic_stop_on_failure,no_shunt_adjustments,no_powerflow_error_recovery `
  --verbose
```

`GENSAL`, `HYGOV`, `GGOV1`, `ESST4B`, `SCRX`, and `ESAC6A` are recognized and
have focused model and reader tests, but still need large-case initialization
and trajectory validation. The coupled renewable demand is `REGCA1` plus
`REECA1` (571 each) and all four Type-3 wind models (576 each).

#### Full-dynamics execution boundary

The package contains both PowerWorld (`.PWB`, `.aux`, `.pwd`, and `.EPC`) and
PSS/E (`.RAW` and `.dyr`) representations. GridDyn's reference import path is
the paired `ACTIVSg70k.RAW` and `ACTIVSg70k_dynamics.dyr` files; the PowerWorld
artifacts are useful cross-tool references, but are not substitutes for the
RAW/DYR import. The DYR is a complete plant assembly, so a run must attach
every record for a machine ID before initializing the case.

| Dynamic assembly                | DYR families required together                                          |                                  Records | Execution consequence                                                                                                                                                                                        |
| ------------------------------- | ----------------------------------------------------------------------- | ---------------------------------------: | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Round-rotor conventional plants | `GENROU`, applicable exciters/governors, and optional `IEEEG1`/`IEEEST` |                   6,937 `GENROU` records | Import recognizes the attached model families; whole-plant initialization, limit behavior, and disturbed trajectories still need validation. Do not drop controller records from a plant assembly.           |
| Salient-pole hydro plants       | `GENSAL` + `HYGOV` + exciter + optional `IEEEST`                        | 2,306 `GENSAL` and 2,306 `HYGOV` records | Native machine and governor models are implemented; validate whole-plant initialization, bases, limits, controller interfaces, and disturbed trajectories.                                                   |
| General-governor plants         | `GENROU` + `GGOV1` + exciter + optional `IEEEST`                        |                    3,419 `GGOV1` records | Native selectable governor/turbine modes are implemented; validate case parameter combinations and reject nonzero `TENG` until transport delay is added.                                                     |
| Renewable converter plants      | `REGCA1` + `REECA1`                                                     |                              571 of each | Components and DYR mappings are implemented; whole-case initialization and disturbed-trajectory validation remain required.                                                                                  |
| Type-3 wind plants              | `WT3G1` + `WT3E1` + `WT3P1` + `WT3T1`                                   |                              576 of each | `WT3G1`/`WT3E1` are dedicated models; WT3P1's documented field order is translated to WTPTA1, and WT3T1 translates to WTARA1 plus WTDS/WTDTA1. Model behavior and full-plant trajectories remain unverified. |

The 70k MATPOWER, RAW, and EPC power-flow paths complete with small bus
residuals. The RAW+DYR DAE initialization completes at a maximum residual of
`6.44e-9`; with WT3P1 setpoint alignment, its one-second no-disturbance run has
no returned root or post-start limit activity and a maximum final state change
of `8.55e-4`. It remains near-flat rather than exactly stationary because the
initial maximum derivative is `0.1662` in an exciter, and initialization
widened 796 controller upper bounds.

The unmodified EPC+DYD pair is not yet a complete dynamic pass: its full
`WLWSCC` active-current load characteristic prevents time-zero power-flow
convergence, and omitting `WLWSCC` alone leaves the Type-3 assemblies without
a terminal electrical model. On a diagnostic copy that omits `WLWSCC` and
adds 576 paired DYR `WT3G1` records, DAE and partitioned one-second runs both
complete without a returned root or post-start limit activity. All 143,964
differential states agree at time zero to floating-point precision; the
maximum DAE/partitioned difference at one second is `6.47e-5` (RMS
`5.47e-7`). Final state drift is `5.15e-4` for DAE and `5.27e-4` for
partitioned. This verifies consistency for the diagnostic assembly, not for
the supplied EPC+DYD pair; its DAE residual (`1.51e-8`) is slightly above the
`1e-8` target, and initialization widens 784 controller upper bounds.

Remaining 70k gaps are the `WLWSCC` convergence cause, inclusion of a
validated Type-3 terminal electrical model in the EPC/DYD format, the small
exciter drift in the RAW/DYR flat run, source-equation and trajectory
validation for provisional `WT3P1`/`WT3T1`, and fault-trajectory validation of
the `REGCA1`/`REECA1` pair. Dynamic-load behavior is not exercised because the
supplied DYR has no dynamic-load records.

#### ACTIVSg70k EPC and DYD reader assessment

The paired `ACTIVSg70k.EPC` and `ACTIVSg70k_dynamics.dyd` import reaches the
power-flow solver with all 39,843 DYD records across 22 model families
recognized: `ESAC1A`, `ESAC6A`, `ESDC1A`, `ESDC2A`, `ESST4B`, `EXAC1`, `EXAC2`,
`EXPIC1`, `GENROU`, `GENSAL`, `GGOV1`, `HYGOV`, `IEEEG1`, `IEEEST`, `IEEET1`,
`REEC_A`, `REGC_A`, `SCRX`, `WLWSCC`, `WT3E`, `WT3P`, and `WT3T`. There is no
unsupported-family error for this input.

The reader accepts the PSLF field extensions that differ from the shared
PSS/E-shaped loaders: `IEEET1` may include a 15th `Spdmlt` field,
`ESDC1A`/`ESDC2A` may include trailing `UEL` and `exclim` selectors, and
`ESAC6A` may include a trailing `Spdmlt`, which maps to the native speed
multiplier option. PSLF `GENROU` and `ESAC6A` parameter orders are now
translated to the native model orders; `WT3P` uses the same initial Pset
alignment rule as the DYR `WT3P1` adapter. Unsupported nonzero `Spdmlt` values on `IEEET1`/`ESAC1A` and
nondefault exciter selectors are reported as grouped warnings. In this case,
104 `ESDC2A` records have `exclim=0`; that selector is not represented by the
native exciter model and remains a fidelity gap.

The one global `WLWSCC` record is read as a load-characteristic template and
applied to non-fixed-shunt loads. Its parameters specify active constant-current
load (`p2=1`) and reactive constant-impedance load (`q1=1`). The ordinary EPC
power flow succeeds, and EPC plus all DYD records also succeeds when only the
global `WLWSCC` record is omitted. In controlled runs, a constant-power curve
and the reactive-impedance-only curve converged; the active-current-only curve
did not, and a native WSCC-load curve with the same nominal-frequency
`P proportional to V` behavior showed the same failure.
With the full `WLWSCC` record, KINSOL reports `-7` and `-11` during the
time-zero power-flow solve. This isolates the remaining failure to
power-flow convergence with the active-current characteristic, not DYD model
recognition. The failing bus or residual source has not yet been identified;
full EPC+DYD power-flow convergence remains open.

With the DYD reader fixes, omitting only `WLWSCC` allows the EPC power flow to
complete, but dynamic initialization then reports that the Type-3 wind
generators lack a terminal electrical model. The DYD has 576 `WT3E`/`WT3P`/
`WT3T` assemblies and no `WT3G1` records. In the paired DYR, all 576 `WT3G1`
records have `Xeq=0.8`, `Kpll=30`, `Kipll=0`, `Pllmax=0.1`; each record's
`WIND_BASE * Prated` equals the corresponding DYD `WT3E.MWCap`. The split into
WIND_BASE and Prated is not encoded in the DYD. The native WT3G1 model uses
Prated only for parameter validation, but adding that model remains a
cross-format diagnostic overlay rather than a validated PSLF interpretation.

A diagnostic DYD copy that omits `WLWSCC` and appends those 576 paired DYR
`WT3G1` records initializes with DAE `IDA_SUCCESS`: maximum residual
`1.51e-8`, maximum algebraic residual `8.17e-11`, and largest initial
derivative `0.0898`. The original DYD's 576 `WT3P.Pset` values are all `1.0`,
so they align silently to initial PORD. On this same diagnostic input, both DAE
and partitioned one-second runs exit successfully, with no logged root return
or dynamic limit change. Maximum final state drift is `5.15e-4` for DAE and
`5.27e-4` for partitioned. All 143,964 differential states match at time zero
to floating-point precision; their largest difference at one second is
`6.47e-5` (RMS `5.47e-7`). Initialization widens 784 controller upper bounds
in each run. The partitioned run takes about 233 seconds versus the shorter
DAE run and repeatedly resets KLU symbolic factorization as its sparse
Jacobian pattern changes. These results support solver consistency for the
diagnostic assembly, not for the unmodified EPC+DYD pair. The DAE residual is
slightly above the prior `1e-8` acceptance threshold. Tightening the requested
solver tolerance from `1e-10` to `1e-11` causes IDA initial-condition
convergence failure (`IDA_CONV_FAIL`, algebraic residual about `9.96e-6`), so
that stricter initialization criterion remains open.

The full `WLWSCC` power-flow blocker persists with both the supplied 100%
active-current curve and a 50% active-current/50% constant-power diagnostic
curve; a 1% active-current curve converges. Seeding EPC bus voltages and angles
from that 1% solution still does not converge with the supplied curve. The
specific failing buses or Jacobian/residual cause remain unidentified.

The supplied RAW is PSS/E v33. It has 67,900 terminal buses, 71,352 branches,
10,555 two-winding transformers, and 2,100 three-winding transformers. Its
delta-to-star expansion yields the 70,000 buses and 88,207 branches in the
MATPOWER file. All transformer records use `CW=1`, `CZ=1`, and `CM=1`; 2,083
winding records use fixed/manual `COD=-1` control. There are no nonempty DC,
FACTS, multisection-line, induction-machine, or system-switching-device
sections. Therefore no new RAW record family is required for this case. The
successful RAW solve exercises the three-winding and fixed-control transformer
paths at scale; comparison against a reference remains necessary before
accepting their numerical equivalence or using the result as a dynamic baseline.

## Implementation order

The cross-case source availability and P1/P2 ranking is maintained in the
[OpenIPSL dynamic-model assessment](openipsl-compatibility.md#synthetic-case-demand-overlay).
That overlay distinguishes models that can be ported from an inspected
OpenIPSL/ANDES/PowerDynamics implementation from source gaps that require
equations before implementation.

1. **Power-flow prerequisites:** resolve the remaining ACTIVSg2000
   control-device parity difference, add grouped PSS/E remote-voltage
   regulation, and investigate the ACTIVSg25k RAW-versus-MATPOWER source-state
   mismatch. ACTIVSg10k RAW/EPC three-winding topology is validated.
2. **P1 conventional machine/governor validation:** `GENSAL`, `GGOV1`,
   `HYGOV`, and the merged `IEESGO` realization are implemented; add captured
   external trajectories and whole-case initialization coverage. GGOV1
   deliberately rejects nonzero `TENG` transport delay.
3. **P1 synchronous excitation:** `ESST4B`, `IEEET1`, `SCRX`, `EXPIC1`, and
   `ESAC6A` are implemented with exact DYR mappings and focused equation
   tests. Capture external trajectories; ANDES's `SEXS` conversion remains
   only an approximation for SCRX/ESAC6A.
4. **P1 renewable generation:** validate the implemented `REGCA1` plus
   `REECA1` (and `REPCA1` for Texas7k), then compare the provisional
   `WT3P1`/`WT3T1` adapters with authoritative source equations and complete
   whole-case Type-3 trajectory checks. Keep the adapter status distinct from
   dedicated-model equivalence.
5. **P2 remaining excitation/source gaps:** `ESDC2A`, `ESAC1A`, `EXPIC1`, and
   `ESAC6A` are merged; capture independent trajectories and finish
   model-specific UEL/OEL action points where the exciter needs them.
6. **Reader and validation hardening:** table-driven DYR dispatch, strict
   unknown-model diagnostics, exact bus-plus-machine-ID resolution, minimized
   fixtures, and whole-case trajectory regressions. Add dedicated
   representative-parameter stability cases for `EXPIC1` and `GAST`, including
   EXPIC1 limiter/rectifier behavior and GAST temperature-selector and valve
   anti-windup transitions.

## Guardrails and acceptance criteria

### Three-winding transformer scope and simplifications

GridDyn represents a three-winding transformer as a star bus and three AC
transformer legs. RAW pairwise leakage impedances are converted from their
delta form to those three star-leg impedances. The initial implementation
supports the ordinary fixed-transformer cases (`CW=1`, `CZ=1/2/3`, and
`CM=1/2`), per-winding ratings, tap ratios, and phase shifts.

For simple planning and test cases, the following deliberately conservative
simplifications are acceptable when documented in the test result:

- Treat `CW=2` or `CW=3` winding-voltage entries as already normalized fixed
  taps, with a reader warning. Use this only when all three nominal voltage
  bases are consistent; otherwise the case needs the dedicated base conversion.
- Treat nonzero `COD` control records as their supplied fixed starting tap.
  This preserves the base power-flow topology but does not reproduce regulated
  tap movement or reactive/active-flow control.
- Convert both magnetizing encodings. `CM=1` conductance/susceptance is
  applied to the first star leg; `CM=2` no-load-loss/excitation-current data
  is converted to system-base `G/B` before the same placement.
- A zero rating is retained as GridDyn's existing “no thermal limit” value; it
  is not replaced with an arbitrary finite rating.

These shortcuts are only suitable for uncomplicated steady-state cases. A
reader is complete only after all PSS/E control codes, voltage-base forms, and
magnetizing representations used by the target cases are implemented and
validated.

- Never silently replace an unsupported PSS/E record with a similarly named
  GridDyn model.
- Resolve DYR controllers by **RAW bus number and quoted machine ID**, never
  generator-array position.
- Validate record length, numeric domains, controller limits, and unit bases
  before attachment.
- Before a full DYR comparison, RAW, EPC, and MATPOWER must retain equivalent
  bus and transformer topology and agree within declared voltage/angle
  tolerances.
- For each new model, require parser/attachment, initialization/limiter, and
  disturbed-trajectory tests against a documented PSS/E or trusted reference.
- Completion requires strict loading with no unknown records plus whole-case
  equilibrium and fault/frequency trajectory agreement.
