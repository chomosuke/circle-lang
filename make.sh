#!/bin/bash
cmake --preset=release
cmake --preset=debug
cmake --build build --target circle-lang
cmake --build build-debug --target all-tests
# --trace-source=CMakeLists.txt --trace-expand
