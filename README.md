Do `circle-lang <file-name>` to interpret a file.

## How to build and run this project

### Requirements
- CMake >= 3.28
- clang >= 18.1.2
- ninja >= 1.11

### Steps
- Insteall
  [vcpkg](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-bash)
  and add it to your `$PATH`.
- Choose your c/c++ compiler with environment variables `CC` and `CXX`. E.g.
  `export CXX=clang++-18 CC=clang-18`.
- run `sh make.sh`.
- run `sh run.sh <source-file-name>`.
- The binary can be found in `./build/circle-lang`.

## Add dependencies
- Modify vcpkg.json
- run `sh make.sh`
- Modify CmakeLists.txt
