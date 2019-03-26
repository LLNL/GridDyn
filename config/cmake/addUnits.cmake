include(ExternalProject)
ExternalProject_Add(units
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/ThirdParty/units
    GIT_REPOSITORY  https://github.com/mcneish1/units.git
    GIT_TAG origin/GridDyn-integration
    UPDATE_COMMAND ""
    BINARY_DIR ${PROJECT_BINARY_DIR}/ThirdParty/units

    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${AUTOBUILD_INSTALL_PATH}
        -DUNITS_HEADER_ONLY=On
        -DUNITS_ENABLE_TESTS=Off

    INSTALL_DIR ${AUTOBUILD_INSTALL_PATH}
)

find_package(units REQUIRED)
