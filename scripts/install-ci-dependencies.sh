#!/bin/bash

# Set variables based on build environment
if [[ "$TRAVIS" == "true" ]]; then
    if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
        HOMEBREW_NO_AUTO_UPDATE=1 brew install pcre
    fi

    export CI_DEPENDENCY_DIR=${TRAVIS_BUILD_DIR}/dependencies

    # Convert commit message to lower case
    commit_msg=$(tr '[:upper:]' '[:lower:]' <<< ${TRAVIS_COMMIT_MESSAGE})

    if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
        os_name="Linux"
    elif [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
        os_name="Darwin"
    fi
else
    export CI_DEPENDENCY_DIR=$1
    commit_msg=""
    os_name="$(uname -s)"
fi

boost_version=$CI_BOOST_VERSION
if [[ -z "$CI_BOOST_VERSION" ]]; then
    boost_version=1.66.0
fi
boost_install_path=${CI_DEPENDENCY_DIR}/boost

cmake_version=3.11.0
cmake_install_path=${CI_DEPENDENCY_DIR}/cmake

sundials_version=3.1.0
sundials_install_path=${CI_DEPENDENCY_DIR}/sundials

# Wipe out cached dependencies if commit message has '[update_cache]'
if [[ $commit_msg == *'[update_cache]'* ]]; then
    individual="false"
    if [[ $commit_msg == *'boost'* ]]; then
        rm -rf ${boost_install_path};
        individual="true"
    fi
    if [[ $commit_msg == *'sundials'* ]]; then
        rm -rf ${sundials_install_path};
        individual="true"
    fi

    # If no dependency named in commit message, update entire cache
    if [[ "$individual" != 'true' ]]; then
        rm -rf ${CI_DEPENDENCY_DIR};
    fi
fi

if [[ ! -d "${CI_DEPENDENCY_DIR}" ]]; then
    mkdir -p ${CI_DEPENDENCY_DIR};
fi

# Install CMake
if [[ ! -d "${cmake_install_path}" ]]; then
    ./scripts/install-dependency.sh cmake ${cmake_version} ${cmake_install_path}
fi

# Set path to CMake executable depending on OS
if [[ "$os_name" == "Linux" ]]; then
    export PATH="${cmake_install_path}/bin:${PATH}"
    echo "*** cmake installed ($PATH)"
elif [[ "$os_name" == "Darwin" ]]; then
    export PATH="${cmake_install_path}/CMake.app/Contents/bin:${PATH}"
    echo "*** cmake installed ($PATH)"
fi

# Install Boost
if [[ ! -d "${boost_install_path}" ]]; then
    echo "*** build boost"
    travis_wait ./scripts/install-dependency.sh boost ${boost_version} ${boost_install_path}
    echo "*** built boost successfully"
fi

# Install Sundials
if [[ ! -d "${sundials_install_path}" ]]; then
    #scripts/install-dependency.sh sundials ${sundials_version} ${sundials_install_path}
    echo "*** built sundials successfully"
fi

if [[ "$os_name" == "Linux" ]]; then
    export LD_LIBRARY_PATH=${zmq_install_path}/lib:${boost_install_path}/lib:$LD_LIBRARY_PATH
elif [[ "$os_name" == "Darwin" ]]; then
    export DYLD_FALLBACK_LIBRARY_PATH=${zmq_install_path}/lib:${boost_install_path}/lib:$DYLD_FALLBACK_LIBRARY_PATH
fi

if [[ "$os_name" == "Linux" ]]; then
    export LD_LIBRARY_PATH=${PWD}/build/src/helics/shared_api_library/:$LD_LIBRARY_PATH
elif [[ "$os_name" == "Darwin" ]]; then
    export DYLD_FALLBACK_LIBRARY_PATH=${PWD}/build/src/helics/shared_api_library/:$DYLD_FALLBACK_LIBRARY_PATH
fi
