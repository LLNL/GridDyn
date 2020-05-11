Bus.con = [ ...
  1  400  1  0  1  1;
  2  400  1  0  2  1;
  3  400  1  0  3  1;
 ];

Line.con = [ ...
  1  2  100  400  60  0  0  0  0.1  0  0  0  0.4  0.4  0  1;
  1  3  100  400  60  0  0  0  0.1  0  0  0  0.4  0.4  0  1;
  2  3  100  400  60  0  0  0  0.1  0  0  0  0.4  0.4  0  1;
 ];

SW.con = [ ...
  1  100  400  1  0  1.5  -1.5  1.1  0.9  0.4  1  1  1;
 ];

PV.con = [ ...
  2  100  400  0.4  1  0.8  -0.2  1.1  0.9  1  1;
  3  100  400  0.4  1  0.8  -0.2  1.1  0.9  1  1;
 ];

PQ.con = [ ...
  3  100  400  1  0.6  1.2  0.8  1  1;
 ];

Demand.con = [ ...
  3  100  1  0.6  1  1  0  0  0  0  0  0  0  0  0  0  0  1;
 ];

Supply.con = [ ...
  1  100  0  0.6  0.1  0  6  9.8  0.1  0  0  0  0  0  1  1.5  -1.5  0  0  1;
  2  100  0  0.6  0.1  0  4  10.7  0.2  0  0  0  0  0  1  0.8  -0.2  0  0  1;
  3  100  0  0.6  0.1  0  8  12.6  0.25  0  0  0  0  0  1  0.8  -0.2  0  0  1;
 ];

Rmpg.con = [ ...
  2  100  0.1  0.1  2  2  5  0  1  1;
  1  100  0.05  0.05  2  2  5  0  1  1;
  3  100  0.15  0.15  2  2  0  5  1  1;
 ];

Ypdp.con = [ ...
  55  75  100  120  100;
 ];

Bus.names = {...
  'Bus1'; 'Bus2'; 'Bus3'};
