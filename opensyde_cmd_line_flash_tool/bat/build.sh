#!/bin/bash
# script usage: 
# * ./build.sh       -> build a cmake configuration, compile only changed files and install build target
# * ./build.sh -l    -> only build cmake configuration (needed for pc_lint_plus scripts)
# * ./build.sh -c    -> clean build directory and exit
# * ./build.sh -d    -> build in Debug mode

build_only_lint_config=false
build_type="Release"

# exit with error code if a command fails
set -e

while getopts ':lcd' flag; do
 case "$flag" in
    l)
        echo "Only build cmake configuration for pc_lint usage"
        build_only_lint_config=true
        ;;
    c)
        echo "Cleaning build directory..."
        [ -d ../temp ] && rm -rf ../temp
        echo "✅ Clean complete."
        exit 0
        ;;
    d)
        echo "Debug build selected"
        build_type="Debug"
        ;;
   \?)
        echo "script usage:"
        echo "  ./build.sh      -> build changed files and install project"
        echo "  ./build.sh -l   -> only cmake config (pc_lint usage)"
        echo "  ./build.sh -c   -> clean build directory"
        echo "  ./build.sh -d   -> debug build"
        exit 1
        ;;
 esac
done

# create build directory if not exists
[ ! -d ../temp ] && mkdir ../temp
cd ../temp

echo "run cmake with toolchain file for $build_type compilation"
cmake ../pjt -DCMAKE_BUILD_TYPE=$build_type -DCMAKE_TOOLCHAIN_FILE=../pjt/toolchain_ubuntu.cmake 

if [ "$build_only_lint_config" = false ] ; then
    echo "build only changed files and install"
    cmake --build . --target all -- -j$(nproc)
    cmake --build . --target install
fi

cd ../bat
