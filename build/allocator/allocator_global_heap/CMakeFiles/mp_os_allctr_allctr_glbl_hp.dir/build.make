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
include allocator/allocator_global_heap/CMakeFiles/mp_os_allctr_allctr_glbl_hp.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include allocator/allocator_global_heap/CMakeFiles/mp_os_allctr_allctr_glbl_hp.dir/compiler_depend.make

# Include the progress variables for this target.
include allocator/allocator_global_heap/CMakeFiles/mp_os_allctr_allctr_glbl_hp.dir/progress.make

# Include the compile flags for this target's objects.
include allocator/allocator_global_heap/CMakeFiles/mp_os_allctr_allctr_glbl_hp.dir/flags.make

allocator/allocator_global_heap/CMakeFiles/mp_os_allctr_allctr_glbl_hp.dir/src/allocator_global_heap.cpp.o: allocator/allocator_global_heap/CMakeFiles/mp_os_allctr_allctr_glbl_hp.dir/flags.make
allocator/allocator_global_heap/CMakeFiles/mp_os_allctr_allctr_glbl_hp.dir/src/allocator_global_heap.cpp.o: ../allocator/allocator_global_heap/src/allocator_global_heap.cpp
allocator/allocator_global_heap/CMakeFiles/mp_os_allctr_allctr_glbl_hp.dir/src/allocator_global_heap.cpp.o: allocator/allocator_global_heap/CMakeFiles/mp_os_allctr_allctr_glbl_hp.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/adfx/mp_os/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object allocator/allocator_global_heap/CMakeFiles/mp_os_allctr_allctr_glbl_hp.dir/src/allocator_global_heap.cpp.o"
	cd /home/adfx/mp_os/build/allocator/allocator_global_heap && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT allocator/allocator_global_heap/CMakeFiles/mp_os_allctr_allctr_glbl_hp.dir/src/allocator_global_heap.cpp.o -MF CMakeFiles/mp_os_allctr_allctr_glbl_hp.dir/src/allocator_global_heap.cpp.o.d -o CMakeFiles/mp_os_allctr_allctr_glbl_hp.dir/src/allocator_global_heap.cpp.o -c /home/adfx/mp_os/allocator/allocator_global_heap/src/allocator_global_heap.cpp

allocator/allocator_global_heap/CMakeFiles/mp_os_allctr_allctr_glbl_hp.dir/src/allocator_global_heap.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/mp_os_allctr_allctr_glbl_hp.dir/src/allocator_global_heap.cpp.i"
	cd /home/adfx/mp_os/build/allocator/allocator_global_heap && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/adfx/mp_os/allocator/allocator_global_heap/src/allocator_global_heap.cpp > CMakeFiles/mp_os_allctr_allctr_glbl_hp.dir/src/allocator_global_heap.cpp.i

allocator/allocator_global_heap/CMakeFiles/mp_os_allctr_allctr_glbl_hp.dir/src/allocator_global_heap.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/mp_os_allctr_allctr_glbl_hp.dir/src/allocator_global_heap.cpp.s"
	cd /home/adfx/mp_os/build/allocator/allocator_global_heap && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/adfx/mp_os/allocator/allocator_global_heap/src/allocator_global_heap.cpp -o CMakeFiles/mp_os_allctr_allctr_glbl_hp.dir/src/allocator_global_heap.cpp.s

# Object files for target mp_os_allctr_allctr_glbl_hp
mp_os_allctr_allctr_glbl_hp_OBJECTS = \
"CMakeFiles/mp_os_allctr_allctr_glbl_hp.dir/src/allocator_global_heap.cpp.o"

# External object files for target mp_os_allctr_allctr_glbl_hp
mp_os_allctr_allctr_glbl_hp_EXTERNAL_OBJECTS =

allocator/allocator_global_heap/libmp_os_allctr_allctr_glbl_hp.a: allocator/allocator_global_heap/CMakeFiles/mp_os_allctr_allctr_glbl_hp.dir/src/allocator_global_heap.cpp.o
allocator/allocator_global_heap/libmp_os_allctr_allctr_glbl_hp.a: allocator/allocator_global_heap/CMakeFiles/mp_os_allctr_allctr_glbl_hp.dir/build.make
allocator/allocator_global_heap/libmp_os_allctr_allctr_glbl_hp.a: allocator/allocator_global_heap/CMakeFiles/mp_os_allctr_allctr_glbl_hp.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/adfx/mp_os/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libmp_os_allctr_allctr_glbl_hp.a"
	cd /home/adfx/mp_os/build/allocator/allocator_global_heap && $(CMAKE_COMMAND) -P CMakeFiles/mp_os_allctr_allctr_glbl_hp.dir/cmake_clean_target.cmake
	cd /home/adfx/mp_os/build/allocator/allocator_global_heap && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/mp_os_allctr_allctr_glbl_hp.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
allocator/allocator_global_heap/CMakeFiles/mp_os_allctr_allctr_glbl_hp.dir/build: allocator/allocator_global_heap/libmp_os_allctr_allctr_glbl_hp.a
.PHONY : allocator/allocator_global_heap/CMakeFiles/mp_os_allctr_allctr_glbl_hp.dir/build

allocator/allocator_global_heap/CMakeFiles/mp_os_allctr_allctr_glbl_hp.dir/clean:
	cd /home/adfx/mp_os/build/allocator/allocator_global_heap && $(CMAKE_COMMAND) -P CMakeFiles/mp_os_allctr_allctr_glbl_hp.dir/cmake_clean.cmake
.PHONY : allocator/allocator_global_heap/CMakeFiles/mp_os_allctr_allctr_glbl_hp.dir/clean

allocator/allocator_global_heap/CMakeFiles/mp_os_allctr_allctr_glbl_hp.dir/depend:
	cd /home/adfx/mp_os/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/adfx/mp_os /home/adfx/mp_os/allocator/allocator_global_heap /home/adfx/mp_os/build /home/adfx/mp_os/build/allocator/allocator_global_heap /home/adfx/mp_os/build/allocator/allocator_global_heap/CMakeFiles/mp_os_allctr_allctr_glbl_hp.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : allocator/allocator_global_heap/CMakeFiles/mp_os_allctr_allctr_glbl_hp.dir/depend

