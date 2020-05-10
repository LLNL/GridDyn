function mpc = case9Q_resqlim
%CASE9Q_RESQLIM

%% MATPOWER Case Format : Version 2
mpc.version = '2';

%%-----  Power Flow Data  -----%%
%% system MVA base
mpc.baseMVA = 100;

%% bus data
%    bus_i    type    Pd    Qd    Gs    Bs    area    Vm    Va    baseKV    zone    Vmax    Vmin
mpc.bus = [
    1    3    0    0    0    0    1    1    0    345    1    1.1    0.9;
    2    2    0    0    0    0    1    1    9.66874113    345    1    1.1    0.9;
    3    2    0    0    0    0    1    1    4.77107324    345    1    1.1    0.9;
    4    1    0    0    0    0    1    0.987006852    -2.40664392    345    1    1.1    0.9;
    5    1    90    30    0    0    1    0.975472177    -4.01726433    345    1    1.1    0.9;
    6    1    0    0    0    0    1    1.00337544    1.92560169    345    1    1.1    0.9;
    7    1    100    35    0    0    1    0.985644882    0.621544555    345    1    1.1    0.9;
    8    1    0    0    0    0    1    0.996185246    3.79912019    345    1    1.1    0.9;
    9    1    125    50    0    0    1    0.95762104    -4.34993358    345    1    1.1    0.9;
];

%% generator data
%    bus    Pg    Qg    Qmax    Qmin    Vg    mBase    status    Pmax    Pmin    Pc1    Pc2    Qc1min    Qc1max    Qc2min    Qc2max    ramp_agc    ramp_10    ramp_30    ramp_q    apf
mpc.gen = [
    1    71.9547016    24.0689578    300    -300    1    100    1    250    10    0    0    0    0    0    0    0    0    0    0    0;
    2    163    14.4601195    300    -300    1    100    1    300    10    0    0    0    0    0    0    0    0    0    0    0;
    3    85    -3.64902553    300    -300    1    100    1    270    10    0    0    0    0    0    0    0    0    0    0    0;
];

%% branch data
%    fbus    tbus    r    x    b    rateA    rateB    rateC    ratio    angle    status    angmin    angmax    Pf    Qf    Pt    Qt
mpc.branch = [
    1    4    0    0.0576    0    250    250    250    0    0    1    -360    360    71.9547    24.0690    -71.9547    -20.7530;
    4    5    0.017    0.092    0.158    250    250    250    0    0    1    -360    360    30.7283    -0.5859    -30.5547    -13.6880;
    5    6    0.039    0.17    0.358    150    150    150    0    0    1    -360    360    -59.4453    -16.3120    60.8939    -12.4275;
    3    6    0    0.0586    0    300    300    300    0    0    1    -360    360    85.0000    -3.6490    -85.0000    7.8907;
    6    7    0.0119    0.1008    0.209    150    150    150    0    0    1    -360    360    24.1061    4.5368    -24.0106    -24.4008;
    7    8    0.0085    0.072    0.149    250    250    250    0    0    1    -360    360    -75.9894    -10.5992    76.4956    0.2562;
    8    2    0    0.0625    0    250    250    250    0    0    1    -360    360    -163.0000    2.2762    163.0000    14.4601;
    8    9    0.032    0.161    0.306    250    250    250    0    0    1    -360    360    86.5044    -2.5324    -84.0399    -14.2820;
    9    4    0.01    0.085    0.176    250    250    250    0    0    1    -360    360    -40.9601    -35.7180    41.2264    21.3389;
];

%%-----  OPF Data  -----%%
%% generator cost data
%    1    startup    shutdown    n    x1    y1    ...    xn    yn
%    2    startup    shutdown    n    c(n-1)    ...    c0
mpc.gencost = [
    2    1500    0    3    0.11    5    150;
    2    2000    0    3    0.085    1.2    600;
    2    3000    0    3    0.1225    1    335;
    2    0    0    3    0.2    0    0;
    2    0    0    3    0.05    0    0;
    2    0    0    3    0.3    0    0;
];
