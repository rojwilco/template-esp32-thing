BUILD_DIR := build

.PHONY: all build test clean

all: test

build:
	@mkdir -p $(BUILD_DIR)
	cmake -S . -B $(BUILD_DIR) -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug
	cmake --build $(BUILD_DIR) -- -j$(shell sysctl -n hw.logicalcpu 2>/dev/null || echo 4)

test: build
	cd $(BUILD_DIR) && ctest --output-on-failure

clean:
	rm -rf $(BUILD_DIR)
