# LLNS Copyright Start
# Copyright (c) 2014-2018, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department
# of Energy by Lawrence Livermore National Laboratory in part under
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

# project name gridDynInout

set(
    reader_sources
    stringInterpret.cpp
    readerHelper.cpp
    fileInput.cpp
    gridParameter.cpp
    readerInfo.cpp
)

set(
    elementReader_sources
    readElementHelperFunctions.cpp
    readBusElement.cpp
    readEventElement.cpp
    readLinkElement.cpp
    readCollectorElement.cpp
    readEconElement.cpp
    readLibraryElement.cpp
    readAreaElement.cpp
    readArrayElement.cpp
    readConditionElement.cpp
    readRelayElement.cpp
    readSolverElement.cpp
    readSimulationElement.cpp
    loadSubObjectsElement.cpp
    readElementFile.cpp
    objectLoadFromElementHelperFunctions.cpp
    objectLoadHelperFunctions.cpp
)

set(
    formattedfileInput_sources
    gridDynReadCDF.cpp
    gridDynReadRAW.cpp
    gridDynReadDYR.cpp
    gridDynReadPSP.cpp
    gridDynReadPTI.cpp
    gridReadEPC.cpp
    gridReadMatPower.cpp
    gridReadPSAT.cpp
    readMatDyn.cpp
    gridDynReadCSV.cpp
    readMatlabData.cpp
    loadGDZ.cpp
    readXMLfile.cpp
)

set(
    fileInput_headers
    readElement.h
    readerHelper.h
    fileInput.h
    elementReaderTemplates.hpp
    readerInfo.h
    gridParameter.h
    readElementFile.h
)

set(
    fileInput_sources
    ${xml_sources}
    ${reader_sources}
    ${elementReader_sources}
    ${formattedfileInput_sources}
    ${fileInput_headers}
)
add_library(fileInput STATIC ${fileInput_sources})

target_link_libraries(fileInput PUBLIC gridDynLoader formatInterpreter)
source_group("Elements" FILES ${elementReader_sources})
source_group("formatSpecific" FILES ${formattedfileInput_sources})
source_group("general" FILES ${reader_sources})

set_target_properties(fileInput PROPERTIES FOLDER libraries)

install(
    TARGETS fileInput
    EXPORT griddyn-targets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT libraries
)

install(
    FILES ${fileInput_headers}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/griddyn/fileInput
    COMPONENT headers
)
