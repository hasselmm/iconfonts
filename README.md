Icon Fonts for Qt
=================

This library provides easy access to a variety of icon fonts.

TBD:

- More words about icon fonts, and why they are great,
  even if the web has moved to SVG.
- Code examples, screenshots.
- Contributing, adding new fonts.

Compiling, Building
-------------------

You need [Qt 6.5](https://www.qt.io/download-open-source) 
and [CMake](https://cmake.org/) to build this project. Additionally 
[Python](https://python.org) and some specific Python modules are is 
needed to generate code for fonts with more complex icon definitions.

Easiest way is to just open [CMakeLists.txt](CMakeLists.txt) in in QtCreator and to give it a go. 
Alternatively try something like this:

```bash
mkdir build && cd build
export Qt6_DIR=/path/to/your/qt6/lib/cmake/Qt6
cmake ..
cmake --build .
```

The optional Python dependencies are:

- [fontTools](https://pypi.org/project/fonttools/), and 
  [brotli](https://pypi.org/project/Brotli/) to convert some webfonts
  into desktop font formats.
- [pylint](https://pypi.org/project/pylint/), and 
  [mypy](https://pypi.org/project/mypy/) to validate the Python code.
  
Install them like this:

```bash
pip install brotli fonttools mypy pylint
```

License
-------

The code is offered under the terms of the [MIT License](LICENSE).
