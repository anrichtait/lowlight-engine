from setuptools import setup, Extension
import sys, os

major_minor = (sys.version_info.major, sys.version_info.minor)

# Check for supported Python versions
if major_minor < (3, 8) or major_minor > (3, 12):
    raise RuntimeError("This package supports Python 3.8 to 3.12.")

package_version = '0.' + (str(os.environ['RUN_NUM']) if 'RUN_NUM' in os.environ else '1')

setup(
    name='pyg3d',
    version=package_version,
    author='research-efficiency',
    author_email='research-efficiency@roblox.com',
    description='Python bindings for G3D using pybind11',
    long_description=open('README.md').read(),
    long_description_content_type='text/markdown',
    url='https://github.com/Roblox/g3d-rbx',
    packages=['pyg3d'],
    package_data={'': ['*.so']},  # Include all .so files
    python_requires='>=3.8, <=3.12',  # Specify Python version range
    zip_safe=False,
)
