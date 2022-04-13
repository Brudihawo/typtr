SHELL:=/usr/bin/bash

CMAKE_FLAGS += -DCMAKE_EXPORT_COMPILE_COMMANDS=true -DCMAKE_C_COMPILER=clang

debug_config:
	mkdir -p build
	cmake -DCMAKE_BUILD_TYPE=Debug -G Ninja -B build -S src $(CMAKE_FLAGS)
	touch .is_debug
	if [ -f '.is_release' ]; then rm -v .is_release ; fi

release_config:
	mkdir -p build
	CC=clang cmake -DCMAKE_BUILD_TYPE=Release -G Ninja -B build -S src $(CMAKE_FLAGS)
	touch .is_release
	if [ -f '.is_debug' ]; then rm -v .is_debug ; fi


debug: debug_config
	cmake --build build

release: release_config
	cmake --build build

clean:
	rm -frv build/*
	rm -v .is_debug
	rm -v .is_release
