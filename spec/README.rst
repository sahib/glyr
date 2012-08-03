Testsuite for libglyr
=====================

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
   
  Before running, you need to install plyr: http://sahib.github.com/python-glyr/intro.html

  :: 

    cd spec/providers
    python runner.py

  ```runner.py``` supports a ```--only```argument, which can be used for exclusion: ::

    python runner.py --only cover|lyrics                          # Run all tests for cover and lyrics
    python runner.py --only cover:google,lastfm|lyrics:lyricswiki # Run the covertests google and lastfm, also lyricswiki.
