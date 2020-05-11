function mpc = case57_resqlim
%CASE57_RESQLIM

%% MATPOWER Case Format : Version 2
mpc.version = '2';

%%-----  Power Flow Data  -----%%
%% system MVA base
mpc.baseMVA = 100;

%% bus data
%    bus_i    type    Pd    Qd    Gs    Bs    area    Vm    Va    baseKV    zone    Vmax    Vmin
mpc.bus = [
    1    3    55    17    0    0    1    1.04    0    0    1    1.06    0.94;
    2    2    3    88    0    0    1    1.01    -1.18816284    0    1    1.06    0.94;
    3    2    41    21    0    0    1    0.985    -5.98812672    0    1    1.06    0.94;
    4    1    0    0    0    0    1    0.980779628    -7.33736538    0    1    1.06    0.94;
    5    1    13    4    0    0    1    0.976498679    -8.5464095    0    1    1.06    0.94;
    6    2    75    2    0    0    1    0.98    -8.67411976    0    1    1.06    0.94;
    7    1    0    0    0    0    1    0.984201657    -7.60139768    0    1    1.06    0.94;
    8    2    150    22    0    0    1    1.005    -4.47791166    0    1    1.06    0.94;
    9    2    121    26    0    0    1    0.98    -9.58467153    0    1    1.06    0.94;
    10    1    5    2    0    0    1    0.986242027    -11.4496577    0    1    1.06    0.94;
    11    1    0    0    0    0    1    0.9739623    -10.1932486    0    1    1.06    0.94;
    12    2    377    24    0    0    1    1.015    -10.471211    0    1    1.06    0.94;
    13    1    18    2.3    0    0    1    0.978887415    -9.80351812    0    1    1.06    0.94;
    14    1    10.5    5.3    0    0    1    0.970176849    -9.35030557    0    1    1.06    0.94;
    15    1    22    5    0    0    1    0.988031614    -7.19016974    0    1    1.06    0.94;
    16    1    43    3    0    0    1    1.01336863    -8.85893533    0    1    1.06    0.94;
    17    1    42    8    0    0    1    1.01745427    -5.39589313    0    1    1.06    0.94;
    18    1    27.2    9.8    0    10    1    1.00065923    -11.7296389    0    1    1.06    0.94;
    19    1    3.3    0.6    0    0    1    0.970157847    -13.2265062    0    1    1.06    0.94;
    20    1    2.3    1    0    0    1    0.963790166    -13.4443088    0    1    1.06    0.94;
    21    1    0    0    0    0    1    1.00849826    -12.9290054    0    1    1.06    0.94;
    22    1    0    0    0    0    1    1.00974399    -12.8743092    0    1    1.06    0.94;
    23    1    6.3    2.1    0    0    1    1.00832989    -12.9395573    0    1    1.06    0.94;
    24    1    0    0    0    0    1    0.999233039    -13.2921201    0    1    1.06    0.94;
    25    1    6.3    3.2    0    5.9    1    0.982520777    -18.1732262    0    1    1.06    0.94;
    26    1    0    0    0    0    1    0.958818251    -12.9812574    0    1    1.06    0.94;
    27    1    9.3    0.5    0    0    1    0.981541101    -11.5136204    0    1    1.06    0.94;
    28    1    4.6    2.3    0    0    1    0.996677987    -10.4816083    0    1    1.06    0.94;
    29    1    17    2.6    0    0    1    1.01021987    -9.77177796    0    1    1.06    0.94;
    30    1    3.6    1.8    0    0    1    0.962661348    -18.7196455    0    1    1.06    0.94;
    31    1    5.8    2.9    0    0    1    0.93593245    -19.3838048    0    1    1.06    0.94;
    32    1    1.6    0.8    0    0    1    0.949874718    -18.5123362    0    1    1.06    0.94;
    33    1    3.8    1.9    0    0    1    0.947580648    -18.5520067    0    1    1.06    0.94;
    34    1    0    0    0    0    1    0.959200035    -14.1489642    0    1    1.06    0.94;
    35    1    6    3    0    0    1    0.966211937    -13.9061916    0    1    1.06    0.94;
    36    1    0    0    0    0    1    0.975828097    -13.6348162    0    1    1.06    0.94;
    37    1    0    0    0    0    1    0.984886491    -13.4459203    0    1    1.06    0.94;
    38    1    14    7    0    0    1    1.01281249    -12.7346231    0    1    1.06    0.94;
    39    1    0    0    0    0    1    0.982823076    -13.4910325    0    1    1.06    0.94;
    40    1    0    0    0    0    1    0.97281069    -13.6582385    0    1    1.06    0.94;
    41    1    6.3    3    0    0    1    0.996216752    -14.0766801    0    1    1.06    0.94;
    42    1    7.1    4.4    0    0    1    0.966525946    -15.5327899    0    1    1.06    0.94;
    43    1    2    1    0    0    1    1.00956379    -11.3543904    0    1    1.06    0.94;
    44    1    12    1.8    0    0    1    1.0167986    -11.8564642    0    1    1.06    0.94;
    45    1    0    0    0    0    1    1.03600497    -9.27009499    0    1    1.06    0.94;
    46    1    0    0    0    0    1    1.05979746    -11.1160699    0    1    1.06    0.94;
    47    1    29.7    11.6    0    0    1    1.03325137    -12.51159    0    1    1.06    0.94;
    48    1    0    0    0    0    1    1.02735055    -12.6106565    0    1    1.06    0.94;
    49    1    18    8.5    0    0    1    1.03624558    -12.936082    0    1    1.06    0.94;
    50    1    21    10.5    0    0    1    1.02333611    -13.4127121    0    1    1.06    0.94;
    51    1    18    5.3    0    0    1    1.05226209    -12.5333981    0    1    1.06    0.94;
    52    1    4.9    2.2    0    0    1    0.980368499    -11.49756    0    1    1.06    0.94;
    53    1    20    10    0    6.3    1    0.970945515    -12.2525906    0    1    1.06    0.94;
    54    1    4.1    1.4    0    0    1    0.996318797    -11.7096563    0    1    1.06    0.94;
    55    1    6.8    3.4    0    0    1    1.03078604    -10.801127    0    1    1.06    0.94;
    56    1    7.6    2.2    0    0    1    0.968368536    -16.0650692    0    1    1.06    0.94;
    57    1    6.7    2    0    0    1    0.964826006    -16.5836973    0    1    1.06    0.94;
];

%% generator data
%    bus    Pg    Qg    Qmax    Qmin    Vg    mBase    status    Pmax    Pmin    Pc1    Pc2    Qc1min    Qc1max    Qc2min    Qc2max    ramp_agc    ramp_10    ramp_30    ramp_q    apf
mpc.gen = [
    1    478.663752    128.849628    200    -140    1.04    100    1    575.88    0    0    0    0    0    0    0    0    0    0    0    0;
    2    0    -0.754984179    50    -17    1.01    100    1    100    0    0    0    0    0    0    0    0    0    0    0    0;
    3    40    -0.90490135    60    -10    0.985    100    1    140    0    0    0    0    0    0    0    0    0    0    0    0;
    6    0    0.871401253    25    -8    0.98    100    1    100    0    0    0    0    0    0    0    0    0    0    0    0;
    8    450    62.0996012    200    -140    1.005    100    1    550    0    0    0    0    0    0    0    0    0    0    0    0;
    9    0    2.28837516    9    -3    0.98    100    1    100    0    0    0    0    0    0    0    0    0    0    0    0;
    12    310    128.630884    155    -150    1.015    100    1    410    0    0    0    0    0    0    0    0    0    0    0    0;
];

%% branch data
%    fbus    tbus    r    x    b    rateA    rateB    rateC    ratio    angle    status    angmin    angmax    Pf    Qf    Pt    Qt
mpc.branch = [
    1    2    0.0083    0.028    0.129    0    0    0    0    0    1    -360    360    102.0883    74.9969    -100.7729    -84.1154;
    2    3    0.0298    0.085    0.0818    0    0    0    0    0    1    -360    360    97.7729    -4.6396    -94.9802    4.4649;
    3    4    0.0112    0.0366    0.038    0    0    0    0    0    1    -360    360    60.2128    -8.1793    -59.7896    5.8910;
    4    5    0.0625    0.132    0.0258    0    0    0    0    0    1    -360    360    13.7984    -4.4319    -13.6681    2.2362;
    4    6    0.043    0.148    0.0348    0    0    0    0    0    1    -360    360    14.1569    -5.0935    -14.0621    2.0750;
    6    7    0.02    0.102    0.0276    0    0    0    0    0    1    -360    360    -17.7786    -1.7105    17.8445    -0.6157;
    6    8    0.0339    0.173    0.047    0    0    0    0    0    1    -360    360    -42.5014    -6.5643    43.1456    5.2211;
    8    9    0.0099    0.0505    0.0548    0    0    0    0    0    1    -360    360    178.0287    19.8259    -174.8720    -9.1229;
    9    10    0.0369    0.1679    0.044    0    0    0    0    0    1    -360    360    17.1711    -9.2251    -17.0384    5.5762;
    9    11    0.0258    0.0848    0.0218    0    0    0    0    0    1    -360    360    12.9030    2.0685    -12.8557    -3.9937;
    9    12    0.0648    0.295    0.0772    0    0    0    0    0    1    -360    360    2.5489    -15.8538    -2.4450    8.6431;
    9    13    0.0481    0.158    0.0406    0    0    0    0    0    1    -360    360    2.3159    -1.9601    -2.3132    -1.9258;
    13    14    0.0132    0.0434    0.011    0    0    0    0    0    1    -360    360    -10.3547    22.3375    10.4415    -23.0968;
    13    15    0.0269    0.0869    0.023    0    0    0    0    0    1    -360    360    -48.8920    4.8896    49.5731    -4.9138;
    1    15    0.0178    0.091    0.0988    0    0    0    0    0    1    -360    360    148.9854    33.7868    -145.0805    -23.9890;
    1    16    0.0454    0.206    0.0546    0    0    0    0    0    1    -360    360    79.2472    -0.8698    -76.6093    7.0828;
    1    17    0.0238    0.108    0.0286    0    0    0    0    0    1    -360    360    93.3428    3.9357    -91.4190    1.7673;
    3    15    0.0162    0.053    0.0544    0    0    0    0    0    1    -360    360    33.7674    -18.1905    -33.5367    13.6512;
    4    18    0    0.555    0    0    0    0    0.97    0    1    -360    360    13.9616    2.4399    -13.9616    -1.3494;
    4    18    0    0.43    0    0    0    0    0.978    0    1    -360    360    17.8728    1.1945    -17.8728    0.1774;
    5    6    0.0302    0.0641    0.0124    0    0    0    0    0    1    -360    360    0.6681    -6.2362    -0.6579    5.0712;
    7    8    0.0139    0.0712    0.0194    0    0    0    0    0    1    -360    360    -77.9353    -12.4106    78.8257    15.0526;
    10    12    0.0277    0.1262    0.0328    0    0    0    0    0    1    -360    360    -17.6045    -20.0896    17.7902    17.6507;
    11    13    0.0223    0.0732    0.0188    0    0    0    0    0    1    -360    360    -9.9254    -4.3909    9.9514    2.6840;
    12    13    0.0178    0.058    0.0604    0    0    0    0    0    1    -360    360    -0.4861    60.3512    1.1820    -64.0888;
    12    16    0.018    0.0813    0.0216    0    0    0    0    0    1    -360    360    -33.3972    8.8191    33.6093    -10.0828;
    12    17    0.0397    0.179    0.0476    0    0    0    0    0    1    -360    360    -48.4619    9.1667    49.4190    -9.7673;
    14    15    0.0171    0.0547    0.0148    0    0    0    0    0    1    -360    360    -68.8359    -9.5999    69.7112    10.9808;
    18    19    0.461    0.685    0    0    0    0    0    0    1    -360    360    4.6343    1.3852    -4.5266    -1.2251;
    19    20    0.283    0.434    0    0    0    0    0    0    1    -360    360    1.2266    0.6251    -1.2209    -0.6164;
    21    20    0    0.7767    0    0    0    0    1.043    0    1    -360    360    1.0791    0.3946    -1.0791    -0.3836;
    21    22    0.0736    0.117    0    0    0    0    0    0    1    -360    360    -1.0791    -0.3946    1.0800    0.3961;
    22    23    0.0099    0.0152    0    0    0    0    0    0    1    -360    360    9.6540    3.1105    -9.6440    -3.0951;
    23    24    0.166    0.256    0.0084    0    0    0    0    0    1    -360    360    3.3440    0.9951    -3.3224    -1.8083;
    24    25    0    1.182    0    0    0    0    1    0    1    -360    360    7.0674    1.7140    -7.0674    -1.0880;
    24    25    0    1.23    0    0    0    0    1    0    1    -360    360    6.7916    1.6471    -6.7916    -1.0455;
    24    26    0    0.0473    0    0    0    0    1.043    0    1    -360    360    -10.5366    -1.5529    10.5366    1.6114;
    26    27    0.165    0.254    0    0    0    0    0    0    1    -360    360    -10.5366    -1.6114    10.7405    1.9253;
    27    28    0.0618    0.0954    0    0    0    0    0    0    1    -360    360    -20.0405    -2.4253    20.3019    2.8288;
    28    29    0.0418    0.0587    0    0    0    0    0    0    1    -360    360    -24.9019    -5.1288    25.1739    5.5108;
    7    29    0    0.0648    0    0    0    0    0.967    0    1    -360    360    60.0908    13.0263    -60.0908    -10.6614;
    25    30    0.135    0.202    0    0    0    0    0    0    1    -360    360    7.5590    4.6290    -7.4492    -4.4646;
    30    31    0.326    0.497    0    0    0    0    0    0    1    -360    360    3.8492    2.6646    -3.7721    -2.5471;
    31    32    0.507    0.755    0    0    0    0    0    0    1    -360    360    -2.0279    -0.3529    2.0524    0.3894;
    32    33    0.0392    0.036    0    0    0    0    0    0    1    -360    360    3.8079    1.9072    -3.8000    -1.9000;
    34    32    0    0.953    0    0    0    0    0.975    0    1    -360    360    7.4603    3.7858    -7.4603    -3.0967;
    34    35    0.052    0.078    0.0032    0    0    0    0    0    1    -360    360    -7.4603    -3.7858    7.4993    3.5477;
    35    36    0.043    0.0537    0.0016    0    0    0    0    0    1    -360    360    -13.4993    -6.5477    13.6025    6.5257;
    36    37    0.029    0.0366    0    0    0    0    0    0    1    -360    360    -17.0672    -10.6140    17.1902    10.7693;
    37    38    0.0651    0.1009    0.002    0    0    0    0    0    1    -360    360    -21.0486    -13.6990    21.4702    14.1527;
    37    39    0.0239    0.0379    0    0    0    0    0    0    1    -360    360    3.8584    2.9297    -3.8526    -2.9206;
    36    40    0.03    0.0466    0    0    0    0    0    0    1    -360    360    3.4647    4.0883    -3.4557    -4.0742;
    22    38    0.0192    0.0295    0    0    0    0    0    0    1    -360    360    -10.7340    -3.5065    10.7580    3.5434;
    11    41    0    0.749    0    0    0    0    0.955    0    1    -360    360    9.1869    3.5302    -9.1869    -2.8327;
    41    42    0.207    0.352    0    0    0    0    0    0    1    -360    360    8.8752    3.2721    -8.6886    -2.9547;
    41    43    0    0.412    0    0    0    0    0    0    1    -360    360    -11.5941    -2.9518    11.5941    3.5460;
    38    44    0.0289    0.0585    0.002    0    0    0    0    0    1    -360    360    -24.3455    5.2301    24.5205    -5.0819;
    15    45    0    0.1042    0    0    0    0    0.955    0    1    -360    360    37.3329    -0.7291    -37.3329    2.0864;
    14    46    0    0.0735    0    0    0    0    0.9    0    1    -360    360    47.8945    27.3968    -47.8945    -25.4711;
    46    47    0.023    0.068    0.0032    0    0    0    0    0    1    -360    360    47.8945    25.4711    -47.2900    -24.0345;
    47    48    0.0182    0.0233    0    0    0    0    0    0    1    -360    360    17.5900    12.4345    -17.5109    -12.3332;
    48    49    0.0834    0.129    0.0048    0    0    0    0    0    1    -360    360    0.0818    -7.3768    -0.0417    6.9279;
    49    50    0.0801    0.128    0    0    0    0    0    0    1    -360    360    9.6649    4.4316    -9.5806    -4.2969;
    50    51    0.1386    0.22    0    0    0    0    0    0    1    -360    360    -11.4194    -6.2031    11.6429    6.5579;
    10    51    0    0.0712    0    0    0    0    0.93    0    1    -360    360    29.6429    12.5134    -29.6429    -11.8579;
    13    49    0    0.191    0    0    0    0    0.895    0    1    -360    360    32.4265    33.8035    -32.4265    -30.3002;
    29    52    0.1442    0.187    0    0    0    0    0    0    1    -360    360    17.9168    2.5506    -17.4541    -1.9505;
    52    53    0.0762    0.0984    0    0    0    0    0    0    1    -360    360    12.5541    -0.2495    -12.4291    0.4110;
    53    54    0.1878    0.232    0    0    0    0    0    0    1    -360    360    -7.5709    -4.4717    7.7250    4.6620;
    54    55    0.1732    0.2265    0    0    0    0    0    0    1    -360    360    -11.8250    -6.0620    12.1331    6.4649;
    11    43    0    0.153    0    0    0    0    0.958    0    1    -360    360    13.5941    4.8545    -13.5941    -4.5460;
    44    45    0.0624    0.1242    0.004    0    0    0    0    0    1    -360    360    -36.5205    3.2819    37.3329    -2.0864;
    40    56    0    1.195    0    0    0    0    0.958    0    1    -360    360    3.4557    4.0742    -3.4557    -3.7435;
    56    41    0.553    0.549    0    0    0    0    0    0    1    -360    360    -5.4294    0.6627    5.6059    -0.4876;
    56    42    0.2125    0.354    0    0    0    0    0    0    1    -360    360    -1.5781    1.4628    1.5886    -1.4453;
    39    57    0    1.355    0    0    0    0    0.98    0    1    -360    360    3.8526    2.9206    -3.8526    -2.6057;
    57    56    0.174    0.26    0    0    0    0    0    0    1    -360    360    -2.8474    0.6057    2.8632    -0.5820;
    38    49    0.115    0.177    0.003    0    0    0    0    0    1    -360    360    -4.6582    -10.5324    4.8033    10.4407;
    38    48    0.0312    0.0482    0    0    0    0    0    0    1    -360    360    -17.2244    -19.3939    17.4291    19.7101;
    9    55    0    0.1205    0    0    0    0    0.94    0    1    -360    360    18.9331    10.3818    -18.9331    -9.8649;
];

%%-----  OPF Data  -----%%
%% generator cost data
%    1    startup    shutdown    n    x1    y1    ...    xn    yn
%    2    startup    shutdown    n    c(n-1)    ...    c0
mpc.gencost = [
    2    0    0    3    0.077579519    20    0;
    2    0    0    3    0.01    40    0;
    2    0    0    3    0.25    20    0;
    2    0    0    3    0.01    40    0;
    2    0    0    3    0.0222222222    20    0;
    2    0    0    3    0.01    40    0;
    2    0    0    3    0.0322580645    20    0;
];

%% bus names
mpc.bus_name = {
    'Kanawha   V1';
    'Turner    V1';
    'Logan     V1';
    'Sprigg    V1';
    'Bus 5     V1';
    'Beaver Ck V1';
    'Bus 7     V1';
    'Clinch Rv V1';
    'Saltville V1';
    'Bus 10    V1';
    'Tazewell  V1';
    'Glen Lyn  V1';
    'Bus 13    V1';
    'Bus 14    V1';
    'Bus 15    V1';
    'Bus 16    V1';
    'Bus 17    V1';
    'Sprigg    V2';
    'Bus 19    V2';
    'Bus 20    V2';
    'Bus 21    V3';
    'Bus 22    V3';
    'Bus 23    V3';
    'Bus 24    V3';
    'Bus 25    V4';
    'Bus 26    V5';
    'Bus 27    V5';
    'Bus 28    V5';
    'Bus 29    V5';
    'Bus 30    V4';
    'Bus 31    V4';
    'Bus 32    V4';
    'Bus 33    V4';
    'Bus 34    V3';
    'Bus 35    V3';
    'Bus 36    V3';
    'Bus 37    V3';
    'Bus 38    V3';
    'Bus 39    V3';
    'Bus 40    V3';
    'Tazewell  V6';
    'Bus 42    V6';
    'Tazewell  V7';
    'Bus 44    V3';
    'Bus 45    V3';
    'Bus 46    V3';
    'Bus 47    V3';
    'Bus 48    V3';
    'Bus 49    V3';
    'Bus 50    V3';
    'Bus 51    V3';
    'Bus 52    V5';
    'Bus 53    V5';
    'Bus 54    V5';
    'Saltville V5';
    'Bus 56    V6';
    'Bus 57    V6';
};
