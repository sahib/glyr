/* File : example.i */
%module glyr

%{
#include "../lib/glyr.h"
%}

/* Let's just grab the original header file here */
%include "../lib/glyr.h"
