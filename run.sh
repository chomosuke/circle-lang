#!/bin/bash
cmake --build build
exitcode=$?
RED='\033[0;31m'
NC='\033[0m'
if [ $exitcode -ne 0 ]; then
	echo -e "${RED}compilation failed${NC}"
	exit 1
fi
./build/circle-lang "$@"
