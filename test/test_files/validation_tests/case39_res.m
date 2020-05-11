function mpc = case39_res
%CASE39_RES

%% MATPOWER Case Format : Version 2
mpc.version = '2';

%%-----  Power Flow Data  -----%%
%% system MVA base
mpc.baseMVA = 100;

%% bus data
%    bus_i    type    Pd    Qd    Gs    Bs    area    Vm    Va    baseKV    zone    Vmax    Vmin
mpc.bus = [
    1    1    97.6    44.2    0    0    2    1.03938364    -13.5366018    345    1    1.06    0.94;
    2    1    0    0    0    0    2    1.04849411    -9.78526661    345    1    1.06    0.94;
    3    1    322    2.4    0    0    2    1.03070771    -12.2763836    345    1    1.06    0.94;
    4    1    500    184    0    0    1    1.00445997    -12.6267345    345    1    1.06    0.94;
    5    1    0    0    0    0    1    1.00600626    -11.1923388    345    1    1.06    0.94;
    6    1    0    0    0    0    1    1.00822558    -10.4083301    345    1    1.06    0.94;
    7    1    233.8    84    0    0    1    0.998397283    -12.7556255    345    1    1.06    0.94;
    8    1    522    176.6    0    0    1    0.997872316    -13.3358436    345    1    1.06    0.94;
    9    1    6.5    -66.6    0    0    1    1.03833197    -14.1784416    345    1    1.06    0.94;
    10    1    0    0    0    0    1    1.01784313    -8.17087504    345    1    1.06    0.94;
    11    1    0    0    0    0    1    1.01338578    -8.93696634    345    1    1.06    0.94;
    12    1    8.53    88    0    0    1    1.00081503    -8.99882365    345    1    1.06    0.94;
    13    1    0    0    0    0    1    1.01492296    -8.92992719    345    1    1.06    0.94;
    14    1    0    0    0    0    1    1.01231896    -10.7152948    345    1    1.06    0.94;
    15    1    320    153    0    0    3    1.01618536    -11.3453995    345    1    1.06    0.94;
    16    1    329    32.3    0    0    3    1.03252026    -10.0333483    345    1    1.06    0.94;
    17    1    0    0    0    0    2    1.03423651    -11.116436    345    1    1.06    0.94;
    18    1    158    30    0    0    2    1.0315726    -11.9861679    345    1    1.06    0.94;
    19    1    0    0    0    0    3    1.05010676    -5.41007288    345    1    1.06    0.94;
    20    1    680    103    0    0    3    0.991010543    -6.82117827    345    1    1.06    0.94;
    21    1    274    115    0    0    3    1.03231918    -7.62874607    345    1    1.06    0.94;
    22    1    0    0    0    0    3    1.05014274    -3.18311986    345    1    1.06    0.94;
    23    1    247.5    84.6    0    0    3    1.04514508    -3.38127627    345    1    1.06    0.94;
    24    1    308.6    -92.2    0    0    3    1.03800101    -9.91375854    345    1    1.06    0.94;
    25    1    224    47.2    0    0    2    1.05768275    -8.3692354    345    1    1.06    0.94;
    26    1    139    17    0    0    2    1.05256129    -9.43876958    345    1    1.06    0.94;
    27    1    281    75.5    0    0    2    1.03834491    -11.3621519    345    1    1.06    0.94;
    28    1    206    27.6    0    0    3    1.05037366    -5.92835921    345    1    1.06    0.94;
    29    1    283.5    26.9    0    0    3    1.0501149    -3.16987411    345    1    1.06    0.94;
    30    2    0    0    0    0    2    1.0499    -7.3704746    345    1    1.06    0.94;
    31    3    9.2    4.6    0    0    1    0.982    0    345    1    1.06    0.94;
    32    2    0    0    0    0    1    0.9841    -0.188437397    345    1    1.06    0.94;
    33    2    0    0    0    0    3    0.9972    -0.193174455    345    1    1.06    0.94;
    34    2    0    0    0    0    3    1.0123    -1.63111902    345    1    1.06    0.94;
    35    2    0    0    0    0    3    1.0494    1.77650688    345    1    1.06    0.94;
    36    2    0    0    0    0    3    1.0636    4.46843745    345    1    1.06    0.94;
    37    2    0    0    0    0    2    1.0275    -1.58289875    345    1    1.06    0.94;
    38    2    0    0    0    0    3    1.0265    3.89281774    345    1    1.06    0.94;
    39    2    1104    250    0    0    1    1.03    -14.5352562    345    1    1.06    0.94;
];

%% generator data
%    bus    Pg    Qg    Qmax    Qmin    Vg    mBase    status    Pmax    Pmin    Pc1    Pc2    Qc1min    Qc1max    Qc2min    Qc2max    ramp_agc    ramp_10    ramp_30    ramp_q    apf
mpc.gen = [
    30    250    161.761648    400    140    1.0499    100    1    1040    0    0    0    0    0    0    0    0    0    0    0    0;
    31    677.871126    221.574486    300    -100    0.982    100    1    646    0    0    0    0    0    0    0    0    0    0    0    0;
    32    650    206.964855    300    150    0.9841    100    1    725    0    0    0    0    0    0    0    0    0    0    0    0;
    33    632    108.292787    250    0    0.9972    100    1    652    0    0    0    0    0    0    0    0    0    0    0    0;
    34    508    166.68837    167    0    1.0123    100    1    508    0    0    0    0    0    0    0    0    0    0    0    0;
    35    650    210.661384    300    -100    1.0494    100    1    687    0    0    0    0    0    0    0    0    0    0    0    0;
    36    560    100.164792    240    0    1.0636    100    1    580    0    0    0    0    0    0    0    0    0    0    0    0;
    37    540    -1.36944739    250    0    1.0275    100    1    564    0    0    0    0    0    0    0    0    0    0    0    0;
    38    830    21.732729    300    -150    1.0265    100    1    865    0    0    0    0    0    0    0    0    0    0    0    0;
    39    1000    78.4673592    300    -100    1.03    100    1    1100    0    0    0    0    0    0    0    0    0    0    0    0;
];

%% branch data
%    fbus    tbus    r    x    b    rateA    rateB    rateC    ratio    angle    status    angmin    angmax    Pf    Qf    Pt    Qt
mpc.branch = [
    1    2    0.0035    0.0411    0.6987    600    600    600    0    0    1    -360    360    -173.7000    -40.3073    174.6777    -24.3579;
    1    39    0.001    0.025    0.75    1000    1000    1000    0    0    1    -360    360    76.1000    -3.8927    -76.0339    -74.7525;
    2    3    0.0013    0.0151    0.2572    500    500    500    0    0    1    -360    360    319.9146    88.5866    -318.5795    -100.8789;
    2    25    0.007    0.0086    0.146    500    500    500    0    0    1    -360    360    -244.5923    82.9736    248.9289    -93.8374;
    2    30    0    0.0181    0    900    900    2500    1.025    0    1    -360    360    -250.0000    -147.2022    250.0000    161.7616;
    3    4    0.0013    0.0213    0.2214    500    500    500    0    0    1    -360    360    37.3396    113.0645    -37.1319    -132.5902;
    3    18    0.0011    0.0133    0.2138    500    500    500    0    0    1    -360    360    -40.7601    -14.5856    40.7774    -7.9374;
    4    5    0.0008    0.0128    0.1342    600    600    600    0    0    1    -360    360    -197.4495    -4.0899    197.7587    -4.5240;
    4    14    0.0008    0.0129    0.1382    500    500    500    0    0    1    -360    360    -265.4186    -47.3199    265.9901    42.4821;
    5    6    0.0002    0.0026    0.0434    1200    1200    1200    0    0    1    -360    360    -536.9366    -43.1124    537.5096    46.1600;
    5    8    0.0008    0.0112    0.1476    900    900    900    0    0    1    -360    360    339.1779    47.6364    -338.2445    -49.3866;
    6    7    0.0006    0.0092    0.113    900    900    900    0    0    1    -360    360    453.8156    81.5487    -452.5550    -73.5949;
    6    11    0.0007    0.0082    0.1389    480    480    480    0    0    1    -360    360    -322.6541    -38.8547    323.3779    33.1423;
    6    31    0    0.025    0    1800    1800    1800    1.07    0    1    -360    360    -668.6711    -88.8539    668.6711    216.9745;
    7    8    0.0004    0.0046    0.078    900    900    900    0    0    1    -360    360    218.7550    -10.4051    -218.5628    4.8445;
    8    9    0.0023    0.0363    0.3804    900    900    900    0    0    1    -360    360    34.8073    -132.0579    -34.4838    97.7190;
    9    39    0.001    0.025    1.2    900    900    900    0    0    1    -360    360    27.9838    -31.1190    -27.9661    -96.7801;
    10    11    0.0004    0.0043    0.0729    600    600    600    0    0    1    -360    360    327.9013    73.3744    -327.4632    -76.1842;
    10    13    0.0004    0.0043    0.0729    600    600    600    0    0    1    -360    360    322.0987    37.4919    -321.6915    -40.6459;
    10    32    0    0.02    0    900    900    2500    1.07    0    1    -360    360    -650.0000    -110.8663    650.0000    206.9649;
    12    11    0.0016    0.0435    0    500    500    500    1.006    0    1    -360    360    -4.0562    -42.2501    4.0853    43.0419;
    12    13    0.0016    0.0435    0    500    500    500    1.006    0    1    -360    360    -4.4738    -45.7499    4.5080    46.6787;
    13    14    0.0009    0.0101    0.1723    600    600    600    0    0    1    -360    360    317.1835    -6.0327    -316.3044    -1.8046;
    14    15    0.0018    0.0217    0.366    600    600    600    0    0    1    -360    360    50.3143    -40.6775    -50.2614    3.6645;
    15    16    0.0009    0.0094    0.171    600    600    600    0    0    1    -360    360    -269.7386    -156.6645    270.5632    147.3331;
    16    17    0.0007    0.0089    0.1342    600    600    600    0    0    1    -360    360    224.0171    -42.5399    -223.6793    32.5030;
    16    19    0.0016    0.0195    0.304    600    600    2500    0    0    1    -360    360    -451.2985    -54.2032    454.3769    58.7545;
    16    21    0.0008    0.0135    0.2548    600    600    600    0    0    1    -360    360    -329.6017    14.4400    330.4228    -27.7427;
    16    24    0.0003    0.0059    0.068    600    600    600    0    0    1    -360    360    -42.6801    -97.3300    42.7099    90.6287;
    17    18    0.0007    0.0082    0.1319    600    600    600    0    0    1    -360    360    199.0388    11.0524    -198.7774    -22.0626;
    17    27    0.0013    0.0173    0.3216    600    600    600    0    0    1    -360    360    24.6405    -43.5555    -24.6247    9.2293;
    19    20    0.0007    0.0138    0    900    900    2500    1.06    0    1    -360    360    174.7289    -9.1736    -174.5105    13.4783;
    19    33    0.0007    0.0142    0    900    900    2500    1.07    0    1    -360    360    -629.1058    -49.5810    632.0000    108.2928;
    20    34    0.0009    0.018    0    900    900    2500    1.009    0    1    -360    360    -505.4895    -116.4783    508.0000    166.6884;
    21    22    0.0008    0.014    0.2565    900    900    900    0    0    1    -360    360    -604.4228    -87.2573    607.2059    108.1513;
    22    23    0.0006    0.0096    0.1846    600    600    600    0    0    1    -360    360    42.7941    41.8843    -42.7694    -61.7499;
    22    35    0    0.0143    0    900    900    2500    1.025    0    1    -360    360    -650.0000    -150.0356    650.0000    210.6614;
    23    24    0.0022    0.035    0.361    600    600    600    0    0    1    -360    360    353.8390    -0.5006    -351.3099    1.5713;
    23    36    0.0005    0.0272    0    900    900    2500    1    0    1    -360    360    -558.5696    -22.3495    560.0000    100.1648;
    25    26    0.0032    0.0323    0.531    600    600    600    0    0    1    -360    360    65.4139    -18.8109    -65.2881    -39.0350;
    25    37    0.0006    0.0232    0    900    900    2500    1.025    0    1    -360    360    -538.3428    65.4483    540.0000    -1.3694;
    26    27    0.0014    0.0147    0.2396    600    600    600    0    0    1    -360    360    257.2957    68.2052    -256.3753    -84.7293;
    26    28    0.0043    0.0474    0.7802    600    600    600    0    0    1    -360    360    -140.8192    -21.2096    141.6077    -56.3568;
    26    29    0.0057    0.0625    1.029    600    600    600    0    0    1    -360    360    -190.1884    -24.9606    192.1022    -67.7912;
    28    29    0.0014    0.0151    0.249    600    600    600    0    0    1    -360    360    -347.6077    28.7568    349.1639    -39.4372;
    29    38    0.0008    0.0156    0    1200    1200    2500    1.025    0    1    -360    360    -824.7661    80.3284    830.0000    21.7327;
];

%%-----  OPF Data  -----%%
%% generator cost data
%    1    startup    shutdown    n    x1    y1    ...    xn    yn
%    2    startup    shutdown    n    c(n-1)    ...    c0
mpc.gencost = [
    2    0    0    3    0.01    0.3    0.2;
    2    0    0    3    0.01    0.3    0.2;
    2    0    0    3    0.01    0.3    0.2;
    2    0    0    3    0.01    0.3    0.2;
    2    0    0    3    0.01    0.3    0.2;
    2    0    0    3    0.01    0.3    0.2;
    2    0    0    3    0.01    0.3    0.2;
    2    0    0    3    0.01    0.3    0.2;
    2    0    0    3    0.01    0.3    0.2;
    2    0    0    3    0.01    0.3    0.2;
];
