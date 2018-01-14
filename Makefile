DEBUG_BUILD_DIR := 'cmake-build-debug'
RELEASE_BUILD_DIR := 'cmake-build-release'

.PHONY: phony
phony: ;

# FIXME: All, python version
.PHONY: init
init:
	git submodule init
	git submodule update
	mkdir -p src/arrow/cpp/build
	cd src/arrow/cpp/build && \
		cmake \
			-DARROW_BUILD_TESTS=OFF \
			-DARROW_PYTHON=ON \
			-DPYTHON_EXECUTABLE=python3 \
			.. && \
		make
	cd src/cpptoml && \
		git apply ../../patch/cpptoml
	cd src/pybind11 && \
		git apply ../../patch/pybind11

.PHONY: debug
debug:
	mkdir -p ${DEBUG_BUILD_DIR}
	cd ${DEBUG_BUILD_DIR} && \
		export CXX=clang++ && \
		cmake -DCMAKE_BUILD_TYPE=Debug .. && \
		make -j 2

.PHONY: release
release:
	mkdir -p ${RELEASE_BUILD_DIR}
	cd ${RELEASE_BUILD_DIR} && \
		export CXX=clang++ && \
		cmake -DCMAKE_BUILD_TYPE=Release .. && \
		make -j 2

.PHONY: all
all: release debug

.PHONY: scan-debug
scan-debug:
	mkdir -p ${DEBUG_BUILD_DIR}
	cd ${DEBUG_BUILD_DIR} && \
		export CXX=clang++ && \
		scan-build cmake -DCMAKE_BUILD_TYPE=Debug .. && \
		scan-build make

.PHONY: scan-release
scan-release:
	mkdir -p ${RELEASE_BUILD_DIR}
	cd ${RELEASE_BUILD_DIR} && \
		export CXX=clang++ && \
		scan-build cmake -DCMAKE_BUILD_TYPE=Release .. && \
		scan-build make

.PHONY: format
format:
	zsh -c 'clang-format -i -style=LLVM src/amanogawa/**/*.(h|cpp)'

.PHONY: clean-debug
clean-debug:
	rm -rf ${DEBUG_BUILD_DIR}

.PHONY: clean-release
clean-release:
	rm -rf ${RELEASE_BUILD_DIR}

.PHONY: clean
clean: clean-debug clean-release
