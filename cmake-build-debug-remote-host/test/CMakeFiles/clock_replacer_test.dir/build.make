# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /tmp/tmp.IyJU8EsJ7j

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /tmp/tmp.IyJU8EsJ7j/cmake-build-debug-remote-host

# Include any dependencies generated for this target.
include test/CMakeFiles/clock_replacer_test.dir/depend.make

# Include the progress variables for this target.
include test/CMakeFiles/clock_replacer_test.dir/progress.make

# Include the compile flags for this target's objects.
include test/CMakeFiles/clock_replacer_test.dir/flags.make

test/CMakeFiles/clock_replacer_test.dir/buffer/clock_replacer_test.cpp.o: test/CMakeFiles/clock_replacer_test.dir/flags.make
test/CMakeFiles/clock_replacer_test.dir/buffer/clock_replacer_test.cpp.o: ../test/buffer/clock_replacer_test.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/tmp/tmp.IyJU8EsJ7j/cmake-build-debug-remote-host/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object test/CMakeFiles/clock_replacer_test.dir/buffer/clock_replacer_test.cpp.o"
	cd /tmp/tmp.IyJU8EsJ7j/cmake-build-debug-remote-host/test && /bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/clock_replacer_test.dir/buffer/clock_replacer_test.cpp.o -c /tmp/tmp.IyJU8EsJ7j/test/buffer/clock_replacer_test.cpp

test/CMakeFiles/clock_replacer_test.dir/buffer/clock_replacer_test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/clock_replacer_test.dir/buffer/clock_replacer_test.cpp.i"
	cd /tmp/tmp.IyJU8EsJ7j/cmake-build-debug-remote-host/test && /bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /tmp/tmp.IyJU8EsJ7j/test/buffer/clock_replacer_test.cpp > CMakeFiles/clock_replacer_test.dir/buffer/clock_replacer_test.cpp.i

test/CMakeFiles/clock_replacer_test.dir/buffer/clock_replacer_test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/clock_replacer_test.dir/buffer/clock_replacer_test.cpp.s"
	cd /tmp/tmp.IyJU8EsJ7j/cmake-build-debug-remote-host/test && /bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /tmp/tmp.IyJU8EsJ7j/test/buffer/clock_replacer_test.cpp -o CMakeFiles/clock_replacer_test.dir/buffer/clock_replacer_test.cpp.s

test/CMakeFiles/clock_replacer_test.dir/buffer/clock_replacer_test.cpp.o.requires:

.PHONY : test/CMakeFiles/clock_replacer_test.dir/buffer/clock_replacer_test.cpp.o.requires

test/CMakeFiles/clock_replacer_test.dir/buffer/clock_replacer_test.cpp.o.provides: test/CMakeFiles/clock_replacer_test.dir/buffer/clock_replacer_test.cpp.o.requires
	$(MAKE) -f test/CMakeFiles/clock_replacer_test.dir/build.make test/CMakeFiles/clock_replacer_test.dir/buffer/clock_replacer_test.cpp.o.provides.build
.PHONY : test/CMakeFiles/clock_replacer_test.dir/buffer/clock_replacer_test.cpp.o.provides

test/CMakeFiles/clock_replacer_test.dir/buffer/clock_replacer_test.cpp.o.provides.build: test/CMakeFiles/clock_replacer_test.dir/buffer/clock_replacer_test.cpp.o


test/CMakeFiles/clock_replacer_test.dir/buffer/grading_buffer_pool_manager_instance_test.cpp.o: test/CMakeFiles/clock_replacer_test.dir/flags.make
test/CMakeFiles/clock_replacer_test.dir/buffer/grading_buffer_pool_manager_instance_test.cpp.o: ../test/buffer/grading_buffer_pool_manager_instance_test.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/tmp/tmp.IyJU8EsJ7j/cmake-build-debug-remote-host/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object test/CMakeFiles/clock_replacer_test.dir/buffer/grading_buffer_pool_manager_instance_test.cpp.o"
	cd /tmp/tmp.IyJU8EsJ7j/cmake-build-debug-remote-host/test && /bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/clock_replacer_test.dir/buffer/grading_buffer_pool_manager_instance_test.cpp.o -c /tmp/tmp.IyJU8EsJ7j/test/buffer/grading_buffer_pool_manager_instance_test.cpp

test/CMakeFiles/clock_replacer_test.dir/buffer/grading_buffer_pool_manager_instance_test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/clock_replacer_test.dir/buffer/grading_buffer_pool_manager_instance_test.cpp.i"
	cd /tmp/tmp.IyJU8EsJ7j/cmake-build-debug-remote-host/test && /bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /tmp/tmp.IyJU8EsJ7j/test/buffer/grading_buffer_pool_manager_instance_test.cpp > CMakeFiles/clock_replacer_test.dir/buffer/grading_buffer_pool_manager_instance_test.cpp.i

test/CMakeFiles/clock_replacer_test.dir/buffer/grading_buffer_pool_manager_instance_test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/clock_replacer_test.dir/buffer/grading_buffer_pool_manager_instance_test.cpp.s"
	cd /tmp/tmp.IyJU8EsJ7j/cmake-build-debug-remote-host/test && /bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /tmp/tmp.IyJU8EsJ7j/test/buffer/grading_buffer_pool_manager_instance_test.cpp -o CMakeFiles/clock_replacer_test.dir/buffer/grading_buffer_pool_manager_instance_test.cpp.s

test/CMakeFiles/clock_replacer_test.dir/buffer/grading_buffer_pool_manager_instance_test.cpp.o.requires:

.PHONY : test/CMakeFiles/clock_replacer_test.dir/buffer/grading_buffer_pool_manager_instance_test.cpp.o.requires

test/CMakeFiles/clock_replacer_test.dir/buffer/grading_buffer_pool_manager_instance_test.cpp.o.provides: test/CMakeFiles/clock_replacer_test.dir/buffer/grading_buffer_pool_manager_instance_test.cpp.o.requires
	$(MAKE) -f test/CMakeFiles/clock_replacer_test.dir/build.make test/CMakeFiles/clock_replacer_test.dir/buffer/grading_buffer_pool_manager_instance_test.cpp.o.provides.build
.PHONY : test/CMakeFiles/clock_replacer_test.dir/buffer/grading_buffer_pool_manager_instance_test.cpp.o.provides

test/CMakeFiles/clock_replacer_test.dir/buffer/grading_buffer_pool_manager_instance_test.cpp.o.provides.build: test/CMakeFiles/clock_replacer_test.dir/buffer/grading_buffer_pool_manager_instance_test.cpp.o


test/CMakeFiles/clock_replacer_test.dir/buffer/grading_lru_replacer_test.cpp.o: test/CMakeFiles/clock_replacer_test.dir/flags.make
test/CMakeFiles/clock_replacer_test.dir/buffer/grading_lru_replacer_test.cpp.o: ../test/buffer/grading_lru_replacer_test.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/tmp/tmp.IyJU8EsJ7j/cmake-build-debug-remote-host/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object test/CMakeFiles/clock_replacer_test.dir/buffer/grading_lru_replacer_test.cpp.o"
	cd /tmp/tmp.IyJU8EsJ7j/cmake-build-debug-remote-host/test && /bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/clock_replacer_test.dir/buffer/grading_lru_replacer_test.cpp.o -c /tmp/tmp.IyJU8EsJ7j/test/buffer/grading_lru_replacer_test.cpp

test/CMakeFiles/clock_replacer_test.dir/buffer/grading_lru_replacer_test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/clock_replacer_test.dir/buffer/grading_lru_replacer_test.cpp.i"
	cd /tmp/tmp.IyJU8EsJ7j/cmake-build-debug-remote-host/test && /bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /tmp/tmp.IyJU8EsJ7j/test/buffer/grading_lru_replacer_test.cpp > CMakeFiles/clock_replacer_test.dir/buffer/grading_lru_replacer_test.cpp.i

test/CMakeFiles/clock_replacer_test.dir/buffer/grading_lru_replacer_test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/clock_replacer_test.dir/buffer/grading_lru_replacer_test.cpp.s"
	cd /tmp/tmp.IyJU8EsJ7j/cmake-build-debug-remote-host/test && /bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /tmp/tmp.IyJU8EsJ7j/test/buffer/grading_lru_replacer_test.cpp -o CMakeFiles/clock_replacer_test.dir/buffer/grading_lru_replacer_test.cpp.s

test/CMakeFiles/clock_replacer_test.dir/buffer/grading_lru_replacer_test.cpp.o.requires:

.PHONY : test/CMakeFiles/clock_replacer_test.dir/buffer/grading_lru_replacer_test.cpp.o.requires

test/CMakeFiles/clock_replacer_test.dir/buffer/grading_lru_replacer_test.cpp.o.provides: test/CMakeFiles/clock_replacer_test.dir/buffer/grading_lru_replacer_test.cpp.o.requires
	$(MAKE) -f test/CMakeFiles/clock_replacer_test.dir/build.make test/CMakeFiles/clock_replacer_test.dir/buffer/grading_lru_replacer_test.cpp.o.provides.build
.PHONY : test/CMakeFiles/clock_replacer_test.dir/buffer/grading_lru_replacer_test.cpp.o.provides

test/CMakeFiles/clock_replacer_test.dir/buffer/grading_lru_replacer_test.cpp.o.provides.build: test/CMakeFiles/clock_replacer_test.dir/buffer/grading_lru_replacer_test.cpp.o


test/CMakeFiles/clock_replacer_test.dir/buffer/grading_parallel_buffer_pool_manager_test.cpp.o: test/CMakeFiles/clock_replacer_test.dir/flags.make
test/CMakeFiles/clock_replacer_test.dir/buffer/grading_parallel_buffer_pool_manager_test.cpp.o: ../test/buffer/grading_parallel_buffer_pool_manager_test.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/tmp/tmp.IyJU8EsJ7j/cmake-build-debug-remote-host/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object test/CMakeFiles/clock_replacer_test.dir/buffer/grading_parallel_buffer_pool_manager_test.cpp.o"
	cd /tmp/tmp.IyJU8EsJ7j/cmake-build-debug-remote-host/test && /bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/clock_replacer_test.dir/buffer/grading_parallel_buffer_pool_manager_test.cpp.o -c /tmp/tmp.IyJU8EsJ7j/test/buffer/grading_parallel_buffer_pool_manager_test.cpp

test/CMakeFiles/clock_replacer_test.dir/buffer/grading_parallel_buffer_pool_manager_test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/clock_replacer_test.dir/buffer/grading_parallel_buffer_pool_manager_test.cpp.i"
	cd /tmp/tmp.IyJU8EsJ7j/cmake-build-debug-remote-host/test && /bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /tmp/tmp.IyJU8EsJ7j/test/buffer/grading_parallel_buffer_pool_manager_test.cpp > CMakeFiles/clock_replacer_test.dir/buffer/grading_parallel_buffer_pool_manager_test.cpp.i

test/CMakeFiles/clock_replacer_test.dir/buffer/grading_parallel_buffer_pool_manager_test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/clock_replacer_test.dir/buffer/grading_parallel_buffer_pool_manager_test.cpp.s"
	cd /tmp/tmp.IyJU8EsJ7j/cmake-build-debug-remote-host/test && /bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /tmp/tmp.IyJU8EsJ7j/test/buffer/grading_parallel_buffer_pool_manager_test.cpp -o CMakeFiles/clock_replacer_test.dir/buffer/grading_parallel_buffer_pool_manager_test.cpp.s

test/CMakeFiles/clock_replacer_test.dir/buffer/grading_parallel_buffer_pool_manager_test.cpp.o.requires:

.PHONY : test/CMakeFiles/clock_replacer_test.dir/buffer/grading_parallel_buffer_pool_manager_test.cpp.o.requires

test/CMakeFiles/clock_replacer_test.dir/buffer/grading_parallel_buffer_pool_manager_test.cpp.o.provides: test/CMakeFiles/clock_replacer_test.dir/buffer/grading_parallel_buffer_pool_manager_test.cpp.o.requires
	$(MAKE) -f test/CMakeFiles/clock_replacer_test.dir/build.make test/CMakeFiles/clock_replacer_test.dir/buffer/grading_parallel_buffer_pool_manager_test.cpp.o.provides.build
.PHONY : test/CMakeFiles/clock_replacer_test.dir/buffer/grading_parallel_buffer_pool_manager_test.cpp.o.provides

test/CMakeFiles/clock_replacer_test.dir/buffer/grading_parallel_buffer_pool_manager_test.cpp.o.provides.build: test/CMakeFiles/clock_replacer_test.dir/buffer/grading_parallel_buffer_pool_manager_test.cpp.o


# Object files for target clock_replacer_test
clock_replacer_test_OBJECTS = \
"CMakeFiles/clock_replacer_test.dir/buffer/clock_replacer_test.cpp.o" \
"CMakeFiles/clock_replacer_test.dir/buffer/grading_buffer_pool_manager_instance_test.cpp.o" \
"CMakeFiles/clock_replacer_test.dir/buffer/grading_lru_replacer_test.cpp.o" \
"CMakeFiles/clock_replacer_test.dir/buffer/grading_parallel_buffer_pool_manager_test.cpp.o"

# External object files for target clock_replacer_test
clock_replacer_test_EXTERNAL_OBJECTS =

test/clock_replacer_test: test/CMakeFiles/clock_replacer_test.dir/buffer/clock_replacer_test.cpp.o
test/clock_replacer_test: test/CMakeFiles/clock_replacer_test.dir/buffer/grading_buffer_pool_manager_instance_test.cpp.o
test/clock_replacer_test: test/CMakeFiles/clock_replacer_test.dir/buffer/grading_lru_replacer_test.cpp.o
test/clock_replacer_test: test/CMakeFiles/clock_replacer_test.dir/buffer/grading_parallel_buffer_pool_manager_test.cpp.o
test/clock_replacer_test: test/CMakeFiles/clock_replacer_test.dir/build.make
test/clock_replacer_test: lib/libbustub_shared.so
test/clock_replacer_test: lib/libgmock_main.so.1.12.1
test/clock_replacer_test: lib/libthirdparty_murmur3.so
test/clock_replacer_test: lib/libgmock.so.1.12.1
test/clock_replacer_test: lib/libgtest.so.1.12.1
test/clock_replacer_test: test/CMakeFiles/clock_replacer_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/tmp/tmp.IyJU8EsJ7j/cmake-build-debug-remote-host/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking CXX executable clock_replacer_test"
	cd /tmp/tmp.IyJU8EsJ7j/cmake-build-debug-remote-host/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/clock_replacer_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/CMakeFiles/clock_replacer_test.dir/build: test/clock_replacer_test

.PHONY : test/CMakeFiles/clock_replacer_test.dir/build

test/CMakeFiles/clock_replacer_test.dir/requires: test/CMakeFiles/clock_replacer_test.dir/buffer/clock_replacer_test.cpp.o.requires
test/CMakeFiles/clock_replacer_test.dir/requires: test/CMakeFiles/clock_replacer_test.dir/buffer/grading_buffer_pool_manager_instance_test.cpp.o.requires
test/CMakeFiles/clock_replacer_test.dir/requires: test/CMakeFiles/clock_replacer_test.dir/buffer/grading_lru_replacer_test.cpp.o.requires
test/CMakeFiles/clock_replacer_test.dir/requires: test/CMakeFiles/clock_replacer_test.dir/buffer/grading_parallel_buffer_pool_manager_test.cpp.o.requires

.PHONY : test/CMakeFiles/clock_replacer_test.dir/requires

test/CMakeFiles/clock_replacer_test.dir/clean:
	cd /tmp/tmp.IyJU8EsJ7j/cmake-build-debug-remote-host/test && $(CMAKE_COMMAND) -P CMakeFiles/clock_replacer_test.dir/cmake_clean.cmake
.PHONY : test/CMakeFiles/clock_replacer_test.dir/clean

test/CMakeFiles/clock_replacer_test.dir/depend:
	cd /tmp/tmp.IyJU8EsJ7j/cmake-build-debug-remote-host && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /tmp/tmp.IyJU8EsJ7j /tmp/tmp.IyJU8EsJ7j/test /tmp/tmp.IyJU8EsJ7j/cmake-build-debug-remote-host /tmp/tmp.IyJU8EsJ7j/cmake-build-debug-remote-host/test /tmp/tmp.IyJU8EsJ7j/cmake-build-debug-remote-host/test/CMakeFiles/clock_replacer_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/CMakeFiles/clock_replacer_test.dir/depend

