#!/bin/bash
while true; do
	cmake --build build --target all-tests
	exitcode=$?
	RED='\033[0;31m'
	NC='\033[0m'
	if [ $exitcode -ne 0 ]; then
		echo -e "${RED}Compilation failed${NC}"
	else
		# cmake --build build --target test
		timeout 2 ./build/all-tests
		exitcode=$?
		if [ $exitcode -eq 124 ]; then
			echo -e "${RED}Timed out${NC}"
		fi
	fi

	inotifywait -e modify,create,delete -r ./
done
