function mpc = case6ww_res
%CASE6WW_RES

%% MATPOWER Case Format : Version 2
mpc.version = '2';

%%-----  Power Flow Data  -----%%
%% system MVA base
mpc.baseMVA = 100;

%% bus data
%    bus_i    type    Pd    Qd    Gs    Bs    area    Vm    Va    baseKV    zone    Vmax    Vmin
mpc.bus = [
    1    3    0    0    0    0    1    1.05    0    230    1    1.05    1.05;
    2    2    0    0    0    0    1    1.05    -3.67115737    230    1    1.05    1.05;
    3    2    0    0    0    0    1    1.07    -4.27326747    230    1    1.07    1.07;
    4    1    70    70    0    0    1    0.989372869    -4.19582164    230    1    1.05    0.95;
    5    1    70    70    0    0    1    0.985444835    -5.27638828    230    1    1.05    0.95;
    6    1    70    70    0    0    1    1.00442527    -5.94745402    230    1    1.05    0.95;
];

%% generator data
%    bus    Pg    Qg    Qmax    Qmin    Vg    mBase    status    Pmax    Pmin    Pc1    Pc2    Qc1min    Qc1max    Qc2min    Qc2max    ramp_agc    ramp_10    ramp_30    ramp_q    apf
mpc.gen = [
    1    107.875497    15.9562063    100    -100    1.05    100    1    200    50    0    0    0    0    0    0    0    0    0    0    0;
    2    50    74.3564751    100    -100    1.05    100    1    150    37.5    0    0    0    0    0    0    0    0    0    0    0;
    3    60    89.6267745    100    -100    1.07    100    1    180    45    0    0    0    0    0    0    0    0    0    0    0;
];

%% branch data
%    fbus    tbus    r    x    b    rateA    rateB    rateC    ratio    angle    status    angmin    angmax    Pf    Qf    Pt    Qt
mpc.branch = [
    1    2    0.1    0.2    0.04    40    40    40    0    0    1    -360    360    28.6897    -15.4187    -27.7847    12.8185;
    1    4    0.05    0.2    0.04    60    60    60    0    0    1    -360    360    43.5849    20.1201    -42.4974    -19.9326;
    1    5    0.08    0.3    0.06    40    40    40    0    0    1    -360    360    35.6009    11.2547    -34.5273    -13.4497;
    2    3    0.05    0.25    0.06    40    40    40    0    0    1    -360    360    2.9303    -12.2687    -2.8900    5.7281;
    2    4    0.05    0.1    0.02    60    60    60    0    0    1    -360    360    33.0909    46.0541    -31.5858    -45.1252;
    2    5    0.1    0.3    0.04    30    30    30    0    0    1    -360    360    15.5145    15.3532    -15.0166    -18.0065;
    2    6    0.07    0.2    0.05    90    90    90    0    0    1    -360    360    26.2489    12.3995    -25.6656    -16.0113;
    3    5    0.12    0.26    0.05    70    70    70    0    0    1    -360    360    19.1168    23.1745    -18.0232    -26.0950;
    3    6    0.02    0.1    0.02    80    80    80    0    0    1    -360    360    43.7732    60.7242    -42.7698    -57.8610;
    4    5    0.2    0.4    0.08    20    20    20    0    0    1    -360    360    4.0832    -4.9421    -4.0470    -2.7853;
    5    6    0.1    0.3    0.06    40    40    40    0    0    1    -360    360    1.6142    -9.6635    -1.5646    3.8723;
];

%%-----  OPF Data  -----%%
%% generator cost data
%    1    startup    shutdown    n    x1    y1    ...    xn    yn
%    2    startup    shutdown    n    c(n-1)    ...    c0
mpc.gencost = [
    2    0    0    3    0.00533    11.669    213.1;
    2    0    0    3    0.00889    10.333    200;
    2    0    0    3    0.00741    10.833    240;
];
