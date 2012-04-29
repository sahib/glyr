#!/usr/bin/env python
"""
setup.py file for glyr's python bindings
"""

from distutils.core import setup, Extension

glypy_module = Extension('_glypy', sources=['python_glyr_wrap.c'], libraries = ['glyr'])

setup(
        name = 'glypy',
        version = '0.1',
        author      = "sahib",
        description = """Python bindings to libglyr""",
        ext_modules = [glypy_module],
        py_modules = ["glypy"],
        )
