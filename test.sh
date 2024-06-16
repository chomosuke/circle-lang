#!/bin/bash
while true; do
	cmake --build build --target all_tests
	exitcode=$?
	RED='\033[0;31m'
	NC='\033[0m'
	if [ $exitcode -ne 0 ]; then
		echo -e "${RED}compilation failed${NC}"
	else
		# cmake --build build --target test
		./build/all_tests
	fi

	inotifywait -e modify,create,delete -r ./lib/ ./test/
done
