%module jpeg_manipulator
%{
#include "../interface/jpeg_manipulator.h"
%}

%include "../interface/jpeg_manipulator.h"
%extend JpegManipulator {
   int __getitem__(int i) { return (self->GetDataBuffer())[i]; }
   void __setitem__(int i, int v) { (self->GetDataBuffer())[i] = v; }
};
