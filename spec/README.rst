Testsuit for libglyr
====================

The testsuite has been rewritten to use Python + plyr,
which saves me quite some work. 

*There are to kind of tests:*
  
  - ```capi```: Tests for the C level API. (unittests)
  - ```provider```: Test all providers (tests known success/error input)

Running
-------

capi:

  Compile with ::

    cmake . -DTEST=true
    make
    ./bin/check_*

provider: 

  :: 

    cd spec/providers
    python runner.py
