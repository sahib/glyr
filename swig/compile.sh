#!/bin/sh
(  swig -ruby glyr.i &&
   gcc -c glyr_wrap.c -I /usr/include/ruby-1.9.1 -I/usr/include/ruby-1.9.1/x86_64-linux -I. -fpic &&
   gcc -shared `find ../lib/CMakeFiles -name "*.o"` glyr_wrap.o -o glyr.so -lcurl &&
   ruby isModuleWorking.rb "./glyr" &&
   echo '- done') || 
   echo '- failed'
