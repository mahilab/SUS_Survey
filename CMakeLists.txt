# MIT License

# Copyright (c) 2022 Mechatronics and Haptic Interfaces Lab - Rice University

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# Author(s): Zane Zook (zaz2@rice.edu)

# check for cmake
cmake_minimum_required(VERSION 3.18.2)

# create project
project(Focus-Haptics_Direction VERSION 0.1.0 LANGUAGES CXX)

# fetch require libs for project
include(FetchContent)

# MAHI GUI
FetchContent_Declare(mahi-gui GIT_REPOSITORY https://github.com/mahilab/mahi-gui.git) 
FetchContent_MakeAvailable(mahi-gui)

# add the include directory for compiler and to add intellisense capabilities
include_directories("include")

# build identification test executable
add_executable(SUS_Survey
    "src/SUS.cpp"
    "favicon.rc")
target_link_libraries(SUS_Survey mahi::gui)