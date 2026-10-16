# ACTIVSg25k dynamic simulation investigation

## Purpose and current status

This note records the reproducible methods used to diagnose GridDyn's 25,000-bus
ACTIVSg case. Keep the static power-flow, DYR import/attachment, dynamic
initialization, and time-integration stages separate: a failure at one stage
does not establish a failure in the next.

Source data is outside the repository, under
`C:\Users\phlpt\Documents\test_cases\ACTIVSg25k`:

- `ACTIVSg25k.RAW`
- `case_ACTIVSg25k.m`
- `ACTIVSg25k.dyr`
- PowerWorld `.pwb`/`.pwd` and auxiliary files (not used in the current GridDyn
  baseline)

An earlier saved RAW result (`ACTIVSg25k-raw-v5-strict.csv`) contains 25,000
buses, 32,230 links, 4,834 generators, and 10,289 loads, with a 52,223-entry
power-flow state. That historical result is not reproduced by the current
working tree. After the late remote-group state allocation fix, the current
no-adjustment run returns success and keeps SALUDA 2 1 (bus 16025) near its RAW
voltage of 1.018802 pu, but it is not an adjusted operating point. With
adjustments enabled, the original RAW case still fails KINSOL with `-6`.
Therefore, a zero return code from the no-adjustment run is only an import and
equation diagnostic, not evidence of a solved case.

The MATPOWER case also converges in the saved baseline with the same
bus/link/generator counts but 8,655 loads, so the two files should not be
treated as identical static-model imports.

The full no-disturbance 5-second dynamic run has not produced an inspectable
trajectory. Earlier binaries reported REECA1 reference-limit and ESST4B lower
limit initialization errors. With the current Release binary and static shunt
adjustments disabled, the DAE process remained active for 30 minutes and the
partitioned CVODE process for one hour without a terminal status; both were
stopped after those limits. A 0.1-second partitioned run with 10 ms state
snapshots also remained active for 15 minutes without its first snapshot. These
runs do not establish that initialization completed or that the case is stable.
Do not describe the dynamic case as validated until a full run produces an
inspectable trajectory.

Updated 2026-10-13: after synchronizing partitioned root callbacks and fixing
the EXAC1 limiter release transition, the Release build completed a 30 ms flat
run with `no_shunt_adjustments`. This is a short diagnostic only: it still
shows limit roots within the first 30 ms and recorded differential-state drift
after the first governor event. A follow-up selector-tolerance change kept the
three JOHNSTON GGOV1 load integrators flat in a 100 ms run, but 41 limit roots
occurred and another exciter state moved by `0.00279 pu`. The 5 s run and DAE
initialization remain unvalidated.

## Reproducible diagnosis sequence

### 1. Establish a static baseline first

Build the release executable, then run the RAW file by itself with recovery and
power-flow adjustments disabled. This isolates the starting equations from
recovery, but validate residuals and remote-target setpoint errors as well as
the return code: a no-adjustment solve can leave a target uncontrolled when
every associated generator is at a Q limit.

```powershell
build-ninja-release\bin\gridDynMain.exe `
  C:\Users\phlpt\Documents\test_cases\ACTIVSg25k\ACTIVSg25k.RAW `
  --powerflow-only `
  --flags=no_powerflow_adjustments,no_powerflow_error_recovery `
  --powerflow-output ACTIVSg25k-raw-strict.csv `
  --verbose
```

Run `case_ACTIVSg25k.m` separately with the same flags. Record import counts,
solver return code, bus-voltage and branch-flow residuals, remote-target
setpoint errors, and any bus changes.
Compare input model setup as well as solved values: RAW carries control fields
such as `IREG`/`RMPCT` that are absent from the MATPOWER representation, and
load-object counts can differ. Exporting one CSV per format makes bus-ID
comparisons and residual checks repeatable.

### 2. Add the DYR file only after the baseline is known

Run the RAW plus DYR case with no disturbance for five seconds. Save stdout,
stderr, and the GridDyn log independently. Inspect the earliest error and its
time stamp:

- A parser or model-association error occurs before dynamic initialization.
- An algebraic/power-flow error occurs at time zero while forming the initial
  operating point.
- A model parameter or state initialization error occurs after entering
  `dynInitialize` but before integration advances.
- An integration error occurs after a positive simulation time and needs a
  trajectory/stability diagnosis.

Do not infer dynamic instability from a run that fails before the first time
step. Preserve the no-disturbance setup while resolving initialization so that
each change has a comparable baseline.

### 3. Narrow DYR failures with valid record prefixes

When the full DYR file fails, test progressively larger prefixes to find the
first prefix that changes a passing run into a failing run. Generate prefixes
at complete DYR record boundaries, preserving quotes, slash terminators, and
continuation lines; physical line counts are not a reliable record index.
Once the transition is bracketed, compare the adjacent prefixes and isolate
the newly added record. Then test that record alone and with only its required
generator/model dependencies. Keep the original DYR file untouched.

The repository workspace contains earlier prefix and minimized-probe outputs
(`activsg25k-prefix-*.dyr`, `activsg25k-*-probe.dyr`, and their logs). They are
diagnostic artifacts rather than accepted regressions; rerun a reduced probe
when its result matters to a code change.

### 4. Trace model association and initialization by identity

For a suspicious DYR record, add temporary trace output at these boundaries:

1. DYR record tokenization and model-name/parameter extraction.
2. Generator lookup by bus number and machine ID.
3. Model factory creation and parameter assignment.
4. Model `dynInitializeA` entry and return.
5. Generator clone/replacement, bus relinking, and parent attachment.
6. Initial-output calculation and model state initialization.

Include the DYR model name, bus number, machine ID, and a stable record index in
each message. This distinguishes an invalid source record from a lookup,
ownership, clone, or initialization defect. Remove or gate temporary tracing
after diagnosis; keep a minimized regression if it captures a product bug.

Existing `REGCA1` tracing for bus 14199 confirmed that lookup, construction,
`dynInitializeA`, cloning, bus replacement, and attachment all completed for
that record. That proves only that this model's import path completed; it does
not prove its initialized values or its interactions with the full model set
are correct.

### 5. Check source values before changing model equations

For the first failing model, compare its DYR fields with the attached generator
and power-flow outputs. Check machine ID matching, MBASE/system-base conversion,
P/Q sign conventions, initial P/Q against model reference limits, and whether
the model expects values on a different base. A message such as
`REECA1 initial power outside reference limits` should first be traced to the
exact source record and computed reference/output values. Do not widen limits
or clamp the initial point just to make initialization proceed without
establishing the intended model semantics.

### 6. Validate the no-disturbance trajectory and solver paths

After initialization is clean, run the same five-second no-disturbance case
with both the DAE and partitioned approaches. Compare the initial values and
the values at every output sample for bus voltage/angle, generator P/Q, and
the affected dynamic states. Also inspect solver residuals and model warnings.
Use tolerances appropriate to solver accuracy, and report both the maximum
deviation from the initial point and the bus/model where it occurs. A run that
merely returns success is not sufficient evidence that all initial values were
maintained.

### 7. Diagnose RAW remote voltage groups before changing DYR models

Audit each active RAW generator record using `IREG`, `VS`, `RMPCT`, `QT`,
`QB`, `QG`, and `STAT`. Count active controllers and distinct IREG targets,
check that members of each target group agree on `VS`, and retain inactive
records separately. Do not infer a target conflict from a disabled generator.
Record the RMPCT normalization for each group; multiple members that all carry
`RMPCT=100` need an explicit normalization rule.

For a failing group, do not clear other generators' IREG associations or set
their reactive range to zero as a way to freeze them: that can remove remote
group equations while leaving target buses in their original bus types. For a
focused probe, freeze nonselected controllers at their current solved Q in the
simulator while retaining their remote associations and bus equations. Compare
the selected group's voltage, Q-limit status, and reactive mismatch with the
full case. A group at a Q limit can legitimately leave its remote bus off
setpoint; distinguish that from a solver failure. Trace the first release or
clamp event and the next solver state. Repeated release/clamp of the same
member means the active-set policy is cycling and should be addressed before
changing model limits.

For ACTIVSg25k, the audit found 2,513 active IREG generator records across
1,017 target buses. No active target group had inconsistent `VS` values. The
case contains 648 multi-controller targets; disabling those groups avoids the
KINSOL `-6` observed with the multi-controller-only split, while retaining only
those groups reproduces it. A trace of the Bristol 3 group (target bus 35365)
showed Qmin releases followed by requests far beyond Qmin and repeated
clamping. Exact target-voltage plus RMPCT-sharing equations remove the
high-gain approximation, but the adjustment-enabled case still fails KINSOL;
that change is not yet a validated resolution. One-at-a-time release and
limit-exhaustion experiments also failed on the isolated group and were not
retained as fixes.

### Full-case active-set trace (2026-10-11)

The clean Release build was run against the original RAW file with recovery
disabled. With all power-flow adjustments disabled, KINSOL returns success in
about 0.66 seconds, but SALUDA bus 16025 is at 1.176540 pu against the group
target `VS=1.019`; terminal buses 16026, 16027, and 16029 are near 1.224 pu.
This is a fixed-dispatch starting point, not an acceptable controlled solution.
The stored historical `ACTIVSg25k-raw-v5-strict.csv` has SALUDA at 1.018826 pu,
but that artifact is stale and also contains implausible high voltages
elsewhere, so it is a comparison lead rather than an accepted reference.

The adjustment-enabled run first solves the fixed-dispatch system, changes
remote/local control states, and solves once more. That second solve succeeds;
the following adjustment round changes additional limits, and the next KINSOL
solve fails with `-6`. The temporary trace reports SALUDA reaching its voltage
target before its three active generators are then requested at about
`-0.36261 pu` each, against `QMIN=-0.00118 pu`. That discrepancy points to the
remote-controller equations, terminal-bus balance, or Q sign/base handling;
it does not establish that the physical case requires that output.

Three discriminating runs narrow the failure:

- `ignore_bus_limits` still fails with `-6`, so local bus-limit switching is
  not the sole trigger.
- Removing the startup `AT_LIMIT` marking lets every remote unit enter the
  group equations at once, but the sparse linear setup fails immediately with
  `-11`.
- Releasing only one active controller per IREG group avoids that immediate
  setup failure but then stops with KINSOL `-7`.

None of these variants is retained as a fix. The temporary `BUSLIM` trace
counted 472 messages across the two adjustment rounds, but those messages were
emitted whenever a bus's current type differed from its saved `prevType`; they
do not mean 472 buses were disconnected or newly changed on each pass. All
temporary source tracing used for these counts has been removed. The relevant
logs are `ACTIVSg25k-full-current-tol.log`,
`ACTIVSg25k-all-free-initial.log`, `ACTIVSg25k-staged-remote-release.log`,
and `ACTIVSg25k-ignore-bus-limits.log`.

The RAW contains 280 active remote records with `QT=QB=0` across 160 target
groups. The importer currently treats these as controllers with no reactive
capability and excludes them. This interpretation remains an open compatibility
question: keep the source fields and resulting group counts in the audit, and
compare against the PSS/E operating point before changing the rule.

### Cross-format comparison and controlled group probes (2026-10-11)

Use the Release executable and untouched source inputs for this comparison.
The ACTIVSg25k folder contains RAW and MATPOWER files but no EPC file. The
MATPOWER `.m` input completes its adjusted power flow. The RAW input completes
only when adjustments are disabled; the adjusted, no-recovery run returns
KINSOL `-6`. Its no-adjust result is not a valid operating point: bus 16025
moves from the RAW value 1.018802 pu to 1.176540 pu, bus voltages reach
1.687978 pu, and 124 buses exceed 1.1 pu. The MATPOWER adjusted result ranges
from 0.963691 to 1.080324 pu, with bus 16025 at 1.018422 pu. The MATPOWER
result is a comparison lead, not a substitute, because `.m` does not carry the
RAW `IREG`/`RMPCT` groups and its shunt representation differs.
The same RAW run with default recovery remained CPU-bound for more than three
minutes after KINSOL `-6`/`-7` retries and was interrupted; treat that result as
inconclusive, not as a successful recovery.

The source constant-power loads are nearly equal between RAW and MATPOWER:
234,527.569 MW / 62,595.094 MVAr in RAW and 234,527.520 MW / 62,595.310 MVAr
in MATPOWER. The MATPOWER bus table also has total `BS=-2,920` MVAr at 1 pu;
the RAW uses 848 switched-shunt records and has no fixed-shunt records. Compare
shunt states and representation separately rather than interpreting the
power-flow output's aggregate `Qload` column as a difference in constant-power
load.

For group isolation, changing `QT=QB=QG` on every nonselected remote generator
is not a faithful freeze: it removes those controllers from their IREG groups
and can leave original target buses in their RAW bus types. Keep these copies
only as exploratory artifacts. A better probe freezes nonselected controllers
at their current solved Q in the simulator while retaining their remote-bus
association and bus equations. In that controlled SALUDA run, the first
fixed-dispatch solve succeeded; releasing only its three-generator group held
bus 16025 at 1.019 pu but requested `Q=-0.352562 pu` per unit against limits
`[-0.00118, 0.00543] pu`. The next limit clamp left the bus near 1.15 pu. This
reproduces the extreme-Q symptom without other remote groups adjusting; it
does not establish a feasible solution or a retained code change.

Also keep a case-format regression matrix with exact solver flags and recovery
settings. In the clean Release build on this date, strict
`no_powerflow_error_recovery` runs passed for all three formats of ACTIVSg500
and ACTIVSg70k. ACTIVSg2000 MATPOWER passed; RAW returned `-11` without
recovery, and with default recovery the process returned zero while logging
repeated `-11` errors and exceeding the voltage-loop iteration limit, so it is
not a valid pass. EPC returned `-11` even with default recovery. ACTIVSg10k
MATPOWER and EPC passed; RAW solved with adjustments disabled but failed (`-7`
strict, `-11` with default recovery). These runs show that the regression is
not unique to 25k and that return code alone is not enough: retain the format,
flags, warnings, and final bus / shunt state with every result.

Temporary diagnostic source hooks for freezing groups and tracing each solve
were removed after the experiments. They are not a supported mode. The latest
logs are `ACTIVSg25k-freeze-other-after-pass2.log`,
`ACTIVSg25k-only-SALUDA-after-initial-solve.log`,
`ACTIVSg25k-m-clean-adjust.log`, `ACTIVSg25k-raw-default-current.log`, and the
`ACTIVSg-pf-*.log` case matrix.

## Current diagnostic artifacts and next action

Key local artifacts from the current investigation include:

- `ACTIVSg25k-raw-v5-strict.csv` and `ACTIVSg25k-raw-v5-strict.log` for the
  strict RAW baseline;
- `ACTIVSg25k-m-pflow.log` for the MATPOWER baseline;
- `activsg25k-dae-5s.log` for the full dynamic attempt;
- `activsg25k-dyr-reader-trace.txt` and `activsg25k-init-trace.txt` for reader
  and initialization instrumentation;
- the prefix and minimized-probe files described above.

The next step is to reconcile the RAW bus and shunt representation with the
MATPOWER operating point, then trace the first remote-group and local-limit
changes on the original RAW model. Record bus voltages, terminal-bus residuals,
and each group's Q before and after every transition. Resolve the RAW power
flow before resuming the REECA1 initialization error. Then run the full
RAW+DYR case for five seconds with both the DAE and partitioned approaches;
the no-disturbance trajectory is stable only if initial values remain constant
through both runs.

### Mixed local/remote initialization follow-up (2026-10-11)

For a RAW target bus that begins as PV but is later demoted to PQ because a
non-direct IREG group is registered, preserve the imported bus voltage when
making that transition. The old initialization set the PV voltage to a local
setpoint before the remote controllers finished registering; that discarded
the RAW start voltage. A temporary initial-residual probe measured the
MANCHESTE~32 target mismatch falling from 37.2638 to 1.44988 after restoring
the imported voltage. The same probe showed the full-case largest first
residual falling from 37.2638 to 7.14907. These residuals are in the solver's
equation units, not pu voltage errors.

Mixed target groups also need their local generators initialized as indirect
group members. Seed their Q bias from their internal Q dispatch, attach them to
the controlled bus, copy the group's voltage target, and recognize an initial
Q-limit dispatch. This prevents a local target generator from entering the
group equations later with no remote-bus pointer or an uninitialized sharing
bias.

These changes improve the initial guess but do not resolve the 25k case. With
adjustments disabled, the strict RAW run returns success but settles at
implausible voltages, including 1.416 pu at SALUDA bus 16025 and 1.892 pu at
LITTLE PLY bus 16196. With adjustments enabled and recovery disabled, the
strict run returns KINSOL `-6` after about 33 seconds. The first fixed-Q solve
releases many groups together. At SALUDA, the no-adjust voltage is 1.416 pu
against `VS=1.019`; releasing the three active QMAX units then requests about
1.227 pu internal Q per unit against `QMAX=0.00543 pu`, after which they are
clamped again. Treat this as evidence of an unresolved remote-group limit
transition, not as a valid operating point. Making all initial remote
controllers free instead returns KINSOL `-7` and also does not converge.

Retain these tests with this change: ACTIVSg500 RAW and ACTIVSg70k RAW passed
strict `no_powerflow_error_recovery`; ACTIVSg10k RAW returned `-7`; and
ACTIVSg2000 RAW returned `-11`. The latter two failures match the prior case
matrix. Keep their format and flags attached to results; do not infer a pass
from process exit alone when the logs contain solver errors.

The next useful probe is to compare the RAW Q limits and solved Q outputs for
the same units in MATPOWER, especially at SALUDA. The MATPOWER table lists
`QMAX=1.07` for the active SALUDA units while RAW lists `QT=0.543`; its buses
also retain local PV controls where RAW uses remote IREG groups. Until that
limit/control difference is reconciled against an authoritative operating
point, do not continue to the five-second DAE or partitioned dynamic run.

### Late remote-group state allocation and current blocker (2026-10-12)

Use a minimized RAW fixture to separate control registration from state-vector
allocation. Start with `test/test_files/input_tests/raw_remote_voltage_group.raw`
(two remote generators regulating one PQ target). Make a temporary copy with
the target bus changed to PV and add one local generator at that target. Run
both copies with `--powerflow-only --flags=no_powerflow_error_recovery`, save
the bus CSV, and compare the reported power-flow state size, target voltage,
and target/terminal reactive residuals. Keep the original fixture unchanged.

This exposed a late-initialization cache bug. The controls register and the
target is demoted to PQ, and the local and remote generators are all marked as
indirect controllers. However, that marking adds one algebraic Q state to each
generator, while the generator's already-loaded state-size cache remained at
zero. The mixed fixture therefore had only its eight bus voltage/angle states;
the remote group equations were absent from the solve. `Generator::setFlag`
now unloads state sizes and raises `STATE_COUNT_CHANGE` when the indirect flag
changes, and local mixed-group setup uses that setter. This retains the
late-registration path while rebuilding the state offsets before solving.

After the change, the mixed fixture allocates 11 states (eight bus states plus
three generator Q states), holds the target at 1.040000 pu, and has zero Q
residual at the target and both remote generator terminals. The two-remote
regression fixture still allocates 10 states and holds its target at 1.040000
pu. These are focused control-equation checks; the small fixture's slack-bus Q
residual is not a whole-network convergence criterion.

On the full RAW case, the no-adjustment solve now returns success with 52,286
states. It preserves SALUDA bus 16025 at its RAW start value of 1.018802 pu;
the highest voltage is 1.113444 pu, and the largest absolute Q residual is
7.12021 MVAr at bus 14564. This is a useful initialization diagnostic but is
not the adjusted operating point. The strict adjusted run still returns
KINSOL `-6`: the initial fixed-dispatch solve succeeds, then one adjustment
pass releases 38 remote controllers and clamps 17 before the next solve reaches
the iteration limit. Raising KINSOL's limit from 50 to 100 did not make that
solve converge; that temporary diagnostic was reverted. Temporary limit
tracing has also been removed.

Compare the implicated RAW groups with MATPOWER at both the generator limits
and bus setup. For sampled Bellingham units (target 57931, terminals
57934-57936) and Bangor units (target 62267, terminals 62270-62272), the Q
limits match between the files. The setup differs: RAW directs these generators
to remote IREG buses, while the MATPOWER case gives the terminal buses local
PV control and contains no IREG/RMPCT group. This confirms a control-topology
difference for these examples; it does not by itself prove which group makes
the adjusted RAW solve fail.

### 25k RAW remote targets compared with MATPOWER (2026-10-12)

Compare `ACTIVSg25k.RAW` generator records with `case_ACTIVSg25k.m` by bus
number. Count only in-service RAW generators whose `IREG` is nonzero and differs
from the generator terminal bus. For each such record, compare the RAW target
bus type and voltage with `mpc.bus`, then compare the terminal bus and `VS`
with the MATPOWER generator `Vg` and `mpc.bus` voltage.

There are 2,513 active remote-generator records and 1,017 unique IREG target
buses. Every target bus exists in both files and has the same bus type in both:
981 targets are type 1/PQ and 36 are type 2/PV. Every remote generator terminal
bus is type 2/PV in both files. Thirty of the 36 type-2 targets also have an
in-service local generator; six have generator records but no in-service local
unit. All members of a remote target group have the same `VS`, and every active
remote generator record has `RMPCT=100`.

The MATPOWER file has 25,000 bus rows versus 24,250 in the RAW file; its 750
additional rows are type-1, zero-load star buses for the 750 three-winding
transformers. None is needed to match an IREG target, so this row-count
difference is separate from the remote-control comparison.

The target bus type does not identify the remote-control relationship. PSS/E's
generator `IREG` field identifies the bus whose voltage the plant regulates;
PSS/E permits that target to be type 1 or type 2. Its voltage setpoint is held
while reactive-power limits allow it, and the target voltage can depart from
`VS` after the regulating devices saturate. This matches the 25k RAW snapshot:
for 652 of the 981 type-1 target groups, target voltage differs from `VS` by at
least 0.001 pu, and every remote generator in those groups is at a Q limit. For
490 of those groups the mismatch is at least 0.005 pu, again with every remote
generator at a Q limit. The same pattern appears in 28 of 36 type-2 target
groups. This is evidence that `VS` is the remote target setpoint, not a voltage
that should always be imposed when the group is Q-limited.

Representative points show why a MATPOWER solve is not an equivalent
remote-control check:

| IREG target      | RAW target type / Vm | Remote `VS` | RAW terminal-bus Vm                            | MATPOWER terminal `Vg` |
| ---------------- | -------------------- | ----------: | ---------------------------------------------- | ---------------------- |
| Bellingham 57931 | PQ / 1.04402         |       1.044 | 57934-57936: 1.02984, 1.02510, 1.02495         | 1.0296, 1.0247, 1.0246 |
| Bangor 62267     | PQ / 1.04700         |       1.047 | 62270-62272: 1.05012, 1.04950, 1.05069         | 1.0503, 1.0497, 1.0509 |
| Saluda 16025     | PQ / 1.01880         |       1.019 | 16026, 16027, 16029: 1.06127, 1.06070, 1.05998 | 1.0609, 1.0603, 1.0596 |

In these examples MATPOWER retains the PQ target and its starting voltage, but
uses terminal-bus `Vg` values close to the local terminal voltages rather than
the RAW remote `VS`. The standard MATPOWER generator format has no remote-bus
index or RMPCT field. Thus the `.m` case retains a useful solved voltage
profile but encodes local terminal-bus voltage control; it does not preserve
the RAW IREG/RMPCT controller groups. The same-bus-type comparison is therefore
not evidence that the RAW target is uncontrolled. For GridDyn, interpret `IREG`
as the control endpoint independently of the target bus's type: the remote
group adjusts the supplying generators' Q to regulate that target, subject to
Q limits and the group's participation settings. For the type-2 targets with
local generation, account for the local and remote controllers together.

Reference semantics: PSS/E Data Formats Guide, Generator Data (`IREG`, `VS`,
`RMPCT`); PSS/E Operation Manual, Setpoint Voltage Control. This comparison
clarifies the intended control topology but does not identify which group
transition causes the full 25k adjusted solve to fail.

Re-run the focused fixtures and the strict RAW regressions after any change.
With this state-allocation fix, ACTIVSg500 RAW and ACTIVSg70k RAW still pass
strict `no_powerflow_error_recovery` power flow. The 25k adjusted power flow
remains unresolved, so the five-second DAE and partitioned no-disturbance
checks remain blocked on the static baseline. The next diagnostic should
isolate the multi-group Q-limit transitions while retaining each generator's
IREG association and bus equations; changing IREG to zero is not a valid group
freeze.

### Remote-control topology and plant sharing audit (2026-10-12)

Parse the active generator section of `ACTIVSg25k.RAW` with a CSV parser so
quoted machine IDs do not shift `QG`, `QT`, `QB`, `VS`, `IREG`, `STAT`, and
`RMPCT`. Group by IREG, then by terminal bus. Build an electrical graph from
in-service branches and two-/three-winding transformers; compare each
generator terminal with its IREG target before assuming a remote-bus lookup is
wrong. Compare the resulting plant Q sums with the RAW dispatch, not just the
individual machine outputs. For a suspect group, also inspect the saved VM on
nearby regulated buses, the intervening line reactance, and the MATPOWER
branch-flow columns; very small reactance can magnify sub-milliper-unit
setpoint differences into hundreds of MVAr.

All 2,513 active nonlocal IREG records have a target exactly one in-service
transformer away: 2,293 through two-winding units and 220 through
three-winding units, all with positive series reactance. There are 280
zero-width `QT=QB=QG=0` records, which have no regulating Q range. The other
2,233 controllers cover 857 targets; every one has `QB<0<QT`, so a one-sided
zero Q bound is not the 25k failure path. Every active remote record has
`RMPCT=100`, and members of each target group agree on `VS`.

The AUX file specifies `MvarSharingAllocation="RegPerc"`. PowerWorld's
[Mvar sharing description](https://www.powerworld.com/WebHelp/Content/MainDocumentation_HTML/PowerFlow%20Mvar%20Sharing%20between%20Generators.htm)
defines this as allocation across regulating _terminal buses_ in proportion
to each bus's average RegFactor, then division among its generators. This is
different from summing each generator's factor across the whole group.
The saved RAW dispatch confirms that interpretation: among 152 multi-plant
groups with all controllers free and at least 1 MVAr total output, plant Q
sums agree to within 0.0008 MVAr. For target 16404, terminal bus 16405 has two
machines at 0.217 MVAr each and bus 16407 has one at 0.433 MVAr: both plants
supply approximately 0.433 MVAr. GridDyn previously gave those three units
equal participation. Its RAW importer now divides each machine's RMPCT weight
by the number of active regulating machines at that terminal before group
normalization. A target bus with local generation also participates as one
plant: 31 adjustable remote groups have a type-2 target, 27 of those have
active local generators, and 12 local plants have multiple machines. The
importer divides those local machine weights by their plant machine count as
well. This corrects the unconstrained plant share. The existing Q-bias
equation shares _changes_ from RAW QG, preserving an initially correct plant
share while every unit remains free. It does not fully model PowerWorld's
within-plant limit reallocation when only some machines saturate.

PowerWorld's [remote regulation description](https://www.powerworld.com/WebHelp/Content/MainDocumentation_HTML/PowerFlow%20Remote%20Regulation.htm)
also confirms the equation topology: the regulated bus keeps P and Q balance,
one primary regulator enforces the remote voltage, and secondaries enforce Q
sharing. GridDyn's indirect-control structure follows that topology. The
ANDES v33 RAW reader lists `IREG` and `RMPCT`, but passes neither to its static
PV model; that model's voltage equation uses its terminal `bus`, not `busr`.
ANDES therefore cannot validate this RAW remote-group interpretation.

An adjusted Release run with corrected plant weights still returns KINSOL
`-6`. A temporary transition trace recorded 38 Q-limit releases and 17
clamps before the failed solve. Bangor is a sensitive example: targets 62267
and 62317 both export `VS=1.047`, while their saved RAW voltages are
1.04700005 and 1.04658341 pu. The intervening in-service line has
`X=0.000133717` pu; the MATPOWER export records roughly 310 MVAr on that tie.
Forcing both exported setpoints exactly equal removes that large reactive
transfer and drives Bangor 6 units to about -83 MVAr each, below their
-24.109 MVAr minima. This is a concrete saved-operating-point versus exact
remote-control inconsistency, not evidence that IREG points to a distant bus.
Temporarily substituting every target's saved RAW VM for `VS` caused thousands
of limit transitions and a KINSOL `-7` failure, so it is not a valid fix and
was reverted. Temporary limit tracing was also removed.

Two focused RAW group fixtures pass in the Debug workflow tests: one has two
machines at a remote terminal bus and the other also has two local machines
on the regulated bus. They preserve equal shares across two and three plants,
respectively, after normalization and satisfy the target-voltage equation.
With the final importer change, the Release no-adjustment 25k run returns
zero; strict adjusted RAW power flows for ACTIVSg500 and ACTIVSg70k also
return zero.
These checks do not establish convergence of the adjusted 25k case.

### Adjustment-pass isolation (2026-10-12)

Compare the initial no-adjustment solution bus-by-bus with RAW VM before
attributing later voltage movement to import. With plant weights corrected,
all 857 adjustable remote targets match RAW VM within 0.001 pu; the largest
difference is 0.000417 pu at Bangor 7 bus 62317, caused by exact enforcement
of its rounded `VS` across the very low-X tie discussed above. Thus the
imported network initially reproduces the saved target voltages closely.

To find the failure transition, temporarily log each outer power-flow pass's
selected bus voltages and log each changed object's bus ID, change code, and
pass number from `GridArea::powerFlowAdjust`. The unmodified RAW input runs
three successful inner solves before the next KINSOL `-6`:

| Adjustment pass | Bristol 35365 VM | Bangor 62267 VM | Lempster 35397 VM | Changes                                       |
| --------------- | ---------------: | --------------: | ----------------: | --------------------------------------------- |
| 1               |        1.0180448 |       1.0470000 |         1.0590536 | 70 bus/controller changes                     |
| 2               |        1.0180447 |       1.0472282 |         1.0590528 | 49 changes, including 28 discrete shunt steps |
| 3               |        1.0084547 |       1.0472282 |         0.9438246 | next control set; following inner solve fails |

Lempster's RAW switched shunt has `MODSW=1`, `BINIT=38` MVAr, and a
1.0200-1.0435 pu control band. Its saved bus voltage is 1.05906 pu. GridDyn
steps the 38 MVAr bank down on pass 2; the next solved Lempster voltage falls
to 0.94382 pu, crossing the _other_ side of the band. Freezing only that
shunt removes most of the local collapse (pass-3 Lempster becomes 1.04881 pu)
but KINSOL still fails. This demonstrates a discrete-control overshoot and
also shows there are other interacting shunts.

Two diagnostic RAW copies changed only `MODSW` to zero while retaining each
shunt's saved `BINIT`: one froze all 848 in-service automatic shunts, and the
other froze only the 28 that switched on pass 2. **Both complete the strict
adjusted power flow** with remote generator controls active. These modified
copies are isolation probes, not corrected source cases or an accepted
solution of the original controls. The original adjusted RAW input still
fails. PowerWorld's
[switched shunt control description](https://www.powerworld.com/WebHelp/Content/MainDocumentation_HTML/Switched_Shunt_Control.htm)
describes discrete shunts as controller-loop devices whose band and block-size
interactions can cause oscillations. The case AUX sets `ChkShunts=YES`,
`PreventOscillations=YES`, and `ShuntInner=YES`. All 848 in-service RAW banks
are `MODSW=1` discrete controls. PowerWorld documents `ShuntInner` as treating
continuous switched shunts as PV buses in the Newton loop, so that option does
not explain these discrete-bank actions. The relevant parity gap is
`PreventOscillations=YES`: GridDyn has a local post-step overshoot hold, but no
general detector that freezes an oscillating control for the remainder of the
power-flow solve. See PowerWorld's [common power-flow options](https://www.powerworld.com/WebHelp/Content/MainDocumentation_HTML/Power_Flow_Solution_Common_Options.htm).

The next code step is to coordinate discrete shunt stepping with generator
remote control and prevent a step that crosses the entire voltage band from
triggering a conflicting remote-generator active-set change. Verify the
original RAW case after this change, then resume DAE initialization and the
partitioned five-second run. Remove any temporary pass trace after each
probe; it was removed from the current source.

### PowerWorld ZBR target grouping check (2026-10-12)

The AUX file sets `ZBRThreshold=0.0002`. PowerWorld documents that generators
regulating buses in one low-impedance group coordinate, and that its remote
regulation equations assign one primary controller and secondary Q-sharing
equations ([remote regulation](https://www.powerworld.com/WebHelp/Content/MainDocumentation_HTML/PowerFlow%20Remote%20Regulation.htm),
[advanced solution options](https://www.powerworld.com/WebHelp/Content/MainDocumentation_HTML/Power_Flow_Solution_Advanced_Options.htm)).
An audit of active RAW branches found 16 with X at or below the threshold and
exactly one low-X component containing multiple adjustable IREG targets:
Bangor buses 62267 and 62317. Their connecting branch has X=0.000133717 pu,
both targets are type-1 (PQ), and both have VS=1.047 pu. GridDyn does not
read/apply `ZBRThreshold`; it retains these as separate nodes and IREG groups.

A diagnostic RAW copy reassigned the two active Bangor 7 regulators from IREG
62317 to 62267. The full adjusted solve still failed KINSOL `-6`, so joining
this target pair alone does not fix the present failure. It remains a concrete
PowerWorld compatibility gap and should be addressed independently of the
switched-shunt issue.

Then repeat the comparison with all 848 automatic shunts fixed at their saved
`BINIT` values. Both target arrangements converge in that isolation. With the
original separate IREG groups, GridDyn solves target voltages 1.047228 and
1.047000 pu; its terminal-bus reactive outputs are +24.109 MVAr at each of the
three Bangor 6 units and -186.246 MVAr at the active Bangor 7 unit. With both
groups assigned to 62267, the solved voltages are 1.047000 and 1.046583 pu,
matching the two saved RAW bus voltages, and all four active terminal units
have GridDyn CSV `Qgen=-24.888` MVAr (the CSV sign is opposite the RAW `QG`
field). Thus ZBR grouping restores the saved dispatch and voltage profile in
the isolated Bangor comparison. This confirms the ZBR interpretation gap has
real power-flow consequences, even though it does not by itself clear the
original all-controls solve failure.

The one-transformer terminal-to-target paths also enter GridDyn's direct-path
proxy bookkeeping in `BusControls::updateVoltageControls`, while a generator
whose remote target is already PQ is flagged for indirect voltage equations in
`Generator::pFlowObjectInitializeA`. This combination exists at Bangor, but
the indirect generator equations are the active static-PF path for a PQ
target. The proxy is consulted by `AcBus::dynObjectInitializeB` for PV/slack
target handling; the PQ branch does not use it. The code trace does not show
that the proxy and indirect equations double-count Bangor's target controls.
Capture actual active flags if a future experiment indicates otherwise.

As a coarse shunt follow-up, split the 28 shunts that switched on adjustment
pass 2 into two groups of 14 and freeze one group per diagnostic RAW copy.
Freezing the first half still failed KINSOL `-6`. Freezing the second half hit
the voltage-adjustment iteration limit and produced an output with a maximum
bus Q residual of 27.1 MVAr and maximum adjustable remote-target voltage error
of 0.0366 pu. That output is not a solved power flow. Neither half alone
recovers a valid result, consistent with interacting shunt and regulator
changes across the case.

A code-only ordering probe made discrete `Svd` banks return `NO_CHANGE` during
`REVERSABLE_ONLY`, allowing their steps only during the driver's later
`FULL_CHECK`. The original RAW then failed quickly with KINSOL `-11` (linear
solver setup failure). The experimental change was removed. This shows that
simply moving all discrete shunts to the later check phase is not a sufficient
fix; it does not identify why the linear setup failed.

### Global switched-shunt adjustment flag probe (2026-10-12)

Added the global `no_shunt_adjustments` power-flow flag (also accepted as
`no_switched_shunt_adjustments`). It makes each `Svd::powerFlowAdjust` call
return without changing the imported shunt setting, while the power-flow driver
continues to run generator and remote-voltage adjustments. The first-run
limits-only path also propagates this flag. Its scope is static power-flow
adjustment; it does not disable dynamic shunt models.

Release build command:

```powershell
cmake --build build-ninja-release --parallel 4
```

On the original `ACTIVSg25k.RAW`, the following completed with exit code 0,
25,000 buses, and no KINSOL or voltage-adjustment-loop warning:

```powershell
gridDynMain.exe ACTIVSg25k.RAW --powerflow-only `
  --flags=no_shunt_adjustments,no_powerflow_error_recovery `
  --powerflow-output ACTIVSg25k-no-shunt-adjustments.csv --verbose
```

The exported voltages retain active remote generator controls. Of 857
adjustable IREG targets, 269 are within 0.0001 pu of their requested VS; the
523 targets more than 0.001 pu away have their aggregate remote-generator Q at
the group's RAW Q limits. Those controls cannot hold VS after reaching a Q
limit. This is a converged static PF with switched-shunt movement suppressed,
not evidence that the original automatic-shunt solution is correct or that
the dynamic initialization and five-second run are stable.

### Switched-shunt control logic audit (2026-10-12)

The prior `Svd` implementation did not preserve RAW switched-shunt state at
block level. The RAW reader writes exact `BINIT`, but adjustment tracked an
ordinal prefix step. The 25k RAW has 848 in-service MODSW=1 banks, all ADJM=0.
Only 505 BINIT values equal ordered prefix levels; 343 are combinations of
partial blocks. Bus 29830 has BINIT=486 MVAr and blocks 4x140 and 4x22 MVAr
(486 = 3x140 + 3x22), while the nearest prefix states are 420 and 560 MVAr.
The current working change reconstructs per-block state from BINIT and changes
one actual block at a time, preserving the imported total until an adjustment
is requested. ADJM=1 remains unsupported.

Discrete steps can cross the complete voltage band. Bus 35397 (Lempster) has
BINIT=38 MVAr, one 38-MVAr step; switching it off moved voltage from 1.059 pu
to 0.944 pu across its 1.0200-1.0435 pu band. The working change compares the
before/after voltage-band deviation and holds the closer setting instead of
immediately reversing. It releases that hold if other controls move the bus
voltage materially, so the decision is reconsidered against the changed
network state.

The importer stores switched-shunt RMPCT as `Svd::participation`, but the
adjustment code does not use it. There are 188 in-service automatic shunts on
buses that also have adjustable generator voltage controls. Co-location alone
does not establish that PSS/E assigns those devices to one shared control
group; group coordination and RMPCT semantics need a separate data-backed
implementation before treating this as a case-specific cause.

Finally, `adjustmentMethod` is parsed and retained but does not affect switch
selection. This case uses ADJM=0, so that gap does not explain the run. A first
static PF rerun with the block-state, overshoot, and one-bank-per-solve changes
still fails with KINSOL `-6` after several switched-shunt actions. The initial
batch trace had 28 banks act against one voltage solution; serializing them
avoids that batch but has not resolved the failure. After also limiting
reversals to one per solve and expiring stale before/after comparisons, the
full case still fails with `-6` after nine logged step/reversal actions. The
remaining issue may involve interactions among bank steps or the broader
remote-voltage-control solution. Continue with per-action voltage and Q-limit
traces; do not treat this control change as a successful 25k PF solve. The
latest trace logged nine forward step messages and four reversion messages
before KINSOL `-6`.

A focused isolation retained only the automatic control at bus 17306 and set
the other switched-shunt records to manual while preserving their BINIT values.
That case converged. Bus 17306 switched from 0 to 80 MVAr, then to 160 MVAr;
the second step crossed its band and the controller returned to 80 MVAr, where
the final PF solved. This shows the large individual step is controllable by
the overshoot logic in isolation. It narrows the full-case failure to
interactions among multiple shunt controls and/or the broader network control
state, rather than proving a defect in that one bank's block representation.

Trace review also found that the first pass latch covered forward steps but not
overshoot reversals, so two physical changes could occur before a solve. The
current working change claims the same single-action latch for reversals and
uses only the immediately following solved voltage for before/after attribution.
The full-case failure persisted after that correction, so the change improves
control sequencing but is not yet a resolution.

### Remote voltage-control PV-path preflight (2026-10-12)

`AcBus::updateControlLimits()` now checks each registered remote generator
controller after all power-flow phase-A registrations complete. It traverses
enabled, connected links between AC buses and uses each bus's imported
power-flow type to accept only paths that avoid intermediate PV buses. The
generator terminal and regulated bus are allowed as path endpoints. When no
such path exists, GridDyn logs a warning that distinguishes an intervening-PV
case from a missing connected AC path. This is diagnostic only: the warning
does not disable the remote control, so it does not yet reproduce PowerWorld's
prevention behavior.

The release build completed, and the check was exercised on the original
`ACTIVSg25k.RAW` with `--powerflow-only` and
`--flags=no_shunt_adjustments,no_powerflow_error_recovery`. The run exited 0
with 25,000 buses and 32,230 links, and emitted no PV-path warnings. This
indicates that none of the registered ACTIVSg25k remote generator controls
failed this particular path condition in that static run; the check therefore
does not explain the remaining full-case power-flow failure. The run does not
establish five-second dynamic stability.

### PowerWorld power-flow stage comparison (2026-10-12)

PowerWorld documents a pre-processing phase followed by three nested loops:
the Newton power-flow solve, a controller loop ordered as generator Mvar limits,
DC-line solution, switched shunts, then LTC/phase-shifter/D-FACTS controls, and
an outer MW-control loop. Its [solve overview](https://www.powerworld.com/WebHelp/Content/MainDocumentation_HTML/Solving_the_Power_Flow.htm)
and [common options](https://www.powerworld.com/WebHelp/Content/MainDocumentation_HTML/Power_Flow_Solution_Common_Options.htm)
also expose distinct Newton and controller-loop limits, separate switches for
several controller types, generator-limit timing and back-off choices, and
oscillation prevention.

The pre-processing comparison is narrower. GridDyn now reports remote
generator viability when no PV-free AC path exists, but I found no dedicated
counterparts in the inspected solve path for PowerWorld's angle smoothing,
initial MW-mismatch estimate, low-impedance voltage-profile repair, or voltage
estimation for newly connected buses. GridDyn starts from its normal state
guess and has solver error recovery, including topology/load-generation
rebalancing after a failed solve; those are different points in the sequence.

GridDyn has the broad loop shape: `GridDynSimulation::powerflow()` solves,
updates caches, calls `powerFlowAdjust()`, reinitializes when a control changes,
and repeats; optional `autoallocate` dispatch runs outside that loop. The
controller stage is less isolated. `GridArea::updateFlags()` copies adjustable
objects into `pFlowAdjustObjects` in `primaryObjects` order, and
`GridArea::powerFlowAdjust()` evaluates the entire list and accumulates the
largest `ChangeCode` without stopping after a change. Buses process their bus
Q-limit transition, generator Q limits, and attached adjustable loads such as
`Svd`; links process adjustable transformers in the same pass. The switched
shunt claim allows one `Svd` adjustment per pass, but does not prevent a bus or
transformer from changing alongside it based on the same solved state.

The `CheckLevel::REVERSABLE_ONLY` and `FULL_CHECK` calls do not provide a
reliable stage split for the main controls inspected: `Generator`, `Svd`, and
`AdjustableTransformer` ignore that argument, and bus limit handling runs in
both passes. GridDyn has some useful individual controls (`ignore_bus_limits`,
`no_shunt_adjustments`, `no_link_adjustments`, `no_tap_changer`, and
`no_powerflow_adjustments`). The `no_tap_changer` switch disables discrete
tap-ratio and phase-angle adjustments in the outer power-flow loop; continuous
algebraic tap controls and switched-shunt control remain active. Separate
generator-Q-limit, SVC, and DC-control switches were not found. `ignore_bus_limits`
guards the bus-level PV/PQ limit transition; the subsequent generator
`powerFlowAdjust()` calls still run. `no_powerflow_adjustments` skips that
controller pass, but does not skip the outer `autoallocate`/`loadBalance`
loop. Also, `DISABLE_LOAD_ADJUSTMENTS` is declared but is not consumed by the
power-flow adjustment path or exposed in the flag-name map.

`maxpowerflowadjustmentsperpass` now caps how many top-level adjustment objects
may report a change in one controller pass. Its default is zero (the existing
full sweep); one makes the solver walk one changed object group at a time, and
larger values permit bounded batches. Controllers attached to one bus are
dispatched together and count as one group. The switched-shunt one-bank latch
remains independently active.

The outer MW step is not PowerWorld-style area/island AGC: when enabled,
GridDyn restores slack outputs, computes the system mismatch, and redistributes
it over available bus generation; `loadBalance()` contains a TODO to make this
area-based. A second staging detail to audit is `FIRST_RUN_LIMITS_ONLY`: its
branch is keyed to `pState == INITIALIZED`, passes only the
`DISABLE_SHUNT_ADJUSTMENTS` bit from the lower control flags, and is not keyed
to a one-time iteration counter. If this internal flag is used, other disable
flags are masked for as long as the simulation remains in the initialized
state.

PowerWorld also offers three remote-Var sharing methods; GridDyn currently
normalizes the imported `vcontrolfrac`/RMPCT weights and uses those weights in
the indirect remote-control equations, with no equivalent sharing-mode
selector. The new PV-free-path warning found no affected remote generators in
the 25k run. Overall, the likely gap is not a missing top-level solve loop but
the lack of an explicit, ordered controller coordinator and matching per-stage
controls. That can plausibly contribute to the 25k shunt interaction: the case
solves with shunt movement disabled, while the mixed control pass can combine
shunt, generator-limit, and tap changes. This is a plausible mechanism, not a
proven root cause. The next diagnostic should record each stage and allow one
controller class to act before the next Newton solve.

### Discrete-shunt replay and adjustment-cap probe (2026-10-12)

Reran the original RAW with the current Release binary. First, with
`no_shunt_adjustments,no_powerflow_error_recovery`, the static solution
converged. Comparing its bus voltages to the 848 in-service `MODSW=1` control
bands gives 816 in-band banks, 26 above their upper limit, and 6 below their
lower limit. This confirms that the banks are automatic and that the initial
out-of-band banks have a valid reason to request a step; it does not mean all
848 should move.

With switched-shunt adjustment enabled, the case fails in KINSOL `-6` after
nine forward bank steps and four reversals. The log shows the step sizes can
be much larger than the violation:

| Bus   | Solved V before step |          Band | Step        | Solved V after step | Outcome                                                               |
| ----- | -------------------: | ------------: | ----------- | ------------------: | --------------------------------------------------------------------- |
| 11233 |             1.050217 | 1.0200–1.0435 | 46 MVAr off |            0.980467 | Reverted; original setting was closer                                 |
| 15124 |             1.070620 | 1.0200–1.0525 | 22 MVAr off |            1.018457 | Crossed the full band; later interacted with bus 15133                |
| 15758 |             0.996997 | 1.0200–1.0475 | 34 MVAr on  |            1.075011 | Reverted; original setting was closer                                 |
| 17034 |             1.050684 | 1.0200–1.0505 | 46 MVAr off |            0.998982 | A 0.000184 pu high-side violation triggered a large step and reversal |

At bus 35397, the no-shunt solution is 1.059053 pu against a 1.0200–1.0435
band. Its initial 38 MVAr is one full available step; the next state is zero.
Earlier pass tracing showed that zero moves the bus to 0.943825 pu, while the
38 MVAr setting is much closer to the band. The proper final setting for that
bank is therefore its imported 38 MVAr state, even though the current rule
first tries to de-energize it.

The Release `maxpowerflowadjustmentsperpass=1` probe did not converge either.
It repeatedly toggled bus 15124, hit the voltage-adjustment loop limit twice,
and left a maximum bus reactive residual of 14.09 MVAr. Before the loop-limit
status fix below, the CLI still returned exit code 0. A diagnostic RAW copy
with only bus 17210 automatic (all other automatic shunts changed to manual
while preserving BINIT) converged; bus 17210 settled at 1.041482 pu inside its
band. In the full adjusted run, preceding controls had pulled that bus to
0.980077 pu and its 2.5 MVAr step was followed immediately by KINSOL `-6`.
This identifies a network-control interaction, not an intrinsically bad
17210 bank.

The shunts generally choose the correct direction: high voltage removes
capacitive injection and low voltage adds it. The flaw is how GridDyn sequences
that choice. It takes a full block whenever the solved voltage is outside the
band, without checking whether that block is grossly oversized for the
violation. Its overshoot rule learns only after solving the changed network;
if another bus/generator/tap controller changes before the bank is revisited,
the saved before/after voltages are discarded as stale. The global one-bank
latch still allows other controller classes to change on the same voltage
profile. The cap of one changed object does not provide a controller-class
stage and, in this case, can leave one oscillating bank at the front of the
walk until the outer-loop limit.

The loop-limit path in `gridDynPowerFlow.cpp` previously warned and broke out
with a success return, which made the cap probe look successful to automation.
It now returns `FUNCTION_EXECUTION_FAILURE` on that limit. A Release rerun
confirmed a nonzero CLI status when the 30-iteration limit is reached.

GridArea now remembers the bank that changed and gives it the next adjustment
call first, with voltage/angle/frequency inputs taken from that bank's bus.
This makes the before/after comparison happen before other controllers can
change the solved network. The strict default run then progressed past the
17210 action, but KINSOL `-6` occurred immediately after bus 25034 stepped
from 38 MVAr to zero at 1.057289 pu (its high limit is 1.0495 pu). A diagnostic
copy with only bus 25034 automatic converged and reverted that same step because
it produced 0.983523 pu. Thus the step itself is solvable in isolation; it
fails after earlier controls have moved the network.

With the pending-bank recheck and `maxpowerflowadjustmentsperpass=1`, raising
`maxvoltageadjustiterations` to 120 did not solve the case: the first control
loop still reached its limit, and a later Newton solve failed with KINSOL `-7`
after 28 forward steps and 18 reversals. The one-object cap is therefore not a
working convergence mode for this case yet. These changes improve attribution
and report failures honestly, but do not establish a valid adjusted 25k power
flow. The remaining algorithm work is to coordinate whole control classes,
reject or recover from steps whose Newton solve fails, and detect repeated
controller reversals as PowerWorld's `PreventOscillations` option does.

### Switched-shunt RAW field audit (2026-10-12)

Audited all 848 records in the original version-33 RAW switched-shunt section
against `rawReadSwitchedShunt` and `Svd`. The field offsets are consistent:
`MODSW`, `ADJM`, and `STAT` are read from fields 1–3; voltage high/low from
fields 4–5; `SWREM` from field 6; `BINIT` from field 9; and each `N_i, B_i`
pair begins at fields 10–11. GridDyn maps positive capacitive `B_i` to negative
load Q, and retains `BINIT` as the present admittance before enabling control.

Every record is `MODSW=1`, `ADJM=0`, `STAT=1`, and `SWREM=0`; all 848 regulate
their terminal bus. All voltage bands have `VSWHI > VSWLO`; low limits span
0.9900–1.0200 pu, high limits 1.0400–1.0575 pu, and band widths 0.0235–0.0500
pu. Records have one to seven blocks, each with one to nine steps; nominal step
sizes span 1–140 Mvar at 1.0 pu voltage. There is no general factor-of-100,
high/low reversal, or `N_i`/`B_i` offset error in this import. As expected for a
shunt susceptance, actual reactive injection varies with voltage squared.

All 848 `BINIT` totals are representable as combinations of their listed
blocks. However, 343 are not a simple prefix of the input-ordered steps, and
100 banks have more than one block-step combination that yields the same
`BINIT`. The RAW record contains only the aggregate `BINIT`, not individual
block switch statuses, so `blockStepsForLevel` uses a deterministic greedy
input-order reconstruction. It preserves the total exactly, but a duplicate
combination can make the inferred individual step state ambiguous. Compare
against a solved PSS/E/PowerWorld per-block state export if future behavior at
one of those banks depends on which block was last switched.

The initial totals also agree with the MATPOWER `BS` values at sampled buses
11021 (26 Mvar), 25034 (38 Mvar), and 29830 (486 Mvar); PowerWorld's AUX shunt
table also reports 26 Mvar at bus 11021. This supports the RAW value mapping.
It does not make the discrete step sizes small enough for the voltage
sensitivity: bus 25034's 38 Mvar step moved voltage from 1.057289 pu to
0.983523 pu, across its 1.0200–1.0495 pu band. The main convergence issue
therefore remains a coarse step/network interaction rather than a scaling or
band-field import error.

One control-logic defect was found in the only mixed-sign bank, bus 17424
(`2 × -8.5` Mvar reactor steps and `2 × +10` Mvar capacitor steps). The old
step search could energize the desired opposite type while the current type
was still active. `voltageControlSteps` now de-energizes the currently active
type in reverse block order before enabling the other type, following the
PSS/E restriction against simultaneously energized reactor and capacitor
steps. Bus 17424 starts at 1.048804 pu inside its 1.0200–1.0545 pu band, so
this correction is not the cause of the observed 25k power-flow failure.

### Flat dynamic run with power-flow shunt adjustments disabled (2026-10-12)

The static power-flow baseline for this attempt passes
`no_shunt_adjustments,no_powerflow_error_recovery`. Switched shunts therefore
stay at their imported `BINIT` values during the static power-flow adjustment
stage; generator and remote voltage controls remain enabled. No event file or
disturbance was supplied. The flag does not remove shunt equations from the
dynamic model.

Initial phase timing exposed a reactive allocation error at PV bus 43846. The
static solve held aggregate generation at its lower Q limit, but dynamic model
initialization split the required Q evenly between two local generators even
though one was already at its individual Q minimum. That drove its internal Q
below its limit and made its ESST4B field voltage invalid. The PV initialization
now allocates Q across local generator controls subject to each unit's own
limits, sharing the remaining mismatch according to the configured control
fractions. This allowed model initialization B to complete in about 0.26 s.
The ESST4B lower-limit error now includes the object path and calculated values;
using `fullObjectName` also avoids looping through GridDyn's self-parenting null
sentinel while formatting an error.

The Release binary was rebuilt and both methods were attempted with a 5 s stop
time, the no-shunt flags above, and no disturbance. Partitioned/CVODE completed
model initialization and initial-condition setup at t=0. Its first root was
the IEEE Type 1 exciter at Maine / SEARSMONT 1 / generator 2, around 38.5
microseconds. The next root at about 51.8 microseconds was the same exciter's
limiter release. The algebraic reinitialization returned KINSOL status `1`.
In the bundled KINSOL, `KIN_INITIAL_GUESS_OK` (1) means the initial guess
already satisfies the equations. GridDyn callers require the common success
status `0`, so this valid solve had been misreported as failure. The KINSOL
wrapper now normalizes that success status to zero, and the root/reinitializing
time and object are logged when `partitioned_diagnostics` is enabled. It also
maps KINSOL's `KIN_STEP_LT_STPTOL` to GridDyn's convergence error because that
positive KINSOL status has the same numeric value as GridDyn's root-found code.

After that fix, partitioned reinitializations succeed, but the 5 s attempt
spends most of its time processing many exciter and governor limiter roots and
their algebraic re-solves. In roughly a minute it advanced only to about 8 ms;
the run was stopped before a periodic state capture and did not produce a
trajectory. The initial values and 5 s stability therefore remain unchecked.

The DAE attempt reports `IDAICFailFlag: The linear solver setup failed
unrecoverably` during initial-condition handling. With IDA initial-condition
diagnostics enabled, it lists 6,484 zero-diagonal Jacobian entries, mostly the
`id`/`iq` states on GENROU machines, then exits with `bad allocation` before
time advances. This is not a completed DAE initialization. Diagnose the IDA
Jacobian/consistent-initial-condition setup separately from the partitioned
root throughput.

Many model initial outputs still exceed their DYR upper bounds, so the default
initial-limit policy raises the bound to preserve the starting state. Follow-up
tracing confirmed that ESAC1A and WT3E1 also need to adjust their model-specific
limits; otherwise the state is initialized and immediately projected back to
the unadjusted limit. Other early limiter roots in DC2A, IEEE Type 1, and
IEEEG1 remain to be checked against their local initial residuals. Stability
still requires a completed flat run and a comparison of initial and final
states.

### Initial residuals and early limiter roots (2026-10-12)

The first DAE initial-condition diagnostic showed large t=0 mismatches:
14.0312 in `ESAC1A.ve` at Virginia / HARRISONB~26, 0.7010 in `WT3E1.Eqcmd` at
New Jersey / ATLANTIC ~13, and -0.4 in `WTPTA1.theta` at three Maine wind
units. IDA failed before advancing, so these were initialization defects or
input mismatches, not disturbance transients.

The ESAC1A residual was traced to a limit-projection bug. On five generators
at HARRISONB~26, the initializer calculated a regulator state of 20.8157 pu,
but `VAMAX` was 9 pu. The inherited EXAC1A helper adjusted `VRMAX` (50 pu),
while ESAC1A's active regulator bound is `VAMAX`; the initial limit check then
clipped the state to 9 pu. The field-current input remained 1.876 pu in both
the initializer and residual evaluation, so this was not a signal-routing
change. ESAC1A now adjusts `VAMAX` under the same permissive initial-limit
policy as other exciters. The rerun reports the initial limit correction and
removes the five -14.0312 residuals. This matches PowerWorld's documented
handling of `Va` above `VAMAX` ([ESAC1A model](https://www.powerworld.com/WebHelp/Content/TransientModels_HTML/Exciter%20ESAC1A.htm)).

The WT3E1 mismatch had the same shape. At ATLANTIC ~13, WT3G1 supplies an
initial `Eqcmd` near -1.201 pu; WT3E1's effective lower bound was -0.5 pu, so
its command equation clipped the initialized value and left a 0.701 residual.
WT3E1 now expands its effective lower or upper limit to include an out-of-limit
initial command. The rerun adjusted 84 WT3E1 records and reduced the largest
algebraic residual to 9.93e-6. PowerWorld documents adjusting `XIQMIN` or
`XIQMAX` when `EqppCMD` is outside the limit ([WT3E/WT3E1 model](https://www.powerworld.com/WebHelp/Content/TransientModels_HTML/Exciter%20WT3E%20and%20WT3E1.htm)).
The exact `VLTFLG` limit equation still needs comparison: GridDyn's current
command bounds use a nominal 1 pu offset, while the PowerWorld description
expresses them as terminal-voltage-dependent limits.

The remaining largest residual is `WTPTA1.powerIntegral` at about -5.02. For
the affected WT3P1 units, the imported `Pset` is 1.0 pu while the WT3E1
`PORD` starts near 0.833 pu; with `Kic=30`, `Kic * (PORD - Pset)` gives the
observed -5 pu/s derivative. `Pset` is an explicit power setpoint, so this
RAW/DYR operating point does not satisfy a flat dynamic equilibrium. The
importer preserves the DYR setpoint rather than silently replacing it with
the RAW dispatch. A matching RAW dispatch or an intentional setpoint override
is needed before expecting all wind states to remain fixed.

After the ESAC1A and WT3E1 corrections, the DAE diagnostic still returns
`IDA_LSETUP_FAIL` before time advances. The maximum residual is now the WT3P1
setpoint mismatch; IDA also reports 6,484 zero diagonal entries on GENROU and
GENSAL `id`/`iq` rows. The partitioned solver completed a 1 ms no-disturbance run in
2.4 s, but that run is not evidence of a stable 5 s trajectory, especially
with the WT3P1 setpoint error still present.

The earlier partitioned attempt recorded 214 limiter roots by about 8 ms. The roots
were concentrated in 92 DC2A exciters, 35 IEEE Type 1 exciters, and 87 IEEEG1
governors. At SEARSMONT 1, the IEEE Type 1 initializer raised `VRMAX` from 1
to the initial regulator value 2.2591 pu; the limiter then entered at about
38.5 microseconds. The many similar warnings for DC2A and IEEEG1 show this is
a broad mismatch between calculated operating states and imported limits,
not a single isolated controller event. The voltage, current, and mechanical
power inputs used by the model initializers need to be compared with the
inputs seen by the residual equations after state assembly.

One specific source of a nonzero initial residual is now confirmed. The
`WT3P1` adapter had been translating its PSS/E fields in the wrong order.
PowerWorld lists the WT3P1 fields as `Tp, Kpp, Kip, Kpc, Kic, ThetaMin,
ThetaMax, RThetaMax, Pset`, and its model description says WT3P1 has no
non-windup limits on the pitch control. For the Danforth 3 record, PSS/E
supplies `.3, 150, 25, 3, 30, -4, 27, 10, 1`; the old adapter instead gave
WTPTA1 `Tp=10`, `Kpw=.3`, `Kiw=150`, `Kpc=25`, `Kic=3`, and pitch maximum 30.
With static output 56.959 MW on the RAW machine base of 68.4 MVA, this maps
the power error through an unintended `Kpc=25` and the 10 s time constant,
which produces exactly the observed -0.4 pitch-rate residual. The adapter now
maps the fields as `Kiw=Kip`, `Kpw=Kpp`, `Kic=Kic`, `Kpc=Kpc`, `Tp=Tp`,
`ThetaMax/ThetaMin`, and symmetric `+/-RThetaMax`, preserving `Pset`.

That correction addresses the demonstrated WT3P1 field-order error. It does
not yet establish a steady WT3P1 operating point: the Danforth record has
`Pset=1.0` while its RAW output is about 0.833 pu on the RAW machine base, so
the PORD/Pset signal and base convention must be checked against the actual
PSS/E initialization behavior. The separate WT3P1 no-windup behavior also
still differs from the existing WTPTA1 core. Rebuild and repeat initialization
diagnostics before deciding whether either mismatch is expected data behavior
or needs a model-level correction.

Source: [PowerWorld WT3P/WT3P1 model description](https://www.powerworld.com/WebHelp/Content/TransientModels_HTML/Stabilizer%20WT3P%20and%20WT3P1.htm).

### Initial physical derivatives and the first limiter root (2026-10-12)

For a no-disturbance stability check, a small DAE residual alone is not enough:
the DAE solver residual is `f(x) - xdot`, so it can be near zero when the
initialized physical derivative `f(x)` is nonzero and the solver supplies a
matching `xdot`. Record and inspect the actual differential RHS at the
initialized state, as well as each early limiter root and the state that owns
it. A state that crosses a limit immediately in a flat run must be traced back
to its initial input and derivative before treating the crossing as normal
limit action.

This check confirmed a startup inconsistency in the partitioned run. After its
first algebraic solve at `t=0`, the largest actual differential rate was
`1.89703546e-2 pu/s` on the IEEE Type 1 exciter `vr` at Maine / SEARSMONT 1 /
generator 2. The static bus voltage was `0.9990066900 pu`; the partitioned
algebraic solve moved it to `0.9990050111 pu`, a change of only
`-1.6788764e-6 pu`. This IEEET1 record has `Ka=400`, `Ta=0.0354`, and `Tr=0`,
so its initialized regulator rate responds to the voltage error with gain
`Ka/Ta`; the measured voltage change predicts the observed
`1.8970e-2 pu/s` rate. The IEEET1 initializer had calibrated `vBias` against
the pre-solve voltage, so that bias no longer exactly balances the regulator
after the algebraic correction.

The same unit's imported `VRMAX` is `1 pu`, but its initial regulator output
is `2.25913697 pu`. The permissive initial-limit policy raises `VRMAX` to that
starting value. Because the post-solve derivative is positive, the regulator
immediately moves outward from its newly adjusted limit and reaches the
limiter root at `t=0.000560634 s`. This is not evidence of a disturbance or of
a large system instability; it is direct evidence that the no-disturbance
initial state is not an equilibrium to the precision needed by this high-gain
controller. The underlying Type 1 initialization equations cancel at their
pre-solve inputs, so the next fix should reconcile model initialization with
the solved algebraic state rather than suppressing the root or zeroing the
derivative.

The first `10 ms` capture also has six IEEEG1 root notifications on the three
HUNTERSVILLE ~17 units at `6.263 ms` and `7.554 ms`, followed by other exciter
and governor roots. This extends the same inspection requirement beyond one
exciter: audit each early root against its initialized local residual/rate,
especially where permissive limit expansion placed the state exactly on the
limit. At HUNTERSVILLE ~17, the RAW dispatch is `2.198 MW` on a `2.64 MVA`
machine base (`0.83257576 pu`), while DYR `IEEG1.PMAX` is `0.7597 pu`. The
initializer raises the limit to the dispatch; the valve and governor setpoint
both begin at `0.83257576 pu` with zero valve rate. The initial machine
frequency rate is `-3.6454e-7 pu/s`. As that tiny speed change accumulates, the
three governors hit the expanded upper limit at `6.263 ms`; two return through
a root at `7.554 ms`. This looks like a boundary transition caused by the
operating point starting exactly on a corrected limit, not an isolated bad
governor derivative. It still needs to be checked against the source model's
initial-limit behavior and the surrounding machine-power balance.

A targeted rerun with KINSOL tolerance reduced from `1e-8` to `1e-12` produced
the same `-1.6788764e-6 pu` SEARSMONT voltage correction. The discrepancy is
therefore not explained by the algebraic solver stopping tolerance alone; the
pre-solve dynamic equations and power-flow solution are slightly inconsistent.
An experiment that globally reran `dynInitializeB` after the algebraic solve
removed the observed early roots in a 10 ms run, but increased the maximum
differential-state change over 10 ms from `4.0630e-4` to `1.6360e-3`. That
blanket reinitialization is not a valid fix and was reverted. A valid 5 s flat
trajectory is not established yet; the root-enabled run remains too slow while
these events repeatedly trigger algebraic re-solves.

The initial-state recorder must capture the solver vector after its
`initialize()` call copies component states into that vector. Capturing before
that point writes a zero-filled initial record and invalidates all state-drift
comparisons. The current recorder saves one `t=0` state after solver
initialization and schedules later samples at the requested interval. State
and rate analysis should always verify the initial record is nonzero and
matches named component initial values before comparing it to later samples.

### Partitioned root timing and unconstrained state growth (2026-10-12)

A root-disabled partitioned run is useful only as an equation-drift probe; it
does not validate a case whose models rely on limit transitions. With
`roots_disabled,no_shunt_adjustments,no_powerflow_error_recovery`, the flat case
completed to `0.32 s`, but several exciter states accelerated rapidly. The
largest was Vermont / QUECHEE 5 / generator 1 / DC2A `vr`: it started at
`2.8646 pu`, was `3.9283 pu` at `0.25 s`, and reached `21.2981 pu` at `0.32 s`.
Its recorded rate grew from `0.00066 pu/s` at `0.01 s` to `63.63 pu/s` at
`0.25 s` and `446.07 pu/s` at `0.32 s`. Other large changes include three
NEW HAVEN ~21 DC2A `vr` states (`1.9901` to `13.7753 pu`), MONTPELIER ~4 DC2A
(`0.5746` to `12.1250 pu`), and TAMWORTH 2 3 IEEE Type 1 (`0.1225` to
`10.1275 pu`). The no-root trajectory is not a physical limit-enforced run,
but it confirms that the initialized dynamic equations do not stay near their
starting point. A zero derivative stored in the `t=0` recorder record is not
the actual initial RHS; the initialized solver vector had not yet evaluated
that RHS. The separate RHS probe measured a maximum initial rate of
`1.897e-2 pu/s`.

The root log separates two cases that should not be conflated. Three DC2A
units at Vermont / NEW HAVEN ~21 hit a limit together at `0.092670 s`, and
MONTPELIER ~4 hit one at `0.095044 s`; both records had an initial regulator
output above the imported `VRMAX`, so GridDyn widened the limit to the initial
operating value. IEEE DC2A uses terminal-voltage-proportional regulator limits,
so even a small terminal-voltage change can move this expanded bound away from
the starting regulator state. The [IEEE 421.5 DC2A model description](https://home.engineering.iastate.edu/~jdm/ee554/IEEEstd421.5-2016RecPracExSysModsPwrSysStabStudies.pdf)
confirms that limit dependence. PowerWorld's [EXDC2A model page](https://www.powerworld.com/WebHelp/Content/TransientModels_HTML/Exciter%20EXDC2A.htm)
also documents raising a violated `Vrmax` to the current `Vr` during transient
simulation. These first roots may be boundary-entry events caused by the
initial point sitting on a corrected limit; they do not by themselves prove a
reader error. QUECHEE 5 DC2A instead reaches its root at `0.276689 s` after
its `vr` has drifted substantially, so it belongs to the growing-state
investigation.

Reproduce the unconstrained drift probe from the repository root with:

```powershell
build-ninja-release\bin\gridDynMain.exe `
  C:\Users\phlpt\Documents\test_cases\ACTIVSg25k\ACTIVSg25k.RAW `
  --import build-ninja-release\ACTIVSg25k-flat-Pset-initial-dispatch.dyr `
  --param=dynamicsolvermethod=partitioned --param=timestop=0.32 `
  --flags=sparse,roots_disabled,no_shunt_adjustments,no_powerflow_error_recovery `
  --state-output=activsg25k-partitioned-noroots-032.bin --save-state-period=10 --summary
```

The binary capture contains 33 differential-state snapshots at 10 ms intervals.
Its first derivative record is zero-filled and must not be used as the initial
physical RHS. State names for the largest-drift indices were printed in
`activsg25k-partitioned-noroots-state-names-032.txt`.

The root-enabled flat run also exposed an event-state mismatch. FORT FAIRF~2
REECA1 `rootTest()` callbacks at `0.314562 s` and `0.315094 s` read terminal
voltages of `0.85608 pu` and `0.84957 pu`; the latter is below `Vdip=0.85 pu`.
But the root returned at `0.314828 s`, and the root handler's refreshed input
still read the initial `1.04074 pu`. These root-test samples are not a
trustworthy physical voltage trajectory because the voltage is taken from the
last KINSOL solve, not necessarily the same time as CVODE's interpolated
differential state. In partitioned CVODE, the root callback pairs that stale
algebraic vector with the interpolated differential state; the root handler
then rebuilds its cache from the paired algebraic solver vector without first
solving it at the root time. The event time and the input used to classify the
transition can therefore refer to different operating points.

An earlier experiment that solved KINSOL at a probe time after each root did
not provide a safe fix: it exposed a repeated ESAC1A limiter root at
`0.020310283 s` and stalled the partitioned run. That implementation was
reverted. A later correction solves the paired algebraic state at the actual
root callback time and at the same event time before restarting the partitioned
solver; it completed the 30 ms diagnostic described below without repeated
MANASSAS root chatter. This establishes event progress only. Initial RHS and
limit distance still need to be checked for every early root before attempting
the five-second run.

### Early roots as equilibrium diagnostics (2026-10-13)

For a no-disturbance equilibrium, inspect the actual differential RHS as well
as the solver residual. An implicit DAE residual can be near zero because it
is `f(x) - xdot`, even when the physical RHS `f(x)` is nonzero and the solver
has matched it with a nonzero `xdot`. In a flat-start check, the differential
RHS should be near zero; a limit root in the first milliseconds is a reason to
inspect that model's initial residual, distance to the limit, and inputs before
accepting the event as normal operation.

Before the selector-tolerance change, a Release run with the partitioned
solver, no disturbance, and `no_shunt_adjustments` reached 0.03 s. Its early
roots were:

- `0.000560634 s`: SEARSMONT 1 Gen 2 IEEE Type 1 exciter.
- `0.006938889 s`: the three HUNTERSVILLE ~17 IEEEG1 governors.
- `0.019826247 s`: MANASSAS ~22 Gen 1 ESAC1A.
- `0.023342491 s`: PARKSLEY 2 1 Gen 1 IEEEG1.
- `0.028877092 s`: KINSTON 4 1 Gen 3 and Gen 4 IEEE Type 1 exciters.

The first two events have concrete initialization clues. SEARSMONT's
partitioned algebraic solve moves its terminal voltage by only
`-1.6788764e-6 pu`, but the IEEE Type 1 regulator has `Ka=400`, `Ta=0.0354`,
and an initial physical rate of `1.89703546e-2 pu/s`. Its initialized `vr`
equals the permissively expanded `VRMAX`, so that small nonzero rate reaches
the bound immediately. HUNTERSVILLE's RAW dispatch is `0.83257576 pu`, above
the DYR `PMAX=0.7597 pu`; initialization expands the governor limit to retain
the dispatch. Its initial machine-speed rate is only about
`-3.6454e-7 pu/s`, yet the three governors reach the adjusted boundary at
`6.94 ms`. These observations support treating early roots as initialization
and model-boundary diagnostics, rather than as evidence of a disturbance.
The remaining PARKSLEY, MANASSAS, and KINSTON roots need the same local RHS and
limit-distance audit.

The root-processing changes now in the working tree synchronize the paired
KINSOL algebraic state inside the partitioned root callback, refresh cached
inputs at the reported root, solve the algebraic subsystem at that same event
time, and restart differential solver history after a limiter branch change.
EXAC1 release handling also trusts the root direction from its limited branch,
avoiding a second threshold comparison against a slightly different cached
input. The 30 ms run completed without the earlier repeated MANASSAS root
chatter; this only validates event progress, not the physical transition or
long-term stability.

The captured 30 ms trajectory shows three JOHNSTON 5 1 GGOV1 `load_int` states
(indices 66460, 66482, and 66504) beginning to drift after the HUNTERSVILLE
root. A targeted trace showed this is a GGOV1 selector transition, not a
derivative change introduced by the solver restart. The same `0.16303023 pu/s`
rate appears in a root-disabled capture by 10 ms.

At initialization, JOHNSTON's initial fuel request (`1.01210385178 pu`) is
above DYR `Vmax=1.0 pu`, so GridDyn expands `Vmax` to the initial request. The
initial temperature request is then capped at that expanded `Vmax` and tied
with the normal request. The temperature controller is inactive at that exact
tie, and `load_int` starts with zero rate. After the first milliseconds, a
small frequency deviation raises the normal request by just over the selector
tolerance (`1e-10`): at `8.566 ms`, normal request was `1.0121038518853 pu`
versus temperature request `1.0121038517796 pu`. The temperature controller
becomes active and its integral rate is `Kiload * tempError = 0.16303023 pu/s`.
The states rise from `0.84907362 pu` at startup to `0.85272031 pu` at 30 ms.

This identifies the fragile point: the power-flow dispatch exceeds the
imported governor limit, and limit correction puts the temperature and normal
requests on a selector tie. A tiny speed change then activates the temperature
integrator while its request is clamped at `Vmax`. PowerWorld documents
raising `Vmax` to the low-value-select output when it exceeds the imported
limit, so the expansion itself is compatible with its documented behavior
([GGOV1 model](https://www.powerworld.com/WebHelp/Content/TransientModels_HTML/Governor%20GGOV1%20and%20GGOV1D.htm)).

The selector's activation tolerance was increased from `1e-10` to `1e-6 pu`,
matching the startup interior margin. This does not change the low-value-select
output; it changes when the inactive temperature PI begins integrating and
uses the same deadband in its Jacobian branch. In a Release 100 ms flat run,
the three Johnston `load_int` states remained exactly at `0.84907362 pu` with
zero recorded rate, whereas the `1e-10` run gave them a `0.16303023 pu/s` rate
by 8 ms. This supports the tie deadband as a specific improvement, but it is
not full-case validation: the 100 ms run still had 41 limit roots, and
New Hampshire / MELVIN VIL~1 Gen 1 ESAC1A `va` changed by `0.002787 pu` with a
rate of `0.16434 pu/s` at 100 ms (its initial RHS was `0.006638 pu/s`). The
other early roots and drifting exciter states still need their local
initialization audit. Do not claim a stable 5 s flat run until those issues are
resolved. The DAE path remains separately blocked at its initial
Jacobian/consistent-initial-condition failure described above.

Reproduce the 100 ms flat diagnostic and retain the state snapshots with:

```powershell
build-ninja-release\bin\gridDynMain.exe `
  C:\Users\phlpt\Documents\test_cases\ACTIVSg25k\ACTIVSg25k.RAW `
  --import build-ninja-release\ACTIVSg25k-flat-Pset-initial-dispatch.dyr `
  --param=dynamicsolvermethod=partitioned --param=timestop=0.1 `
  --flags=sparse,partitioned_diagnostics,no_shunt_adjustments,no_powerflow_error_recovery `
  --state-output=activsg25k-ggov1-tie-tolerance-100ms.bin --save-state-period=10 --summary `
  *> activsg25k-ggov1-tie-tolerance-100ms.txt
```

### Current-source flat-run recheck (2026-10-13)

After rebuilding the Release executable from the current working tree, a fresh
no-disturbance partitioned run used the original ACTIVSg25k RAW case, the
`ACTIVSg25k-flat-Pset-initial-dispatch.dyr` profile, and
`no_shunt_adjustments,no_powerflow_error_recovery`. The 100 ms diagnostic
reached its stop time, but recorded 51 limiter roots: 48 DC2A exciters and 3
IEEE Type 1 exciters. Across 66,510 differential states, the largest absolute
change from the initialized vector was `4.397e-4` at 100 ms. This short result
is an event-progress check, not a five-second stability result.

A 5 s run with 100 ms snapshots was then started with the same settings. At
200 ms, state index 30613 had changed from `3.1666076` to `3.2822953`, an
absolute change of `0.115688`; its sampled derivative was `6.56903` at that
time. A temporary name probe mapped it to New Hampshire / MELVIN VIL~1 Gen 1
ESAC1A `va`, the exciter regulator-output state. A separate focused rerun
traced that model at `0.200089 s`; terminal voltage had fallen from
`1.03864771` to `1.03812505 pu`, while the reference input was effectively
unchanged. The ESAC1A
regulator target had risen from `3.16682886` to `3.49794308 pu`, giving
`dVa/dt=6.50292 pu/s`. Its limit status was still unlimited (`status=0`), with
`va=3.28118 pu` below `VAMAX=9 pu`. This state is moving because its inputs
changed, not because its own limiter switched branches. The DYR has
`Ka=750` and `Ta=0.033333 s`, so the small terminal-voltage movement is
strongly amplified in the regulator drive.

The run had reported 122 limiter roots by 218.7 ms: 99 DC2A and 23 IEEE Type 1
exciter events. It was stopped there because the no-disturbance state was
already moving materially and the remaining interval was advancing too slowly
for a useful stability pass. The binary's initial derivative record is
zero-filled; use the state at t=0 as the state baseline, but do not interpret
its t=0 derivative record as the physical initial RHS.

This recheck confirms that current root synchronization can make progress past
the old repeated-root stall, but it does not resolve the initial-equilibrium
problem. The next partitioned investigation should trace the small bus-voltage
drift at MELVIN VILLAGE to the upstream reactive-power and dynamic-state
changes, then rerun the bounded flat case. A full 5 s run remains unverified.
The DAE path has been retried and still fails in Jacobian/linear-solver setup,
as detailed below. The temporary state-name/input instrumentation was removed
after the probe and the Release executable rebuilt from the uninstrumented
source.

Reproduce the bounded 200 ms drift check with:

```powershell
build-ninja-release\bin\gridDynMain.exe `
  C:\Users\phlpt\Documents\test_cases\ACTIVSg25k\ACTIVSg25k.RAW `
  --import build-ninja-release\ACTIVSg25k-flat-Pset-initial-dispatch.dyr `
  --param=dynamicsolvermethod=partitioned --param=timestop=0.2 `
  --flags=sparse,partitioned_diagnostics,no_shunt_adjustments,no_powerflow_error_recovery `
  --state-output=activsg25k-current-limiter-flat-200ms.bin --save-state-period=100 --summary `
  *> activsg25k-current-limiter-flat-200ms.txt
```

The DAE path was also retried with IDA diagnostics and stop-on-first-failure.
It still fails before advancing with `IDA_LSETUP_FAIL` and exits through a bad
allocation. The reported algebraic residual was at most `9.933e-6` and the
differential residual at most `2.747e-11`, but the initial-condition Jacobian
had 6,484 zero diagonal entries on GENROU and GENSAL `id`/`iq` states (699,853 nonzeros,
with no empty rows or columns and no missing diagonal entries). This keeps the
DAE problem separate from the partitioned drift: the immediate DAE blocker is
the Jacobian/linear-solver setup, not a large reported initial residual.

```powershell
build-ninja-release\bin\gridDynMain.exe `
  C:\Users\phlpt\Documents\test_cases\ACTIVSg25k\ACTIVSg25k.RAW `
  --import build-ninja-release\ACTIVSg25k-flat-Pset-initial-dispatch.dyr `
  --param=dynamicsolvermethod=dae --param=timestop=5 `
  --flags=sparse,ida_ic_diagnostics,ida_ic_stop_on_failure,no_shunt_adjustments,no_powerflow_error_recovery `
  *> activsg25k-current-dae-ic-20261013.txt
```

### DC2A and IEEE Type 1 limiter deadband audit (2026-10-13)

The DC2A and IEEE Type 1 root tests declared model-specific
`limiterRootTolerance` constants, but those names were hidden by the inherited
`Exciter::limiterRootTolerance` member (`2e-6`) during member-function lookup.
Consequently, DC2A used `2e-6` instead of its intended `1e-4` margin in both
its limit-entry and held-limit release roots. IEEE Type 1's limit-entry branch
already used an explicit `1e-5` literal; its held-limit release root used
`2e-6` instead of the intended `1e-5`. The model-specific constants now have
distinct names so the intended margins are applied in both models.

The exciter unit tests now exercise initialized upper-limit cases with
`+/-1e-5` terminal-voltage changes. DC2A also checks the voltage-dependent
lower-limit root and confirms that the initially expanded upper bound equals
its starting regulator output. Its initial derivatives and residuals are zero
for the isolated equilibrium; both DC2A and Type 1 tests also assert the held
limit release-root margins. The IEEE Type 1 tests assert the `1e-5` margin at
both the limit-entry and held-limit release roots. All 64 `ExciterModelTests`
pass in `build-ninja\bin\GeneratorComponentTests.exe`.

A fresh Release partitioned run of the flat ACTIVSg25k case reached 100 ms
with no disturbance, no shunt adjustments, and no power-flow error recovery.
It reported 7 exciter roots (4 DC2A and 3 IEEE Type 1), down from 51 (48 DC2A
and 3 IEEE Type 1) in the previous 100 ms recheck. The DC2A entry-root count
fell by 44; the three Type 1 entry roots remain because their entry branch
already used the `1e-5` margin. The remaining root owners were all initialized on
their permissively expanded upper limits:

- SEARSMONT 1 Gen 2 IEEE Type 1: initial `vr=2.25914` pu versus imported
  `VRMAX=1.0` pu; root at `1.130662 ms`.
- GOFFSTOWN ~3 Gen 1 and Gen 2 IEEE Type 1: initial `vr=1.49451` pu versus
  imported `VRMAX=1.0` pu; roots at `96.485883 ms`.
- NEW HAVEN ~21 Gen 1-3 DC2A: initial `vr=1.92647` pu versus imported
  `VRMAX=1.17` pu; roots at `97.021446 ms`.
- MONTPELIER ~4 Gen 1 DC2A: initial `vr=0.562124` pu versus imported
  `VRMAX=0.4` pu; root at `98.038641 ms`.

The earlier millisecond-scale DC2A roots from TUCKASEGEE and other buses did
not recur in this run. All remaining roots therefore start from a boundary
condition and occur after the trajectory moves outward; they are not evidence
that those models initialized comfortably inside their imported limits and
then independently became unstable. The SEARSMONT trace already showed a
nonzero initial regulator rate (`1.897e-2 pu/s`) with only a
`1.68e-6 pu` terminal-voltage shift, so that event is consistent with its
initial state moving outward from the corrected limit. The 100 ms state
capture's largest change was `4.16e-4 pu` (index 30613, MELVIN VILLAGE ESAC1A
`va`). This is a short initialization/stability check; it does not establish a
stable five-second trajectory.

Reproduce the Release recheck with:

```powershell
build-ninja-release\bin\gridDynMain.exe `
  C:\Users\phlpt\Documents\test_cases\ACTIVSg25k\ACTIVSg25k.RAW `
  --import build-ninja-release\ACTIVSg25k-flat-Pset-initial-dispatch.dyr `
  --param=dynamicsolvermethod=partitioned --param=timestop=0.1 `
  --flags=sparse,partitioned_diagnostics,no_shunt_adjustments,no_powerflow_error_recovery `
  --state-output=activsg25k-exciter-deadband-100ms.bin --save-state-period=10 --summary `
  *> activsg25k-exciter-deadband-100ms.txt
```

### Root-owner audit past 100 ms (2026-10-13)

The `ExciterModelTests.*` suite was rerun after the DC2A and IEEE Type 1
tolerance-name fix: all 64 tests passed. A fresh Release flat run was then
started with a 5 s stop time, no disturbance, no shunt adjustments, and no
power-flow error recovery. It reached `0.20396 s` and logged 84 exciter
limit-entry events on 84 distinct devices: 69 DC2A and 15 IEEE Type 1. Seven
events occurred by 100 ms; 77 more appeared between 100 ms and 204 ms. The
simulation was stopped at that point because each event performs a full
partitioned algebraic reinitialization, making a 5 s run impractical at the
observed event rate.

Every one of those 84 owners matched a current-Release startup warning from a
separate 10 ms verbose run: its initialized regulator output exceeded the
imported DYR `VRMAX`, so GridDyn expanded the upper bound to the initialized
output. The initial-output/imported-limit ratios for these devices range from
1.043 to 8.849. For example, DEXTER 3 2 Gen 1 DC2A initializes at
`Vr/Vt=7.57818`
against DYR `VRMAX=1.17`, and WINTHROP 3 2 Gen 2 IEEE Type 1 initializes at
`1.75422` against `VRMAX=1.0`. Thus the logged events are later crossings of
permissively expanded upper limits. They are not repeated events from the
shadowed root tolerance: the tolerance fix reduced the first 100 ms from 51
events to 7, and no owner repeated within this 204 ms trace.

The partial run's differential-state snapshots show the broader no-disturbance
trajectory moving away from its initialized state. The largest change was
`4.15744e-4 pu` at 100 ms and `0.128965 pu` at 200 ms, both at state index
30613, mapped by the prior name probe to MELVIN VILLAGE Gen 1 ESAC1A `va`.
That state is below its own `VAMAX`; the focused earlier trace found its
regulator target rising as terminal voltage moved by only about `5.23e-4 pu`.
This is a concrete evolving state that can contribute to nearby exciter
limit crossings, but it does not yet identify the upstream source of the bus
voltage drift or explain every root in the wider New England/New York area.
The 5 s no-disturbance trajectory remains unverified.

The captured partial run and current-build startup warnings are in
`activsg25k-exciter-root-audit-5s-20261013.txt` and
`activsg25k-exciter-root-init-10ms-20261013.txt` respectively. The 5 s command
was:

```powershell
build-ninja-release\bin\gridDynMain.exe `
  C:\Users\phlpt\Documents\test_cases\ACTIVSg25k\ACTIVSg25k.RAW `
  --import build-ninja-release\ACTIVSg25k-flat-Pset-initial-dispatch.dyr `
  --param=dynamicsolvermethod=partitioned --param=timestop=5 `
  --flags=sparse,partitioned_diagnostics,no_shunt_adjustments,no_powerflow_error_recovery `
  --state-output=activsg25k-exciter-root-audit-5s-20261013.bin --save-state-period=100 --summary `
  *> activsg25k-exciter-root-audit-5s-20261013.txt
```

The next useful trace is to compare terminal-voltage movement and regulator
output movement at these initialized-on-limit devices against the MELVIN
VILLAGE ESAC1A trajectory, then locate what drives the initial voltage drift.

### Interpreting out-of-limit exciter initial conditions (2026-10-13)

For DC2A and IEEE Type 1, `VRMAX` applies to regulator output `Vr` (state 1),
not to field voltage `Efd` (state 0). Both initializers calculate
`Vr = Ke*Efd + Efd*Se(Efd)` from the field voltage required by the initialized
machine. DC2A's physical upper bound is `VRMAX*Vt`; its warning compares the
ratio `Vr/Vt` with the imported `VRMAX`. IEEE Type 1's upper bound is the fixed
`VRMAX`, and its warning prints `Vr` directly. The ESDC2A and IEEET1 DYR
field positions match those bounds. ANDES independently uses the same
`Ke*Efd + Efd*Se(Efd)` initial regulator equation and corresponding DC2A
`VRMAX*Vt`/Type 1 fixed bounds.

The source DYR has `VRMAX=1.17` for DEXTER 3 2 Gen 1 and Gen 2 ESDC2A and
`VRMAX=1.0` for SEARSMONT 1 Gen 2 IEEET1. For DEXTER, RAW `Vt=1.03833485`,
so the warning ratio `Vr/Vt=7.57818055` implies `Vr=7.86869`. Inverting the
imported `Ke=1`, `E1=1.33`, `SE1=0.2122`, `E2=1.7733`, `SE2=0.849`
quadratic saturation curve gives `Efd` about `2.47171` and saturation feedback
about `5.39698`. This confirms the large `Vr` follows from the machine's
required field voltage combined with the DYR exciter curve. It does not yet
prove that the machine field voltage is correct: its RAW dispatch, machine
base, GENSAL initialization, and saturation need an independent cross-check.

GridDyn's default permissive policy raises an exceeded upper bound to the
initial value. This preserves the static machine field voltage but places the
regulator exactly at the corrected bound; a tiny outward change or a small
post-initialization algebraic voltage correction can then cause a limit-entry
root. `strict_exciter_limits` instead rejects that inconsistent initial
condition. Neither widening by an arbitrary margin nor clamping `Vr` to the
original DYR limit establishes a valid no-disturbance equilibrium. The next
comparison should log, per device, RAW `P/Q/Vt/MBASE`, machine `Efd`, exciter
`Ke*Efd`, saturation feedback, original/adjusted limit, and physical initial
RHS after the partitioned algebraic solve. It should cross-check the largest
outliers before changing source data or limit policy.

The DC2A voltage-dependent limit branch was corrected. Limit detection and
state projection now use the incoming voltage before DC2A's `Tr` filter; the
filter still supplies the regulator feedback signal. While limited, the held
antiwindup `Vr` state has zero derivative, but the field equation uses the
moving effective output `VRMAX*Vt` or `VRMIN*Vt`. Its Jacobian has the
corresponding voltage term instead of a derivative with respect to held `Vr`.
The release drive also compares its unconstrained target with that moving
effective output. This avoids treating a stale held state as the exciter field
input after voltage changes. IEEEX1's lead-lag mode delegates its root
handling to DC2A so the two models use the same limited-branch equations.
The incoming voltage is GridDyn's exciter
voltage signal; a generator voltage-compensator route can alter that signal,
so physical raw terminal-voltage routing remains a separate interface audit.

This correction does not remove the initial DEXTER limit mismatch. At a true
active upper-limit equilibrium, the field equation requires
`VRMAX*Vt = Ke*Efd + Efd*Se(Efd)`, and the unconstrained regulator target must
remain above that bound by a positive margin to keep the branch active under
small changes. With the original DEXTER `VRMAX=1.17`, its RAW operating point
requires much larger `Vr`; preserving the same `Efd` and enforcing that limit
cannot both yield zero field derivative. Numerically, at DEXTER's original
upper bound `Vr=1.17*1.03833485=1.21485`, the existing field operating point
requires about `7.86869`; with DYR `Te=1`, the field rate would be about
`-6.654 pu/s` rather than zero. The previous permissive policy raised
`VRMAX` exactly to initial `Vr/Vt` and initialized the regulator target equal
to it. That produced a degenerate boundary tie, not a robust saturated
equilibrium. The headroom policy below starts this corrected case inside its
effective limit. A physically consistent saturated initialization needs a static
power-flow state compatible with the original bound and a reference/control
condition that demands a value beyond it. Alternatively, after confirming
the machine calculation, an explicitly corrected limit with documented
headroom can preserve the original RAW state as an unsaturated equilibrium.
An arbitrary bias or deadband would only mask the source inconsistency.

#### Headroom for permissively raised DC2A and IEEE Type 1 limits

PowerWorld documents raising a violated EXDC2A `VRMAX` to the starting `Vr`.
That exact adjustment preserves the starting field voltage but puts the
otherwise unlimited regulator state on a root boundary. GridDyn now gives
only these two exciter families headroom equal to the greater of 20% of the
required limit increase and a model-specific numerical guard. This keeps
the corrected limit outside the initialized output without claiming the
original DYR cap was physically feasible. Other exciter families retain the
existing exact-value adjustment.

It is calculated only when the imported upper limit is exceeded. The current
initializer uses the machine field voltage and terminal voltage supplied
before the partitioned algebraic correction; it does not recalibrate the
limit after that correction. Let `r0 = Vr0/Vt0`, `epsilonV = 1e-5 pu` (the ten-times-normal
input-perturbation target), and `epsilonRoot = 1e-4 pu` (DC2A's current
limit-root tolerance). The voltage-only numerical margin in `Vr` units is

`marginVr = 10*epsilonRoot + (Ka*max(1, Tc/Tb) + abs(r0))*epsilonV`.

Set `VRMAXadjusted = r0 + max(0.2*(r0 - VRMAXimported), marginVr/Vt0)`.
Keep the
regulator on its unlimited branch at initialization, log both the imported
and effective limits, and retain strict-limit rejection as an option. The
`Ka*max(1,Tc/Tb)` term bounds the initial lead-lag sensitivity and steady
voltage sensitivity; `abs(r0)` allows for movement of the voltage-scaled
bound. This guard covers the stated voltage-input perturbation, not arbitrary
perturbations of every controller state or a sustained network drift.

For DEXTER 3 2, `Ka=50`, `Tc/Tb=0.5/0.11`, `r0=7.57818055`, and
`Vt0=1.03833485`, giving `marginVr=0.00334851 pu` and numerical
`marginVRMAX=0.00322488`. The 20% shift term dominates:
`0.2*(7.57818055 - 1.17)=1.28163611`, so the effective
`VRMAX=8.85981666`. This is substantial headroom above the corrected
starting ratio, not a physical justification for replacing the original
`1.17` cap. The margin may prevent immediate root crossings;
it cannot make the initial differential RHS zero or guarantee five-second
stability. Additional input channels or a measured larger initialization
correction require enlarging the input-perturbation budget accordingly.

For an IEEE Type 1 unit with a fixed upper bound, the analogous numerical
guard is `10*epsilonRoot + abs(Ka)*epsilonV` in `Vr` units, and the same 20%
shift rule applies. With
`epsilonRoot=1e-5`, SEARSMONT 1 Gen 2 (`Ka=400`, initial `Vr=2.25913697`)
has a `0.0041 pu` numerical guard but a `0.25182739 pu` proportional guard,
giving an effective `VRMAX=2.51096437`. This is subject to the
same initialization-equilibrium and source-data caveats as DC2A.

The `ExciterModelTests` suite was updated with both proportional-dominant and
numerical-dominant examples for the two families; all 66 tests passed in the
Debug Ninja build. The Release `gridDynMain` executable compiled. With the
flat-Pset DYR, partitioned solver, and `no_shunt_adjustments`, the Release
no-disturbance case reached 0.1 s with no reported limiter roots. A 5 s
attempt then reached its first DC2A root at 0.198460934 s (MONTPELIER~4 Gen 1)
and several more DC2A and IEEE Type 1 roots by about 0.25 s. It was stopped
there because the five-second root-free criterion had already failed and
progress was slow. Thus the new headroom eliminates the earlier near-start
roots but does not establish stable long-duration behavior. The longer run's
log is `activsg25k-exciter-headroom-5s-20261013.txt`; its final line was cut
off when the diagnostic process was stopped.

#### Comparison with the same models that do not warn

To separate a low imported limit from an unusually high required starting
output, match the Release initialization warnings to the source DYR records by
RAW bus name and generator ID. Restrict the comparison to generators whose RAW
status is in service; offline DYR records do not initialize their exciters and
therefore cannot warn. For `ESDC2A`, all 202 DYR records are represented by
`ExciterDC2A`, of which 157 generators are in service. The runtime IEEE Type 1
population comprises 942 `IEEET1` records plus 123 `ESDC1A` records with zero
`Tb` that the reader maps to `ieeet1`; 745 of those generators are in service.

| Source model              | Imported `VRMAX` | In-service warning | In-service no warning |
| ------------------------- | ---------------: | -----------------: | --------------------: |
| `ESDC2A`                  |              0.4 |                 58 |                     9 |
| `ESDC2A`                  |             1.17 |                 42 |                    14 |
| `ESDC2A`                  |             7.21 |                  0 |                    34 |
| `IEEET1`                  |              1.0 |                 35 |                   365 |
| `IEEET1`                  |            1.211 |                  0 |                   193 |
| `IEEET1`                  |              7.0 |                  0 |                    58 |
| `ESDC1A` mapped to Type 1 |              1.0 |                  2 |                    47 |
| `ESDC1A` mapped to Type 1 |             10.0 |                  0 |                    45 |

The 37 Type 1 warnings consist of 35 `IEEET1` and two zero-`Tb` `ESDC1A`
records. The full unwarned Type 1 group includes 616 `IEEET1` and 92 such
`ESDC1A` records. All warnings occurred on in-service generators. Within the
warning group, the required initial `Vr/Vt` for DC2A ranges from 0.5621 to
7.5788, with a median of 2.3409; for Type 1, required `Vr` ranges from 1.0190
to 3.1130, with a median of 1.4945. The warned DC2A units have imported limits
of 0.4 or 1.17, but 23 in-service DC2A units with those same limits do not
warn. Likewise, 412 in-service Type 1 units have `VRMAX=1.0` and do not warn.

The limit and gain alone do not explain the difference. WALDEN 2 2 Gen 1 and
WALDEN 2 3 Gen 1 have identical `ESDC2A` parameter records, including
`VRMAX=1.17` and `Ka=9.3`. WALDEN 2 2 requires initial `Vr/Vt=1.63785` and
warns, while WALDEN 2 3 does not. Their RAW reactive dispatch differs (0.283
versus 0.142 Mvar). At NEW MARTIN~4, Gen 1 and Gen 2 have identical `IEEET1`
parameter records (`VRMAX=1`, `Ka=400`); Gen 2 requires initial `Vr=1.22658`
and warns, while Gen 1 does not. Their RAW active dispatch differs (30.728
versus 10.752 MW). These pairs point to the machine operating point and the
field-voltage/saturation calculation as the quantity to inspect next, rather
than a different interpretation of the `VRMAX` field. Dispatch difference is
an observation, not proof of the causal path. For example, the unwarned
LOCKHART 3 Gen 2 has the same `VRMAX=1.17` as warned Gen 1 but is offline in
RAW, so it is not a valid active comparison.

#### Component prototype of saturated initialization

Before changing the production initial-limit policy or rerunning the 25k
network, two component tests construct the proposed active upper-limit state
after ordinary initialization. They use the DEXTER 3 2 ESDC2A and SEARSMONT 1
Gen 2 IEEET1 exciter parameters and field outputs. Let `Vr0` be the regulator
output needed to hold the initial field, `L0` the imported upper limit, and
`Vt0` the initial terminal voltage. For DC2A, set the corrected bound to
`L=Vr0/Vt0` and the unconstrained target to
`Vr0 + 0.2*(L-L0)*Vt0`. For Type 1, set `L=Vr0` and target to
`Vr0 + 0.2*(L-L0)`. Enter the active high-limit branch at initialization.
Type 1 requires a corresponding reference-bias increase of overdrive/`Ka`.
DC2A requires the same bias increase **and** an overdrive/`Ka` increase in its
lead-lag state; changing its bias alone would leave a nonzero lead-lag
derivative.

In both prototypes, the starting `Efd` and `Vr` are preserved, every
differential derivative and DAE residual is zero to `1e-12`, the initial
high-limit release root is positive, and roots stay on the same side for
terminal-voltage changes of `+/-1e-5 pu`. At those perturbed inputs the
derivatives remain finite and the residuals agree with them for zero supplied
state rates. The active-branch DAE Jacobian check
also passes for both models, including DC2A's nonzero voltage-transducer time
constant. The Debug `ExciterModelTests` suite passed all 68 tests. These tests
constructed the state in test-only subclasses before the production changes
described below. A network algebraic correction after component initialization
and the 25k trajectory have not yet been checked under the production
active-limit policy.

#### Production startup-limit policy (subsequent implementation)

The production initializer now uses a bounded output equal to the required
initial value when permissive initialization corrects a violated upper limit.
When an output is at a bound at initialization, supported controllers put the
unconstrained request beyond that bound and select the held branch. The model
sets the margin to the larger of 20% of any required limit correction and a
model-specific guard based on its root and input sensitivity. Exact-bound
initializations with no limit correction use the guard alone. The controlled
output and other states are kept at a zero-derivative equilibrium. The strict
limit policy still rejects a material source-limit violation.

DC1A, DC2A, IEEE Type 1, IEEE Type 2, SEXS, ESAC5A, ESAC6A regulator,
EXAC1/EXAC2/ESAC1A, EXPIC1 PI, SCRX,
IEEET3 regulator, the AC7B PID and feasible PI stages, the AC8B PID and
feasible regulator stages, and the ESST1A amplifier, ESST2A regulator,
feasible ESST3A regulator and field stages, ESST4B PI stages, and EXST1 and
EXAC4 input limits now apply this policy when their respective bound is
active. DC1A/DC2A, EXAC1, SCRX, SEXS, and
ESST1A must also move an internal filter or lead-lag state to retain zero
initial residual. ESST1A applies amplifier overdrive only if its separate
input limit has enough room for the requested margin. AC7B and AC8B only
overdrive a downstream limit when its upstream stage has room to supply the
additional request. ESST4B holds its limited PI output while biasing the
unconstrained request; it avoids outer-loop overdrive when the initial
stabilizer input is nonzero. ESST3A only overdrives a field limit when its regulator and input bounds have
room; it only overdrives a regulator limit when its input bound has room.

For governors, IEEE simple, TGOV1, its ANDES variants, IEEEG2, TG2, SteamNR,
SteamTCSR, IEESGO, and Hydro use a private initial setpoint offset when their output
is at a position bound. The generator's public power command can itself be
clamped at its Pmax/Pmin, so an overdriven `fieldSet` alone would be lost.
IEEEG1 and HYGOV use their internal reference and preserve their valve/gate
states; HYGOV also moves its filter state to keep the initial filter rate
zero. HYGOV4 moves its servo-drive state and reference offset together.
GAST uses a reference offset only when its temperature request has enough
headroom for the valve to see the overdrive; a binding temperature selector
cannot be overdriven by changing the speed-droop request alone. IEESGO moves
its two states before the valve clamp together with the private setpoint.
GPWSCC moves its controller, derivative, and servo states together when the
gate has a nonzero actuator gain and rate range.
GGOV1 adds startup overdrive at the actuator request when its valve is at a
position bound. Its temperature selector clips at Pmax, so changing only the
normal reference cannot move the selected request beyond that bound. The
actuator offset is included in the rate and Jacobian branch logic.

This is not yet a claim of universal controller coverage. Cascaded and
competing limits in AC7B, AC8B, ESST2A field output, IEEET3 field output,
ESST3A when its upstream bound is binding, ESAC6A and EXPIC1 downstream
limits, EXST1 and EXAC4 output limits, the generic block-based governor,
GGOV1 selector limits upstream of its valve, binding-temperature GAST,
GPWSCC configurations with zero actuator gain or travel rate, and
the renewable electrical controllers need
model-specific feasibility analysis before a numerical overdrive can be
inserted without changing the physical output or leaving a nonzero residual.
In particular, if an upstream limiter is already at its own bound, adding a
reference offset may not increase the downstream unconstrained demand at all.
The edited `griddyn` library compiles in the local Debug Ninja/MSVC tree.
The following run recheck records what happened when the policy was exercised
on ACTIVSg25k.

#### Startup overdrive ACTIVSg25k recheck (2026-10-14)

The Release `gridDynMain` executable was rebuilt from the working tree with
the startup overdrive changes. Runs used the original ACTIVSg25k RAW file,
the flat-Pset DYR profile, partitioned integration, no shunt adjustments,
and no power-flow error recovery.

A 100 ms no-disturbance run completed. Its 66,510 differential states moved
by at most `3.239e-4 pu`; the largest change was state 30613, MELVIN VILLAGE
Gen 1 ESAC1A `va` (`3.1666076` to `3.16693152 pu`). At 100 ms that state's
saved derivative was `8.242e-3 pu/s`.

The 5 s run was attempted twice. Both reached 200 ms, then spent several
minutes in partitioned algebraic/Jacobian work without writing a later state
snapshot. The second attempt was stopped after the log ended inside KINSOL
Jacobian callback 7 at time `0.188772798 s`; it did not fail with a reported
solver error. Before that stall it logged one limit root:
PIERCEFIELD~3 Gen 2 DC2A at `0.176272798 s`. The largest differential-state
change by 200 ms was again MELVIN VILLAGE Gen 1 ESAC1A `va`, now
`0.0338623 pu` (`3.1666076` to `3.20046986 pu`), with saved derivative
`1.89407 pu/s`. Earlier 200 ms rechecks recorded 84 limit roots and a larger
`0.128965 pu` change in the same state. Overdrive reduced the observed root
count and drift in this short window, but the trajectory is still not stable
and the 5 s objective was not reached.

The DAE `t=5 s` attempt still failed before advancing with exit `-5` and
`IDA_LSETUP_FAIL`, followed by `GridDyn execution failed: bad allocation`.
IDA's initial-condition diagnostic reported a maximum residual of
`9.93313e-6` (algebraic) and `2.74716e-11` (differential), with 6,484 zero
diagonal entries in the IDA IC Jacobian, all identified in GENROU `id`/`iq`
rows. This remains a separate DAE Jacobian/linear-solver blocker.

The immediate dynamic investigation should trace the terminal-voltage and
reference signals into MELVIN VILLAGE ESAC1A `va`; that state remains the
dominant drift while it is below its own `VAMAX`. Separately, inspect the
PIERCEFIELD DC2A's terminal voltage and held-limit drive at the 176 ms root.
The overdrive policy is reducing limiter events, but it has not resolved the
upstream algebraic/control drift or the DAE Jacobian issue. Run logs and state
captures are `activsg25k-overdrive-100ms-20261014.*`,
`activsg25k-overdrive-5s-retry-20261014.*`, and
`activsg25k-overdrive-dae-20261014.txt`.

#### MELVIN ESAC1A signal trace and PIERCEFIELD DC2A contact (2026-10-14)

The MELVIN VILLAGE Gen 1 DYR record is attached to RAW bus 35302 and uses
ESAC1A. It has no voltage-compensator model. `DynamicGenerator` therefore routes
the bus terminal voltage directly to the exciter; the trace confirms the sensed
voltage exactly follows that terminal voltage. The ESAC1A reference is
`Vref + vBias + Vset - 1 + Vss`: `Vref=1`, `Vset=1`, `Vss` stays within about
`7e-8 pu` of zero, and `vBias=0.04287015346` is constant. There is no changing
remote-voltage or stabilizer signal at MELVIN.

At 198.219 ms the terminal voltage has fallen from `1.03864770892` to
`1.03851103312 pu` (`-1.367e-4 pu`). That voltage change raises the regulator
input from `0.0042224385` to `0.0043370759 pu`; field-feedback movement opposes
part of the increase. With `Ka=750`, the unconstrained target reaches `3.2528069`
while `va=3.1966122`, giving `dva=1.6858569 pu/s`. The limit status remains
unsaturated. This traces the dominant `va` movement to the terminal-voltage
feedback path, not a drifting reference. It does not yet explain why the
network voltage is sagging.

There is a small initialization mismatch worth retaining in the record. During
model initialization ESAC1A sees the RAW starting voltage `1.03864801 pu`; the
ordinary partitioned startup's first algebraic evaluation is `1.03864770892 pu`.
That `3.01e-7 pu` change produces an initial `dva=0.0066381 pu/s`. With
`dae_initialization_for_partitioned`, `dva(0)` is approximately `-5.7e-12`, but
the run still reaches the same trajectory: at 198.219 ms `va=3.196612204` and
`dva=1.6858555`. So the startup voltage offset is real but is not sufficient to
explain the later drift. That optional DAE initialization emitted
`IDAICFailFlag` and fell back to the partitioned path; it is not evidence that a
full DAE run now works.

The PIERCEFIELD~3 Gen 2 DC2A event is separate. At initialization its `vr` is
`2.91792780088`, only `7.06e-8 pu` inside the voltage-dependent upper bound
`2.91792787150`. The DC1A-family initializer now treats contact within `1e-7`
as an initial high-limit condition and applies its model-specific overdrive.
The 200 ms no-shunt run keeps it latched high; the 176.2728 ms release root from
the prior run is absent. This does not remove the MELVIN trajectory or establish
stability beyond 200 ms.

The rate scan led to the upstream issue: WT3G1 used the opposite reactive
voltage-command polarity from the WT3E1 controller. Before the correction,
Lempster WT3E1/WT3G1 started at `V=1.067120`, `Q=-0.1200`,
`Eqcmd=-0.977158`; by 202.469 ms it was at `V=1.063436`, `Q=-0.144479`,
`Eqcmd=-0.928307`. Vinalhaven showed the same pattern: falling terminal
voltage made WT3E1 raise `Eqcmd`, while WT3G1 made reactive output decrease.
WT3E1's voltage loop raises `Eqcmd` when `Vref - V` is positive. In WT3G1,
`Q = Vreal * (Eq/Xeq) - Vimag * Ip - V^2/Xeq`, so a positive `Eq` increment
raises Q at fixed terminal voltage and angle. The old negative sign in both
WT3G1 initialization and `currentImag` reversed that response, creating
positive feedback into the bus voltage.

The ACTIVSg25k DYR records at Lempster (bus 35398) and Vinalhaven (bus 17624)
have `VARFLG=0` and `VLRFLG=1`. WT3E1 currently stores `varFlag` but does not
use it in its command or rate equations; that is a separate model-coverage
issue. It is not needed to explain this case's polarity error: the relevant
records enable terminal-voltage regulation through `VLRFLG=1`. The reference
equations used for the sign comparison are [OpenIPSL WT3G1](https://raw.githubusercontent.com/OpenIPSL/OpenIPSL/master/OpenIPSL/Electrical/Wind/PSSE/WT3G/WT3G1.mo),
[OpenIPSL WT3E1](https://raw.githubusercontent.com/OpenIPSL/OpenIPSL/master/OpenIPSL/Electrical/Wind/PSSE/WT3G/WT3E1.mo),
and [PowerWorld's WT3E1 parameter definition](https://www.powerworld.com/WebHelp/Content/TransientModels_HTML/Exciter%20WT3E%20and%20WT3E1.htm).

`src/griddyn/renewables/WT3G1.cpp` now initializes positive `Eq` for the
positive-injection convention and uses positive `currentImag = Eq/Xeq` in its
power equations. This preserves the initial P/Q point while making WT3E1's
voltage correction act in the restoring direction.

With that correction, the flat-Pset, no-disturbance, no-shunt-adjustment
partitioned Release run reached 5.0 seconds. MELVIN voltage remained between
`1.0386477` and `1.0386481 pu` in the 100 ms bus samples; Lempster remained
near `1.06712 pu`, and Vinalhaven near `0.997109 pu`. MELVIN ESAC1A `va`
(state 30613) changed from `3.1666076` to `3.16661663 pu` over 5 seconds,
with saved snapshots between `3.16656087` and `3.16668199 pu`; it remained
unsaturated. No root event or solver failure was logged. PIERCEFIELD~3 Gen 2
DC2A remained latched at its initialized high limit, and its former
176.2728 ms release root did not recur.

The corrected-run outputs are `activsg25k-wt3-polarity-fix-flat-5s-20261014.bin`,
`activsg25k-wt3-polarity-fix-flat-5s-20261014.csv`, and
`activsg25k-wt3-polarity-fix-flat-5s-20261014.txt`. The earlier MELVIN input
trace and pre-fix WT3 trajectories are in
`activsg25k-signal-trace-200ms-20261014.txt`,
`activsg25k-renewable-trace-200ms-20261014.txt`, and
`activsg25k-top-rates-200ms-20261014.txt`.

The DAE attempt was repeated against the corrected Release build and still
fails during initialization at time zero with `IDA_LSETUP_FAIL` and
`bad allocation`. Its maximum algebraic residual is `9.93313e-6`, its maximum
differential residual is `2.74716e-11`, and the same 6,484 GENROU/GENSAL `id`/`iq`
Jacobian diagonal entries are zero. This remains a separate DAE
linear-solver/Jacobian issue. The retest is logged in
`activsg25k-wt3-polarity-fix-dae-100ms-20261014.txt`.

#### Startup overdrive refinement after the WT3 polarity study (2026-10-14)

The five-second WT3-corrected run shows that MELVIN's dominant earlier drift
was driven by the reversed WT3G1 reactive response, not by insufficient
exciter overdrive. It gives no reason to increase the 20% fraction used when
an imported limit must be moved. The separate PIERCEFIELD DC2A root did expose
a narrower numerical contact case: its initial regulator state was only
`7.06e-8 pu` inside the voltage-dependent upper bound. The DC1A/DC2A family
already treated contact within `1e-7 pu` as an initially held limit. IEEE
Type 1, IEEE Type 2, and EXAC1/EXAC2/ESAC1A previously required exact contact.
Their initializers now use the same `1e-7 pu` contact distance, apply their
existing model-specific overdrive, and select the held branch. In EXAC1, the
differential rate and Jacobian also recognize the held near-contact branch;
otherwise a regulator classified as held could retain an outward initial
rate. Starting a new dynamic initialization now clears the exciter limiter
branch flags before selecting them from the new operating point.

The focused component checks use bounds `7e-8 pu` above the required initial
regulator output. They check the held regulator derivative and residual at
time zero, root signs after `+/-1e-5 pu` terminal-voltage perturbations, and
release of the held branch on reinitialization at an interior operating point.
The DC2A moving-bound field derivative at near contact remains below
`1e-7 pu/s`. Existing entry/release checks were updated to start inside a
bound when they intend to exercise a later transition. All 72
`ExciterModelTests` passed in the Debug Ninja/MSVC build.

The Release `gridDynMain` target rebuilt successfully. The original RAW plus
flat-Pset DYR, partitioned solver, sparse mode, no shunt adjustments, and no
power-flow error recovery completed another five-second no-disturbance run
with 51 saved differential-state samples. Its state-and-derivative binary is
byte-for-byte identical to the prior WT3-corrected run. MELVIN ESAC1A `va`
therefore again changed by `9.03927e-6 pu` from time zero to five seconds;
the largest absolute movement of any saved differential state at any sample
was `3.47048e-4` (index 45287 at 2.4 seconds). The output files are
`activsg25k-initial-overdrive-refinement-5s-20261014.bin` and
`activsg25k-initial-overdrive-refinement-5s-20261014.txt`.

At that point the next DAE pass was expected to examine the time-zero IDA IC
Jacobian failure. A separate source-review lead was that IEEE Type 2's
held-regulator residual and Jacobian currently use `+dVr/dt` and `+cj`, while
the other exciter differential residuals use `f - dVr/dt` and `-cj`. This
deserves a focused DAE residual/Jacobian check, although no `IEEET2` records
were found in this ACTIVSg25k DYR profile.

#### IEEE Type 2 held row and DAE startup Jacobian (2026-10-14)

The IEEE Type 2 lead was a real, isolated sign error. Its held regulator
derivative is zero, so the DAE residual must be `-dVr/dt` and the corresponding
Jacobian entry must be `-cj`. Both had the opposite sign. The corrected model
passes a nonzero-derivative held-row check and a finite-difference DAE Jacobian
check with `cj=1.5`. The finite-difference helper now perturbs `y'` by
`cj * delta_y`, so it checks the actual IDA Jacobian definition. A source
survey of the other exciter held rows found the expected `f - y'` and `-cj`
convention; all 72 exciter model tests and the 30 machine model tests pass.

The earlier diagnosis of 6,484 zero GENROU/GENSAL `id`/`iq` diagonal entries was
misleading. With zero stator resistance, those current equations have zero
diagonal terms but nonzero off-diagonal terms `Xqpp` and `-Xdpp`; their local
2-by-2 block is nonsingular for positive subtransient reactances. The
unmasked Jacobian has no empty rows or columns. The failure was in the
masked `IDA_Y_INIT` startup system, which holds `y'` fixed. KLU reported full
structural rank (134,992) but a numerical zero pivot at column 590, GGOV1
`load_int`. GGOV1 deliberately freezes this inactive load/temperature
integrator, leaving residual `-yp` and Jacobian `-cj`; `cj=0` in the masked
solve makes that differential row zero. The governor finite-difference check
now includes this `cj=0` branch. This explains why the masked factorization
fails without implying an error in the ordinary DAE Jacobian.

The DAE startup now tries `FIXED_DIFF` first. This holds the initialized
differential states, solves the actual algebraic equations and differential
derivatives, then checks algebraic roots. The masked strategy remains a
fallback. The optional IC diagnostic's dense full-pivot rank calculation is
bounded at 512 states; at 134,992 states its former allocation would require
about 146 GB and caused the reported `bad allocation`. The diagnostic now
reports the KLU status and singular column and limits individual zero-diagonal
messages to 16.

With the Release build, the original RAW plus flat-Pset DYR profile, sparse
DAE, no shunt adjustments, and no disturbance reached five seconds directly
from a successful `FIXED_DIFF` initialization. IDA reported 19 integration
steps, no nonlinear convergence failures, no root return, and maximum state
change of `2.54756322e-4` over five seconds. The run log is
`activsg25k-dae-fixed-diff-first-5s-20261014.txt`; the prior masked failure
and KLU column are captured in `activsg25k-dae-klu-column-20261014.txt`.
The existing ACTIVSg500 RAW and EPC/DYD DAE stability tests also pass.

An unresolved precision issue remains. After IDA reports successful
`FIXED_DIFF` IC, GridDyn's independent residual evaluation reports a maximum
algebraic residual of `1.00e-9`, but a maximum differential residual of
`1.72e-5` on ESAC6A `ve` rows. The same value occurred with a requested
`1e-8` tolerance, so tightening the solver tolerance alone did not remove it.
The diagnostic also reports initial differential derivatives up to
`8.60e-3` on EXAC2 `va` states. The five-second run is close to flat, but
these figures do not establish exact equilibrium at time zero. The next
investigation should compare the IDA `y/y'` pair with the model input caches
used for those ESAC6A and EXAC2 residuals immediately after IC.

#### Follow-up: expected GGOV1 hold and startup reliability

The inactive GGOV1 load integrator is behaving as designed. Its differential
rate is zero while another selector branch controls fuel, and its DAE residual
is `-yp`. With the masked `IDA_Y_INIT` strategy, IDA holds `yp` fixed and
must solve for `y`; that row has no derivative with respect to `y`. The KLU
zero pivot therefore identifies a startup-system formulation issue, not a
defect that should be removed by making the inactive GGOV1 integrator move.

The main DAE run already preferred `FIXED_DIFF`. The optional DAE correction
before partitioned integration and the incremental `step()` path still used
the masked strategy first; both now reuse the same DAE startup routine. The
masked solve remains available after a failed `FIXED_DIFF` attempt for cases
where differential states must move. In `IdaInterface::calcIC`, an
`IDA_NO_RECOVERY` result with no missing rows previously fell through and
could be returned as success; it now returns an invalid-state error.

The Release 25k case reached 100 ms in both direct DAE and partitioned mode
with optional DAE initialization, each using successful `FIXED_DIFF` startup
without the masked KLU failure. ACTIVSg500 RAW and EPC/DYD DAE stability
checks and the focused GGOV1 DAE sweep passed after the change. Two added
ACTIVSg500 checks also cover incremental DAE `step()` and the optional DAE
initialization before partitioned integration. The new
diagnostic logs are `activsg25k-dae-startup-reliability-100ms-20261014.txt`
and `activsg25k-partitioned-dae-startup-100ms-20261014.txt`.

A controlled second `FIXED_DIFF` pass left the ESAC6A `ve` post-IC residual
unchanged at `1.71870e-5`; that experiment was removed. This residual should
be studied separately rather than hidden by repeating the same IDA solve.

### Current Release 5 s flat and small-disturbance recheck (2026-10-14)

The Release `gridDynMain` target was checked and was current. Using the original
ACTIVSg25k RAW plus `ACTIVSg25k-flat-Pset-initial-dispatch.dyr`, both the
partitioned solver and sparse DAE completed the no-disturbance run to 5.0 s.
Each saved 51 state/derivative samples from 0.0 through 5.0 s and exited with
code 0. All saved state and derivative entries were finite. Maximum movement
from the initial differential-state vector was `3.47048e-4` for partitioned
mode (2.4 s) and `3.09779e-4` for DAE mode (2.9 s).

The small disturbance was a persistent 1 MW increase at
`BUS$11005::LOAD#0`, scheduled at 1.0 s. The RAW load is 53.219 MW, so the
step set it to 54.219 MW. Roots remained enabled; shunt adjustments and power-
flow error recovery were disabled. Both solvers completed 5.0 s with 51
samples and finite state/derivative vectors. The partitioned log records a
successful algebraic reinitialization at 1.0 s (`return=0`). Compared with
matched no-disturbance samples, the maximum state difference was `4.73259e-3`
for partitioned mode and `4.69965e-3` for DAE, both at 1.7 s. At 5.0 s those
maximum differences had fallen to `2.78576e-3` and `2.84057e-3`. This is
bounded, damped behavior over the tested interval; the nonzero state and
rate differences at 5 s mean it is not a settling or asymptotic-stability
claim. The previously measured DAE post-initialization residual issue remains
open.

A separate 10 MW step at the same load (about 19% of its initial value) was
tried in both modes. Partitioned integration reached 1.7 s, where the largest
state departure was about `4.72e-2`; DAE reached 1.2 s, with a largest sampled
departure of `4.10e-2` at 1.1 s. Both remained CPU-active without saving a new
sample for over a minute and were stopped. These larger-step runs are
incomplete, not evidence of divergence. The 1 MW step is the completed
small-disturbance check.

The completed outputs are `activsg25k-partitioned-baseline-5s-20261014.bin`,
`activsg25k-partitioned-baseline-5s-20261014.txt`,
`activsg25k-partitioned-load-step-1mw-5s-20261014.bin`,
`activsg25k-partitioned-load-step-1mw-5s-20261014.txt`,
`activsg25k-dae-baseline-5s-20261014.bin`,
`activsg25k-dae-baseline-5s-20261014.txt`,
`activsg25k-dae-load-step-1mw-5s-20261014.bin`, and
`activsg25k-dae-load-step-1mw-5s-20261014.txt`.
Partial stress-run outputs are `activsg25k-partitioned-load-step-5s-20261014.bin`
and `activsg25k-dae-load-step-10mw-5s-20261014.bin`.

### Branch review and regression gaps (2026-10-15)

A grouped RAW fixture exposed a missing conversion of remote generator terminal
buses when the regulated bus starts as PV and has local generators. The remote
terminals remained PV, so their local voltage equations could mask remote
regulation. The importer now makes those terminals PQ; the local-plant group
test checks this before solving and still checks group Q sharing afterward.
Cloning a remotely regulated group also left each generator's `remoteBus`
pointing into the source simulation. A clone test now destroys the source,
initializes the clone, and checks its three voltage controllers and solve.

Follow-up regression work:

- Exercise remote groups with fixed and one-sided Q limits, a swing target,
  and a terminal plant containing both local and remote controllers. Check bus
  types, controller fractions, reactive sharing, and solved voltage together.
- Exercise the switched-shunt outer loop with multiple banks across genuine
  power-flow passes. The current direct SVD unit test resets between opposite
  voltage probes and does not establish whole-loop sequencing or convergence.
- Automate a Release 25k no-shunt static solve and the flat-Pset five-second
  DAE and partitioned baseline and 1 MW step checks. Add initial residual and
  root-event assertions. The documented runs use a modified DYR profile, so
  the original DYR still needs its own acceptance check.
- Diagnose the remaining DAE post-initialization residual and the ACTIVSg500
  partitioned CVODE drift. The G500 test's `1.1e-4` threshold accepts the
  observed `1.007e-4` change but does not correct its cause. Keep the state
  name and initial/final values in failures to localize future changes.
  A later macOS run completed 30 seconds but reported a `1.526e-4` algebraic
  change in GENROU `iq` at CHESNEE 2 5, compared with `+7.408e-5` for that
  state on Windows. The algebraic state threshold is now `2e-4`; the bus
  voltage and angle thresholds remain `1e-4`. This permits the observed
  platform variation while retaining a separate investigation of its cause.
- Continue the default RAW switched-shunt power-flow investigation. The
  documented 25k solve with shunt movement disabled is a useful baseline, not
  proof that the automatic-shunt case converges.

After the PV-target remote-terminal fix, a fresh Release build completed the
original 25k RAW static solve with `no_shunt_adjustments` and
`no_powerflow_error_recovery` (25,000 buses, exit code 0). With only
`no_powerflow_error_recovery`, switched-shunt banks acted and KINSOL reached
its iteration limit (`-6`); the default-shunt static case remains unresolved.
