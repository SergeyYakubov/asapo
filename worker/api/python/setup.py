from distutils.core import setup
from distutils.core import Extension

from Cython.Build import cythonize

ext_modules = cythonize([
    Extension("asapo_worker", ["asapo_worker.pyx"],
              extra_objects=['/home/yakubov/projects/asapo/cmake-build-debug/worker/api/cpp/libasapo-worker.a',
                             '/home/yakubov/opt/curl/lib/libcurl.a'],
              include_dirs=['../cpp/include',"/home/yakubov/projects/asapo/common/cpp/include"],
              extra_compile_args=['--std=c++11','--coverage','-fprofile-arcs','-ftest-coverage'],
              extra_link_args=['--coverage','-fprofile-arcs','-ftest-coverage'],
              language="c++")
])

setup(ext_modules = ext_modules)
