"""
mantid.geometry
===============

Defines Python objects that wrap the C++ Geometry namespace.

"""

###############################################################################
# The _api C extension depends on exports defined in the _kernel extension
###############################################################################
from mantid.kernel import dlopen as _pydlopen
import os as _os
clib = _os.path.join(_os.path.dirname(__file__), '_geometry.so')
flags = _pydlopen.setup_dlopen(clib, ['libMantidKernel', 'libMantidGeometry']) # Ensure the library is open with the correct flags
from _geometry import *
_pydlopen.restore_flags(flags)
