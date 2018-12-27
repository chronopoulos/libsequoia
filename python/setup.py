from distutils.core import setup, Extension

# the c extension module
extension_mod = Extension("sequoia", ["sequoia.c"])

setup(name="sequoia", ext_modules=[extension_mod])
