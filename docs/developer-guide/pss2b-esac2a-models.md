# PSS2B and ESAC2A

GridDyn provides native `pss2b` and `esac2a` factory models, DYR records, and
explicit positional DYD records. The equation reference is OpenIPSL commit
`8155c73f51ceeec935eb158247c7c043eb697ff5`, specifically:

- `Electrical/Controls/PSSE/PSS/PSS2B.mo` and
  `NonElectrical/Continuous/RampTrackingFilter.mo`.
- `Electrical/Controls/PSSE/ES/ESAC2A.mo`, its rotating-exciter and rectifier
  base classes, and `NonElectrical/Functions/SE.mo`.

## PSS2B equations and signals

The selected local inputs are clipped to `[VS1MIN, VS1MAX]` and
`[VS2MIN, VS2MAX]`. Each branch has two washouts followed by a lag; the second
branch includes `KS2`. With their outputs denoted `b1` and `b2`:

```text
W(T) = s*T/(1+s*T), L(T) = 1/(1+s*T)
b1 = W(TW1)*W(TW2)*L(T6)*u1
b2 = KS2*W(TW3)*W(TW4)*L(T7)*u2
u = b1 + KS3*b2
R(s) = (1+s*T8)^N / (1+s*T9)^(M+N)
v = KS1*(R(s)*u - b2)
VSS = clamp(LL(T1,T2)*LL(T3,T4)*LL(T10,T11)*v, VSTMIN, VSTMAX)
```

`LL(a,b) = (1+s*a)/(1+s*b)`. A zero washout time bypasses that washout.
A zero lag denominator requires a zero lead numerator. The ramp filter is
bypassed when either `M` or `N` is zero, matching OpenIPSL, and then `T9=0`
is allowed. `M` and `N` must be integers in `[0,8]` with `M*N <= 8`.

The supported signal subset is:

| MODE | GridDyn input                            | Limitation                                                            |
| ---- | ---------------------------------------- | --------------------------------------------------------------------- |
| 0    | Zero                                     | Disabled input branch                                                 |
| 1    | Machine speed minus 1 pu                 | Local machine only                                                    |
| 2    | Machine speed minus 1 pu                 | Speed proxy; no independent bus-frequency measurement                 |
| 3    | Terminal electrical power                | Machine base, local machine only                                      |
| 4    | Mechanical power minus its initial value | No electrical-power subtraction; not a full accelerating-power signal |
| 5    | Terminal voltage magnitude               | Local terminal only                                                   |

Nonzero `BUSR` and mode 6 are rejected.
PSS2B requests terminal electrical power, including its analytic derivatives,
to match the OpenIPSL SMIB test's `PELEC` connection. Existing stabilizers retain
their electrical torque route. Washout and downstream states initialize at
DC equilibrium, including fully bypassed washout branches.

PSS2A and PSS2B share branch, ramp-cascade, lead-lag, and Jacobian code. PSS2A
retains its fixed `M=5, N=1` realization and two lead-lag stages.

## ESAC2A equations

```text
VC = lag(TR, sensed/compensated voltage ECOMP)
Vreference = Vref + Vbias + Vset - 1
error = Vreference + VSS - VC - KF/TF*(VFE - feedbackState)
VA = bounded lag(KA, TA, LL(TC,TB)*error, VAMIN, VAMAX)
VFE = KE*VE + VE*SE(VE) + KD*XADIFD
VR = clamp(KB*(VA - KH*VFE), VRMIN, VRMAX)
dVE/dt = (VR - VFE)/TE
dfeedbackState/dt = (VFE - feedbackState)/TF
EFD = VE*FEX(KC*XADIFD/VE)
```

The field integrator is bounded below by zero. The upper bound follows the
OpenIPSL connection of **rectified EFD** to the ceiling's saturation block:

```text
VEmax*(KE + SE(EFD(VEmax))) = VFEMAX - KD*XADIFD
```

`ECOMP` is the generator's compensated voltage when a voltage compensator
is attached, and terminal voltage otherwise.

For normalized current `IN=KC*XADIFD/VE`, the shared rectifier uses:

```text
FEX(IN) = 1                         when IN<=0
          1-0.577*IN                when 0<IN<=0.433
          sqrt(0.75-IN^2)           when 0.433<IN<0.75
          1.732*(1-IN)              when 0.75<=IN<=1
          0                         when IN>1
```

At `VE<=1e-12`, ESAC2A returns zero field output to avoid division by zero.
Both exciters use these segments. Analytic Jacobian tests perturb each
segment away from its breakpoints; derivatives at a breakpoint use the
selected branch and are not two-sided slopes.

The implementation solves the monotone ceiling equation and differentiates it
implicitly for the limited field output and current Jacobian. A nonpositive
available ceiling is rejected. Saturation is disabled when `SE1=0`; otherwise
the two supplied saturation points define the OpenIPSL quadratic cutoff curve.
For enabled saturation, `a=sqrt(E1*SE1/(E2*SE2))`,
`A=(E1-a*E2)/(1-a)`, and `B=E2*SE2/(E2-A)^2`. The stored curve is
`E*SE(E)=B*(E-A)^2` above `A` and zero below it. The ceiling converts this
product back to `SE(EFD)` and differentiates the quotient.
All rectifier segments are included. `TR=0`, `TB=TC=0`, and `TA=0` bypasses
are supported. `TE`, `TF`, `KA`, `KB`, and `KE` must be positive.
`KC`, `KD`, and `KH` must be nonnegative. Saturation requires positive,
increasing voltage points with `E1*SE1 < E2*SE2` when enabled. Parameters
and required initial voltage, field-current, and enabled selector inputs
must be finite.

Regulator and field states stop outward integration at their limits and
resume when driven inward. Root callbacks project states into the permitted
range. The regulator output remains clipped if an explicit solver overshoots
its state. Held integrators retain explicit zero diagonal Jacobian entries
for CVODE's sparse matrix. Initialization solves the rectifier inverse and uses
`VA0 = (KH + 1/KB)*VFE0` to satisfy the actual equilibrium equations for
nondefault `KB/KH`; OpenIPSL initializes its regulator with `VFE0` directly.
The existing initial-upper-limit policy applies, with strict rejection
available through the normal simulation initialization flags.
GridDyn's bounded regulator and field states differ from OpenIPSL's
`SimpleLagLim` internal state/reset behavior. Limit-transition trajectory
parity has not been established.

Optional `uel_enabled` and `oel_enabled` parameters enable the OpenIPSL
absolute high and low selector inputs before the final VR clamp. They are
disabled by default. These inputs require **absolute selector levels**;
GridDyn limiter components that produce additive correction signals cannot
be connected interchangeably. DYR and DYD records do not enable these
optional selectors.

## Reader schemas

DYR records start with `bus 'MODEL' 'machine-id'`, end with `/`, and contain
exactly the following parameter payloads:

```text
PSS2B (29 values):
MODE1 BUSR1 MODE2 BUSR2 M N
TW1 TW2 T6 TW3 TW4 T7 KS2 KS3 T8 T9 KS1
T1 T2 T3 T4 T10 T11 VS1MAX VS1MIN VS2MAX VS2MIN VSTMAX VSTMIN

ESAC2A (22 values):
TR TB TC KA TA VAMAX VAMIN KB VRMAX VRMIN TE VFEMAX
KH KF TF KC KD KE E1 SE1 E2 SE2
```

The ESAC2A order is also confirmed by the existing `griddyn_test_cases/Base.dyr`
record. Explicit DYD records use the same payload after
`model bus "bus-name" base-kv "machine-id" :`, optionally preceded by a
version marker such as `#9`. These routes accept only these explicit payloads;
PSLF-specific selector/trailer variants and default parameter records are
not supported. Wrong field counts, nonnumeric values, and unavailable remote
signals produce reader errors. `disable_stabilizers` zeros PSS2B's `KS1`
through both readers.

## Validation

[testPSS2BESAC2A.cpp](../../test/componentTests/testPSS2BESAC2A.cpp)
checks equation values, equilibrium, variable ramp orders including the
maximum active orders, Jacobian allocation bounds,
input/output clipping, all rectifier segments, the moving field ceiling,
analytic Jacobians against finite differences, bypasses, and dynamic limiter
entry/release. `NewControlReaderTests` checks every imported parameter in both
formats, malformed records, diagnostic gain disabling, terminal power with
stator losses, coupled DAE and partitioned equations/Jacobians, and setpoint
responses. Limit entry and release are also checked in a coupled network with
IDA and partitioned CVODE. Initialization regressions reject nonfinite used
signals. External OpenIPSL trajectory comparisons remain pending.

See the equation comments in
[StabilizerPss2b.h](../../src/griddyn/stabilizers/StabilizerPss2b.h) and
[ExciterESAC2A.h](../../src/griddyn/exciters/ExciterESAC2A.h), the
[ESST2A notes](esst2a-model.md), and
[control signal routing](control-signal-routing.md).
