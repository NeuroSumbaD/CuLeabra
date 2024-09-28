from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
import subprocess

class CustomBuildExt(build_ext):
    def run(self):
        # Call the Makefile
        subprocess.check_call(['make'])
        # Optionally, you can also add additional checks here

setup(
    name='culeabra',
    version='0.1',
    packages=['culeabra'],
    ext_modules=[Extension('culeabra', sources=[])],  # Empty sources, handled by Makefile
    cmdclass={'build_ext': CustomBuildExt},
)