import distutils.dir_util
import os
import platform
import sys
from codecs import open
from distutils.command.clean import clean as _clean
from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext as _build_ext
from setuptools.command.sdist import sdist as _sdist
from setuptools.command.test import test as _test
from setuptools.command.install_lib import install_lib as _install_lib
from shutil import copy, rmtree

system = platform.system()
version_info = sys.version_info
here = os.path.abspath(os.path.dirname(__file__))

with open('README.md', 'r') as fh:
    long_description = fh.read()

class CMakeExtension(Extension):
    def __init__(self, name):
        # don't invoke the original build_ext for this special extension
        Extension.__init__(self, name, sources=[])

class BuildRLClientPythonBindings(_build_ext):

    def run(self):
        for ext in self.extensions:
            self.build_cmake(ext)
        _build_ext.run(self)

    def build_cmake(self, ext):
        # Make build directory
        distutils.dir_util.mkpath(self.build_temp)

        # Ensure lib output directory is made
        lib_output_dir = os.path.join(here, os.path.dirname(self.get_ext_fullpath(ext.name)), ext.name)
        distutils.dir_util.mkpath(lib_output_dir)

        # example of cmake args
        config = 'Debug' if self.debug else 'Release'
        cmake_args = [
            '-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + str(lib_output_dir),
            '-DCMAKE_BUILD_TYPE=' + config,
            '-DRL_PY_VERSION=' + '{v[0]}.{v[1]}'.format(v=version_info),
        ]

        # example of build args
        build_args = [
            '--config', config,
            '--', '-j8',
            # Build the rl_client target
            "_rl_client"
        ]

        cmake_directory = os.path.join(here, '..', '..')
        os.chdir(str(self.build_temp))
        self.spawn(['cmake', str(cmake_directory)] + cmake_args)
        if not self.dry_run:
            self.spawn(['cmake', '--build', '.'] + build_args)
        os.chdir(str(here))

setup(
    version = '0.1.1',
    name = 'rl_client',
    url = 'https://github.com/JohnLangford/vowpal_wabbit',
    description = 'Python binding for reinforcement learning client library',
    long_description = long_description,
    author = 'Microsoft Corporation',
    author_email = 'decisionservicedevs@microsoft.com',
    license = 'MIT',
    ext_modules = [CMakeExtension('rl_client')],
    py_modules = ['rl_client.py'],
    packages = find_packages(),
    classifiers = (
        'License :: OSI Approved :: MIT License',
        'Programming Language :: Python :: 3.6',
    ),
    cmdclass={
        'build_ext': BuildRLClientPythonBindings,
    },
)
