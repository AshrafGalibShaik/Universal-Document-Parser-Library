from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension, build_ext

ext_modules = [
    Pybind11Extension(
        "docparser",
        [
            "docparser/src/python_bindings.cpp",
        ],
        include_dirs=[
            "docparser/src",
        ],
        language='c++',
        cxx_std=17,
    ),
]

setup(
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
)