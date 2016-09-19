function mpc = case39_resqlim
%CASE39_RESQLIM

%% MATPOWER Case Format : Version 2
mpc.version = '2';

%%-----  Power Flow Data  -----%%
%% system MVA base
mpc.baseMVA = 100;

%% bus data
%	bus_i	type	Pd	Qd	Gs	Bs	area	Vm	Va	baseKV	zone	Vmax	Vmin
mpc.bus = [
	1	1	97.6	44.2	0	0	2	1.03943011	-13.5354369	345	1	1.06	0.94;
	2	1	0	0	0	0	2	1.04861423	-9.78479455	345	1	1.06	0.94;
	3	1	322	2.4	0	0	2	1.03078774	-12.2757641	345	1	1.06	0.94;
	4	1	500	184	0	0	1	1.00450252	-12.6261349	345	1	1.06	0.94;
	5	1	0	0	0	0	1	1.00603333	-11.1918233	345	1	1.06	0.94;
	6	1	0	0	0	0	1	1.00825014	-10.4078596	345	1	1.06	0.94;
	7	1	233.8	84	0	0	1	0.998421667	-12.7550031	345	1	1.06	0.94;
	8	1	522	176.6	0	0	1	0.997896447	-13.3351737	345	1	1.06	0.94;
	9	1	6.5	-66.6	0	0	1	1.03834191	-14.1774623	345	1	1.06	0.94;
	10	1	0	0	0	0	1	1.0178657	-8.17052575	345	1	1.06	0.94;
	11	1	0	0	0	0	1	1.01340918	-8.93657804	345	1	1.06	0.94;
	12	1	8.53	88	0	0	1	1.00084034	-8.99844626	345	1	1.06	0.94;
	13	1	0	0	0	0	1	1.01494891	-8.9295672	345	1	1.06	0.94;
	14	1	0	0	0	0	1	1.01235297	-10.7148915	345	1	1.06	0.94;
	15	1	320	153	0	0	3	1.01622184	-11.3451461	345	1	1.06	0.94;
	16	1	329	32.3	0	0	3	1.03255703	-10.033271	345	1	1.06	0.94;
	17	1	0	0	0	0	2	1.03429586	-11.1164337	345	1	1.06	0.94;
	18	1	158	30	0	0	2	1.0316401	-11.9858815	345	1	1.06	0.94;
	19	1	0	0	0	0	3	1.05012028	-5.41011228	345	1	1.06	0.94;
	20	1	680	103	0	0	3	0.991017923	-6.82117219	345	1	1.06	0.94;
	21	1	274	115	0	0	3	1.03234506	-7.62877633	345	1	1.06	0.94;
	22	1	0	0	0	0	3	1.0501566	-3.18328204	345	1	1.06	0.94;
	23	1	247.5	84.6	0	0	3	1.04515942	-3.38143399	345	1	1.06	0.94;
	24	1	308.6	-92.2	0	0	3	1.03803452	-9.91368049	345	1	1.06	0.94;
	25	1	224	47.2	0	0	2	1.05789595	-8.37286555	345	1	1.06	0.94;
	26	1	139	17	0	0	2	1.05266742	-9.44040946	345	1	1.06	0.94;
	27	1	281	75.5	0	0	2	1.03843076	-11.3628243	345	1	1.06	0.94;
	28	1	206	27.6	0	0	3	1.05042858	-5.93031085	345	1	1.06	0.94;
	29	1	283.5	26.9	0	0	3	1.05015258	-3.1719908	345	1	1.06	0.94;
	30	2	0	0	0	0	2	1.0499	-7.37027931	345	1	1.06	0.94;
	31	3	9.2	4.6	0	0	1	0.982	0	345	1	1.06	0.94;
	32	2	0	0	0	0	1	0.9841	-0.188266302	345	1	1.06	0.94;
	33	2	0	0	0	0	3	0.9972	-0.193244704	345	1	1.06	0.94;
	34	2	0	0	0	0	3	1.0123	-1.63113028	345	1	1.06	0.94;
	35	2	0	0	0	0	3	1.0494	1.77627908	345	1	1.06	0.94;
	36	2	0	0	0	0	3	1.0636	4.46818601	345	1	1.06	0.94;
	37	1	0	0	0	0	2	1.02802543	-1.59183395	345	1	1.06	0.94;
	38	2	0	0	0	0	3	1.0265	3.89055273	345	1	1.06	0.94;
	39	2	1104	250	0	0	1	1.03	-14.5340978	345	1	1.06	0.94;
];

%% generator data
%	bus	Pg	Qg	Qmax	Qmin	Vg	mBase	status	Pmax	Pmin	Pc1	Pc2	Qc1min	Qc1max	Qc2min	Qc2max	ramp_agc	ramp_10	ramp_30	ramp_q	apf
mpc.gen = [
	30	250	161.081306	400	140	1.0499	100	1	1040	0	0	0	0	0	0	0	0	0	0	0	0;
	31	677.857519	221.480303	300	-100	0.982	100	1	646	0	0	0	0	0	0	0	0	0	0	0	0;
	32	650	206.860029	300	150	0.9841	100	1	725	0	0	0	0	0	0	0	0	0	0	0	0;
	33	632	108.204067	250	0	0.9972	100	1	652	0	0	0	0	0	0	0	0	0	0	0	0;
	34	508	166.647258	167	0	1.0123	100	1	508	0	0	0	0	0	0	0	0	0	0	0	0;
	35	650	210.56179	300	-100	1.0494	100	1	687	0	0	0	0	0	0	0	0	0	0	0	0;
	36	560	100.108314	240	0	1.0636	100	1	580	0	0	0	0	0	0	0	0	0	0	0	0;
	37	540	0	250	0	1.0275	100	1	564	0	0	0	0	0	0	0	0	0	0	0	0;
	38	830	21.4905357	300	-150	1.0265	100	1	865	0	0	0	0	0	0	0	0	0	0	0	0;
	39	1000	78.2348932	300	-100	1.03	100	1	1100	0	0	0	0	0	0	0	0	0	0	0	0;
];

%% branch data
%	fbus	tbus	r	x	b	rateA	rateB	rateC	ratio	angle	status	angmin	angmax	Pf	Qf	Pt	Qt
mpc.branch = [
	1	2	0.0035	0.0411	0.6987	600	600	600	0	0	1	-360	360	-173.7116	-40.4982	174.6894	-24.1783;
	1	39	0.001	0.025	0.75	1000	1000	1000	0	0	1	-360	360	76.1116	-3.7018	-76.0454	-74.9435;
	2	3	0.0013	0.0151	0.2572	500	500	500	0	0	1	-360	360	319.9807	88.8706	-318.6448	-101.1580;
	2	25	0.007	0.0086	0.146	500	500	500	0	0	1	-360	360	-244.6701	81.8657	248.9954	-92.7485;
	2	30	0	0.0181	0	900	900	2500	1.025	0	1	-360	360	-250.0000	-146.5580	250.0000	161.0813;
	3	4	0.0013	0.0213	0.2214	500	500	500	0	0	1	-360	360	37.3565	113.2530	-37.1482	-132.7722;
	3	18	0.0011	0.0133	0.2138	500	500	500	0	0	1	-360	360	-40.7117	-14.4950	40.7290	-8.0319;
	4	5	0.0008	0.0128	0.1342	600	600	600	0	0	1	-360	360	-197.4441	-3.9700	197.7532	-4.6455;
	4	14	0.0008	0.0129	0.1382	500	500	500	0	0	1	-360	360	-265.4077	-47.2578	265.9791	42.4168;
	5	6	0.0002	0.0026	0.0434	1200	1200	1200	0	0	1	-360	360	-536.9261	-43.0187	537.4991	46.0652;
	5	8	0.0008	0.0112	0.1476	900	900	900	0	0	1	-360	360	339.1729	47.6642	-338.2396	-49.4159;
	6	7	0.0006	0.0092	0.113	900	900	900	0	0	1	-360	360	453.8088	81.5527	-452.5483	-73.6009;
	6	11	0.0007	0.0082	0.1389	480	480	480	0	0	1	-360	360	-322.6503	-38.8429	323.3741	33.1291;
	6	31	0	0.025	0	1800	1800	1800	1.07	0	1	-360	360	-668.6575	-88.7750	668.6575	216.8803;
	7	8	0.0004	0.0046	0.078	900	900	900	0	0	1	-360	360	218.7483	-10.3991	-218.5561	4.8379;
	8	9	0.0023	0.0363	0.3804	900	900	900	0	0	1	-360	360	34.7956	-132.0220	-34.4723	97.6782;
	9	39	0.001	0.025	1.2	900	900	900	0	0	1	-360	360	27.9723	-31.0782	-27.9546	-96.8216;
	10	11	0.0004	0.0043	0.0729	600	600	600	0	0	1	-360	360	327.8979	73.3573	-327.4599	-76.1679;
	10	13	0.0004	0.0043	0.0729	600	600	600	0	0	1	-360	360	322.1021	37.4131	-321.6950	-40.5679;
	10	32	0	0.02	0	900	900	2500	1.07	0	1	-360	360	-650.0000	-110.7705	650.0000	206.8600;
	12	11	0.0016	0.0435	0	500	500	500	1.006	0	1	-360	360	-4.0566	-42.2471	4.0857	43.0387;
	12	13	0.0016	0.0435	0	500	500	500	1.006	0	1	-360	360	-4.4734	-45.7529	4.5076	46.6817;
	13	14	0.0009	0.0101	0.1723	600	600	600	0	0	1	-360	360	317.1874	-6.1138	-316.3083	-1.7248;
	14	15	0.0018	0.0217	0.366	600	600	600	0	0	1	-360	360	50.3293	-40.6920	-50.2763	3.6768;
	15	16	0.0009	0.0094	0.171	600	600	600	0	0	1	-360	360	-269.7237	-156.6768	270.5482	147.3430;
	16	17	0.0007	0.0089	0.1342	600	600	600	0	0	1	-360	360	224.0331	-42.8039	-223.6953	32.7676;
	16	19	0.0016	0.0195	0.304	600	600	2500	0	0	1	-360	360	-451.2992	-54.0845	454.3772	58.6301;
	16	21	0.0008	0.0135	0.2548	600	600	600	0	0	1	-360	360	-329.6020	14.5222	330.4231	-27.8269;
	16	24	0.0003	0.0059	0.068	600	600	600	0	0	1	-360	360	-42.6801	-97.2767	42.7099	90.5743;
	17	18	0.0007	0.0082	0.1319	600	600	600	0	0	1	-360	360	198.9902	10.9541	-198.7290	-21.9681;
	17	27	0.0013	0.0173	0.3216	600	600	600	0	0	1	-360	360	24.7050	-43.7217	-24.6891	9.3926;
	19	20	0.0007	0.0138	0	900	900	2500	1.06	0	1	-360	360	174.7287	-9.1351	-174.5104	13.4396;
	19	33	0.0007	0.0142	0	900	900	2500	1.07	0	1	-360	360	-629.1059	-49.4950	632.0000	108.2041;
	20	34	0.0009	0.018	0	900	900	2500	1.009	0	1	-360	360	-505.4896	-116.4396	508.0000	166.6473;
	21	22	0.0008	0.014	0.2565	900	900	900	0	0	1	-360	360	-604.4231	-87.1731	607.2060	108.0620;
	22	23	0.0006	0.0096	0.1846	600	600	600	0	0	1	-360	360	42.7940	41.8794	-42.7693	-61.7457;
	22	35	0	0.0143	0	900	900	2500	1.025	0	1	-360	360	-650.0000	-149.9414	650.0000	210.5618;
	23	24	0.0022	0.035	0.361	600	600	600	0	0	1	-360	360	353.8389	-0.5586	-351.3099	1.6257;
	23	36	0.0005	0.0272	0	900	900	2500	1	0	1	-360	360	-558.5696	-22.2957	560.0000	100.1083;
	25	26	0.0032	0.0323	0.531	600	600	600	0	0	1	-360	360	65.3491	-18.4645	-65.2234	-39.4000;
	25	37	0.0006	0.0232	0	900	900	2500	1.025	0	1	-360	360	-538.3445	64.0129	540.0000	0.0000;
	26	27	0.0014	0.0147	0.2396	600	600	600	0	0	1	-360	360	257.2311	68.3608	-256.3109	-84.8926;
	26	28	0.0043	0.0474	0.7802	600	600	600	0	0	1	-360	360	-140.8181	-21.1043	141.6066	-56.4750;
	26	29	0.0057	0.0625	1.029	600	600	600	0	0	1	-360	360	-190.1896	-24.8565	192.1034	-67.9107;
	28	29	0.0014	0.0151	0.249	600	600	600	0	0	1	-360	360	-347.6066	28.8750	349.1627	-39.5584;
	29	38	0.0008	0.0156	0	1200	1200	2500	1.025	0	1	-360	360	-824.7662	80.5691	830.0000	21.4905;
];

%%-----  OPF Data  -----%%
%% generator cost data
%	1	startup	shutdown	n	x1	y1	...	xn	yn
%	2	startup	shutdown	n	c(n-1)	...	c0
mpc.gencost = [
	2	0	0	3	0.01	0.3	0.2;
	2	0	0	3	0.01	0.3	0.2;
	2	0	0	3	0.01	0.3	0.2;
	2	0	0	3	0.01	0.3	0.2;
	2	0	0	3	0.01	0.3	0.2;
	2	0	0	3	0.01	0.3	0.2;
	2	0	0	3	0.01	0.3	0.2;
	2	0	0	3	0.01	0.3	0.2;
	2	0	0	3	0.01	0.3	0.2;
	2	0	0	3	0.01	0.3	0.2;
];