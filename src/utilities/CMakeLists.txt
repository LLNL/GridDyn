# LLNS Copyright Start
# Copyright (c) 2014-2018, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department
# of Energy by Lawrence Livermore National Laboratory in part under
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

# project name

set(utilities_sources
    gridRandom.cpp
    saturation.cpp
    ThreePhaseFunctions.cpp
    matrixOps.cpp
    functionInterpreter.cpp
    zipUtilities.cpp
    matrixCreation.cpp
    logger.cpp
    loggerCore.cpp
    matrixDataSparse.cpp
    OperatingBoundary.cpp
    GlobalWorkQueue.cpp
)

set(utilities_headers
    dataDictionary.hpp
    ThreePhaseFunctions.h
    gridRandom.h
    saturation.h
    vectData.hpp
    matrixData.hpp
    matrixOps.h
    matrixDataCompact.hpp
    matrixDataSparse.hpp
    matrixDataSparse_impl.hpp
    matrixDataBoost.hpp
    matrixDataSparseSM.hpp
    matrixDataTranslate.hpp
    matrixDataContainer.hpp
    matrixDataScale.hpp
    matrixDataOrdered.hpp
    matrixDataOrdering.hpp
    matrixDataFilter.hpp
    matrixDataCustomWriteOnly.hpp
    functionInterpreter.h
    zipUtilities.h
    valuePredictor.hpp
    matrixCreation.h
    indexTypes.hpp
    logger.h
    loggerCore.hpp
    OperatingBoundary.h
    GlobalWorkQueue.hpp
)

add_library(utilities STATIC ${utilities_sources} ${utilities_headers})
set(SKIP_INSTALL_LIBRARIES ON)
add_subdirectory(
    ${PROJECT_SOURCE_DIR}/ThirdParty/Zlib/zlib-1.2.6 ${PROJECT_BINARY_DIR}/Zlib EXCLUDE_FROM_ALL
)

add_subdirectory(
    ${PROJECT_SOURCE_DIR}/ThirdParty/Minizip ${PROJECT_BINARY_DIR}/Minizip EXCLUDE_FROM_ALL
)

target_link_libraries(utilities PUBLIC minizip::minizip zlib::zlib gmlc_utilities griddyn_base)

set_target_properties(utilities PROPERTIES FOLDER libraries)
set_target_properties(minizip zlib PROPERTIES FOLDER Extern)
if(ENABLE_64_BIT_INDEXING)
    target_compile_definitions(utilities PUBLIC -DENABLE_64_BIT_INDEXING)
endif(ENABLE_64_BIT_INDEXING)
if(UNSIGNED_INDEXING)
    target_compile_definitions(utilities PUBLIC -DUNSIGNED_INDEXING)
endif(UNSIGNED_INDEXING)
