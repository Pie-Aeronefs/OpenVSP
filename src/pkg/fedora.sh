#!/bin/sh

# These are the instructions we would put in the
# Githib workflow file, except that we want to run
# them from within a Docker container, so it is
# more useful to have them here in fedora.sh.

set -e

# docker shares the workspace directory into /workspace
GITHUB_WORKSPACE=/workspace
export GITHUB_WORKSPACE
cd $GITHUB_WORKSPACE
mkdir -p build buildlibs artifacts

case $1 in
install)
    while pgrep dnf
    do
        echo another dnf is stll running
        sleep 1
    done
    dnf install -y cmake libxml2-devel fltk-fluid fltk-devel g++ \
            openjpeg-devel glm-devel \
            cminpack-devel glew-devel swig doxygen graphviz texlive-scheme-basic \
            python3-devel
    ;;
libs)
    cd $GITHUB_WORKSPACE/buildlibs
    cmake -DCMAKE_BUILD_TYPE=Release -DVSP_USE_SYSTEM_LIBXML2=true -DVSP_USE_SYSTEM_FLTK=true -DVSP_USE_SYSTEM_GLM=true \
        -DVSP_USE_SYSTEM_GLEW=true -DVSP_USE_SYSTEM_CMINPACK=true -DVSP_USE_SYSTEM_CPPTEST=false ../Libraries
    make -j
    ;;
exe)
    cd $GITHUB_WORKSPACE/build
    cmake -DCMAKE_BUILD_TYPE=Release -DVSP_CPACK_GEN=DEB -DVSP_LIBRARY_PATH=$GITHUB_WORKSPACE/buildlibs ../src
    make -j
    ;;
rpm)
    cd $GITHUB_WORKSPACE/build
    cmake -DCMAKE_BUILD_TYPE=Release -DVSP_CPACK_GEN=DEB -DVSP_LIBRARY_PATH=$GITHUB_WORKSPACE/buildlibs ../src
    make -j package
    cp *.rpm $GITHUB_WORKSPACE/artifacts
    ls -l $GITHUB_WORKSPACE/artifacts
    echo "RPM CREATION SUCCESSFUL"
    ;;
*)
    echo "unknown step $1"
    exit 1
esac

