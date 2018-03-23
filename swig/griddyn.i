#define __attribute__(x)
#define GRIDDYN_Export __attribute__ ((visibility ("default")))

%module griddyn

%include cpointer.i
%pointer_functions(double, doublep);
%pointer_functions(char, charp);

%include carrays.i
%array_class(double, doubleArray);
%array_class(char, charArray);
int gridDynSimulation_getResults(gridDynSimReference sim, const char *datatype, double data[0], int maxSize);
int gridDynObject_getString (const gridDynObject obj, const char *parameter, char *value, int N);

%{
#include "griddyn_export.h"
#include "griddyn_export_advanced.h"
%}

%include "griddyn_export.h"
%include "griddyn_export_advanced.h"



