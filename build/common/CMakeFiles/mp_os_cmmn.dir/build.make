# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
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
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/adfx/mp_os

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/adfx/mp_os/build

# Include any dependencies generated for this target.
include common/CMakeFiles/mp_os_cmmn.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include common/CMakeFiles/mp_os_cmmn.dir/compiler_depend.make

# Include the progress variables for this target.
include common/CMakeFiles/mp_os_cmmn.dir/progress.make

# Include the compile flags for this target's objects.
include common/CMakeFiles/mp_os_cmmn.dir/flags.make

common/CMakeFiles/mp_os_cmmn.dir/src/not_implemented.cpp.o: common/CMakeFiles/mp_os_cmmn.dir/flags.make
common/CMakeFiles/mp_os_cmmn.dir/src/not_implemented.cpp.o: ../common/src/not_implemented.cpp
common/CMakeFiles/mp_os_cmmn.dir/src/not_implemented.cpp.o: common/CMakeFiles/mp_os_cmmn.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/adfx/mp_os/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object common/CMakeFiles/mp_os_cmmn.dir/src/not_implemented.cpp.o"
	cd /home/adfx/mp_os/build/common && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT common/CMakeFiles/mp_os_cmmn.dir/src/not_implemented.cpp.o -MF CMakeFiles/mp_os_cmmn.dir/src/not_implemented.cpp.o.d -o CMakeFiles/mp_os_cmmn.dir/src/not_implemented.cpp.o -c /home/adfx/mp_os/common/src/not_implemented.cpp

common/CMakeFiles/mp_os_cmmn.dir/src/not_implemented.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/mp_os_cmmn.dir/src/not_implemented.cpp.i"
	cd /home/adfx/mp_os/build/common && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/adfx/mp_os/common/src/not_implemented.cpp > CMakeFiles/mp_os_cmmn.dir/src/not_implemented.cpp.i

common/CMakeFiles/mp_os_cmmn.dir/src/not_implemented.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/mp_os_cmmn.dir/src/not_implemented.cpp.s"
	cd /home/adfx/mp_os/build/common && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/adfx/mp_os/common/src/not_implemented.cpp -o CMakeFiles/mp_os_cmmn.dir/src/not_implemented.cpp.s

common/CMakeFiles/mp_os_cmmn.dir/src/operation_not_supported.cpp.o: common/CMakeFiles/mp_os_cmmn.dir/flags.make
common/CMakeFiles/mp_os_cmmn.dir/src/operation_not_supported.cpp.o: ../common/src/operation_not_supported.cpp
common/CMakeFiles/mp_os_cmmn.dir/src/operation_not_supported.cpp.o: common/CMakeFiles/mp_os_cmmn.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/adfx/mp_os/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object common/CMakeFiles/mp_os_cmmn.dir/src/operation_not_supported.cpp.o"
	cd /home/adfx/mp_os/build/common && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT common/CMakeFiles/mp_os_cmmn.dir/src/operation_not_supported.cpp.o -MF CMakeFiles/mp_os_cmmn.dir/src/operation_not_supported.cpp.o.d -o CMakeFiles/mp_os_cmmn.dir/src/operation_not_supported.cpp.o -c /home/adfx/mp_os/common/src/operation_not_supported.cpp

common/CMakeFiles/mp_os_cmmn.dir/src/operation_not_supported.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/mp_os_cmmn.dir/src/operation_not_supported.cpp.i"
	cd /home/adfx/mp_os/build/common && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/adfx/mp_os/common/src/operation_not_supported.cpp > CMakeFiles/mp_os_cmmn.dir/src/operation_not_supported.cpp.i

common/CMakeFiles/mp_os_cmmn.dir/src/operation_not_supported.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/mp_os_cmmn.dir/src/operation_not_supported.cpp.s"
	cd /home/adfx/mp_os/build/common && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/adfx/mp_os/common/src/operation_not_supported.cpp -o CMakeFiles/mp_os_cmmn.dir/src/operation_not_supported.cpp.s

# Object files for target mp_os_cmmn
mp_os_cmmn_OBJECTS = \
"CMakeFiles/mp_os_cmmn.dir/src/not_implemented.cpp.o" \
"CMakeFiles/mp_os_cmmn.dir/src/operation_not_supported.cpp.o"

# External object files for target mp_os_cmmn
mp_os_cmmn_EXTERNAL_OBJECTS =

common/libmp_os_cmmn.a: common/CMakeFiles/mp_os_cmmn.dir/src/not_implemented.cpp.o
common/libmp_os_cmmn.a: common/CMakeFiles/mp_os_cmmn.dir/src/operation_not_supported.cpp.o
common/libmp_os_cmmn.a: common/CMakeFiles/mp_os_cmmn.dir/build.make
common/libmp_os_cmmn.a: common/CMakeFiles/mp_os_cmmn.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/adfx/mp_os/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX static library libmp_os_cmmn.a"
	cd /home/adfx/mp_os/build/common && $(CMAKE_COMMAND) -P CMakeFiles/mp_os_cmmn.dir/cmake_clean_target.cmake
	cd /home/adfx/mp_os/build/common && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/mp_os_cmmn.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
common/CMakeFiles/mp_os_cmmn.dir/build: common/libmp_os_cmmn.a
.PHONY : common/CMakeFiles/mp_os_cmmn.dir/build

common/CMakeFiles/mp_os_cmmn.dir/clean:
	cd /home/adfx/mp_os/build/common && $(CMAKE_COMMAND) -P CMakeFiles/mp_os_cmmn.dir/cmake_clean.cmake
.PHONY : common/CMakeFiles/mp_os_cmmn.dir/clean

common/CMakeFiles/mp_os_cmmn.dir/depend:
	cd /home/adfx/mp_os/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/adfx/mp_os /home/adfx/mp_os/common /home/adfx/mp_os/build /home/adfx/mp_os/build/common /home/adfx/mp_os/build/common/CMakeFiles/mp_os_cmmn.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : common/CMakeFiles/mp_os_cmmn.dir/depend

