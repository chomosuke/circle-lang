# Circle Lang: The perfect programming language

- Do `circle-lang <file-name>` to interpret a file.
- Do `circle-lang <file-name> --debug` to debug a circle lang program.
- Do `circle-lang <file-name> --from-bf` to transpile a Brainfuck program into a
circle lang program.

## How to build and run this project

### Requirements
- CMake >= 3.28
- clang >= 18.1.2
- ninja >= 1.11
- [vcpkg](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-basj)

### Steps
- Choose your c/c++ compiler with environment variables `CC` and `CXX`. E.g.
  `export CXX=clang++-18 CC=clang-18`.
- run `sh make.sh`.
- run `sh run.sh <source-file-name>`.
- The binary can be found in `./build/circle-lang`.

## Add dependencies
- Modify vcpkg.json
- run `sh make.sh`
- Modify CmakeLists.txt

## Sample programs
- [sample-program/hello-world.crcl](sample-program/hello-world.crcl)
- [sample-program/postfix-calculator.crcl](sample-program/postfix-calculator.crcl)

## Working with Circle Lang
Check out [documentation.md](documentation.md) and [guide.md](guide.md).
