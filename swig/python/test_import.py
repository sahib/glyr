#!/usr/bin/env python

success = True

try:
    import glypy
except ImportError as e:
    print(e)
    success = False
finally:
    print('=>',success)
    

