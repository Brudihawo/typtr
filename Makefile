SHELL:=/usr/bin/bash

debug_config:
	mkdir -p build
	cmake -G Ninja -B build -S src -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=true
	touch .is_debug
	if [ -f '.is_release' ]; then rm -v .is_release ; fi

release_config:
	mkdir -p build
	cmake -G Ninja -B build -S src -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=true
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
