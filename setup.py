from setuptools import setup, Extension
from pybind11.setup_helpers import Pybind11Extension, build_ext
from pybind11 import get_cmake_dir
import pybind11
import os
import sys

__version__ = "1.0.0"

# Define the extension module
ext_modules = [
    Pybind11Extension(
        "docparser",
        [
            "python_bindings.cpp",
        ],
        include_dirs=[
            # Path to pybind11 headers
            pybind11.get_include(),
            # Add current directory for our headers
            ".",
        ],
        language='c++',
        cxx_std=17,  # Use C++17 standard
        define_macros=[
            ("VERSION_INFO", '"{}"'.format(__version__)),
        ],
    ),
]

setup(
    name="docparser",
    version=__version__,
    author="Document Parser Library",
    author_email="ashrafgalibshaik@gmail.com",
    url="https://github.com/AshrafGalibShaik/Universal-Document-Parser-Library",
    description="Universal document parser library for Python",
    long_description="""
    A high-performance C++ library with Python bindings for parsing various document formats.
    
    Supported formats:
    - Plain text (.txt)
    - CSV files (.csv)
    - JSON files (.json)
    - XML/HTML files (.xml, .html, .htm)
    - Markdown files (.md, .markdown)
    
    Features:
    - Fast C++ parsing engine
    - Memory efficient
    - Extensible architecture
    - Comprehensive metadata extraction
    - Easy-to-use Python API
    """,
    long_description_content_type="text/markdown",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    python_requires=">=3.6",
    install_requires=[
        "pybind11>=2.6.0",
    ],
    extras_require={
        "dev": [
            "pytest",
            "pytest-cov",
            "black",
            "flake8",
        ],
    },
    classifiers=[
        "Development Status :: 4 - Beta",
        "Intended Audience :: Developers",
        "License :: OSI Approved :: MIT License",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: C++",
        "Topic :: Software Development :: Libraries :: Python Modules",
        "Topic :: Text Processing",
        "Topic :: Text Processing :: Markup",
    ],
)