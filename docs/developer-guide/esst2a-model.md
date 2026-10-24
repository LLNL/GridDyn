# ESST2A

GridDyn's native `esst2a` model includes compound-source excitation,
supplementary limiter inputs, zero-time bypasses, and a compact DYR/explicit
DYD importer with dedicated equation and solver coverage.

## Equation reference

Reference: OpenIPSL commit `8155c73f51ceeec935eb158247c7c043eb697ff5`,
`Electrical/Controls/PSSE/ES/ESST2A.mo`, its rectifier block,
`SimpleLagLim.mo`, and `IntegratorLimVar.mo`.

```text
VE = |KP*VT + j*KI*IT|
VB = VE*FEX(KC*XADIFD/VE), or 1 when KP<=0 and KI<=0
TF*dXF/dt = EFD-XF
VF = KF/TF*(EFD-XF)
Vreference = Vref + Vbias + Vset - 1
error = Vreference + VSS - OELaction - VC - VF
regulatorInput = max(error, VUEL) when uel_enabled, otherwise error
TA*dVA/dt = KA*regulatorInput-VA
TE*dEFD/dt = VB*VA-KE*EFD
```

The shared rectifier helper translates GridDyn's machine dq signals to the
OpenIPSL current/voltage convention. All FEX segments are used, with analytic
derivatives through the compound source and field current.

`VC` filters the routed sensed/compensated voltage `ECOMP` with `TR`;
without a voltage compensator this is terminal voltage. The compound source
uses terminal `VT/IT` independently. `TR=0` bypasses measurement.
`TA=0` uses an instantaneous, clipped regulator. Bypassed measurement and
regulator states are retained as constant states to preserve the existing
state layout: `efd, vmeas, va, wf, efstate`.

`VA` is limited to `[VRMIN, VRMAX]`; `EFD` is limited to `[0, EFDMAX]`.
GridDyn uses bounded integration with root-driven entry, projection, and
release. Downstream regulator and field signals remain clipped if an
explicit step overshoots. This preserves the existing GridDyn non-windup
realization; OpenIPSL's `SimpleLagLim` keeps an internal lag state beyond
its output limits before resetting on release. External trajectory parity
at limit transitions remains unverified.

Initialization computes `VA0=KE*EFD0/VB0`, initializes feedback at `EFD0`,
and adjusts voltage bias to satisfy equilibrium. It honors the normal
initial-regulator-upper-limit policy and strict initialization flag.
Each initialization clears previous regulator and field limiter flags.
Field targets within `1e-7` of a bound are projected to that bound before
initializing output, feedback, and regulator states. Other out-of-range field
targets are rejected. Parameters and required initial signals must be finite;
`KA`, `TF`, and `TE` must be positive, `TR`/`TA`/`KC`/`EFDMAX` nonnegative,
and `VRMIN <= VRMAX`. The compound source requires synchronous-machine dq
and field-current inputs unless both source gains are nonpositive.

## Supplementary limiter inputs

OpenIPSL adds its signed `VOEL` before the UEL high selector. GridDyn's OEL
components produce positive reduction actions, so ESST2A subtracts that
action at the same junction. OEL routing is supported by default and its
initial action is included in voltage bias.

`uel_enabled=1` enables the OpenIPSL absolute `VUEL` high selector before
`KA`. It defaults to disabled because a zero neutral additive UEL action
would otherwise clamp negative voltage error. This selector requires an
absolute level: additive UEL components are not interchangeable. An active
selector that makes the requested initial equilibrium inconsistent is rejected.

## Reader schema

The existing GridDyn 13-value core schema is preserved:

```text
TR KA TA VRMAX VRMIN KP KI KC KF TF KE TE EFDMAX
```

DYR: `bus 'ESST2A' 'machine-id' <13 values> /`.

Explicit DYD: `esst2a bus "bus-name" base-kv "machine-id" : #9 <13 values> /`.
The version marker is optional. These are explicit core records; PSLF
selector/trailer variants and `def` parameter records are rejected. Numeric
values must be finite; wrong field counts and nonnumeric values produce
reader errors. `uel_enabled` is configured directly on the model, outside
the positional schema.

The compact order is GridDyn's existing contract; general PSLF compatibility
has not been verified from an authoritative explicit ESST2A record.

## Validation

- Compound-source equation values and all rectifier segments; analytic
  Jacobians against finite differences, including nonzero `cj`.
- TR/TA and compound-source bypasses, OEL summation, UEL selector branches,
  factory/clone behavior, parameter validation, near-boundary initial
  consistency, and reinitialization.
- Every parameter through DYR and DYD, malformed records, and generator steps.
- PSS2B and OEL coupling with partitioned algebraic/differential residuals,
  updates, derivatives, and Jacobians before and after a reference step.
- Regulator and field limit entry/release with IDA and partitioned CVODE.

Independent OpenIPSL trajectories remain pending.

Equation comments are in
[ExciterESST2A.h](../../src/griddyn/exciters/ExciterESST2A.h).
Dedicated checks are in
[testExciters.cpp](../../test/componentTests/testExciters.cpp) (`Esst2aTests`)
and [testModelComparison.cpp](../../test/comparisonTests/testModelComparison.cpp)
(`Esst2aReaderTests`). See also [PSS2B and ESAC2A](pss2b-esac2a-models.md)
and [control signal routing](control-signal-routing.md).
