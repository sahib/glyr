#!/usr/bin/env python
try:
    import glypy
except ImportError as e:
    print(False,'=>',e)
else:
    print(True,'=>',glypy.glyr_version())
