#!/bin/bash
cmake --preset=default
# --trace-source=CMakeLists.txt --trace-expand
mv ./build/compile_commands.json .
