from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

ext_modules = [
    Extension( "US100",
               sources=["US100wrapper.pyx", 'US100.c'],
               include_dirs = ["/opt/vc/include"],
               libraries = [ "bcm_host" ],
               library_dirs = ["/opt/vc/lib"]
    )
]


    
setup(
    name = "US100",
    ext_modules = cythonize( ext_modules )
)
