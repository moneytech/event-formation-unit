
get_filename_component(TEST_JSON_PATH "configs/v20_mappings.json" DIRECTORY)
get_filename_component(TEST_CSV_PATH "configs/sumo_voxel_map_20190711.csv" DIRECTORY)

set(JalReadoutTest_SRC
  ReadoutTest.cpp
  Readout.cpp
  )
set(JalReadoutTest_INC
  Readout.h
  )
create_test_executable(JalReadoutTest SKIP_MEMGRIND)

set(TEST_DATA_PATH "${REFDATA}/jalousie/2019_07")
if(EXISTS ${TEST_DATA_PATH})
  set(JalCdtFileTest_SRC
    CdtFileTest.cpp
    CdtFile.cpp
    Readout.cpp
    )
  set(JalCdtFileTest_INC
    CdtFile.h
    Readout.h
    )
  create_test_executable(JalCdtFileTest)
  target_compile_definitions(JalCdtFileTest
    PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}/")

  set(JalBuilderReadoutsTest_SRC
    BuilderReadoutsTest.cpp
    BuilderReadouts.cpp
    Readout.cpp
    CdtFile.cpp
    )
  set(JalBuilderReadoutsTest_INC
    BuilderReadouts.h
    Readout.h
    CdtFile.h
    )
  create_test_executable(JalBuilderReadoutsTest)
  target_compile_definitions(JalBuilderReadoutsTest
    PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}/")

  set(JalSumoMappingsTest_SRC
    SumoMappingsTest.cpp
    SumoMappings.cpp
    )
  set(JalSumoMappingsTest_INC
    SumoMappings.h
    )
  create_test_executable(JalSumoMappingsTest)
  target_compile_definitions(JalSumoMappingsTest
    PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}/")

  set(JalConfigTest_SRC
    ConfigTest.cpp
    Config.cpp
    SumoMappings.cpp
    )
  set(JalConfigTest_INC
    Config.h
    SumoMappings.h
    )
  create_test_executable(JalConfigTest)
  target_compile_definitions(JalConfigTest
    PRIVATE TEST_DATA_PATH="${TEST_DATA_PATH}/")
else()
  message(WARNING "*** No Jalousie reference data found. Skipping JalCdtFileTest.")
  message(WARNING "*** No Jalousie reference data found. Skipping JalBuilderReadoutsTest.")
  message(WARNING "*** No Jalousie reference data found. Skipping JalSumoMappingsTest.")
endif()

set(udpgen_jalousie_SRC
  ../generators/generator.cpp
  CdtFile.cpp
  Readout.cpp
  )
set(udpgen_jalousie_INC
  CdtFile.h
  Readout.h
  )
create_executable(udpgen_jalousie)
target_compile_definitions(udpgen_jalousie PUBLIC GENERATOR_JALOUSIE_CDT)

set(jalousie_common_inc
  Config.h
  Readout.h
  SumoMappings.h
  rapidcsv.h
  )
set(jalousie_common_src
  Config.cpp
  Readout.cpp
  SumoMappings.cpp
  )

#
set(jalousie_INC
  JalousieBase.h
  ${jalousie_common_inc}
  )
set(jalousie_SRC
  JalousieBase.cpp
  Jalousie.cpp
  ${jalousie_common_src}
  )
create_module(jalousie)

#
set(JalousieBaseTest_INC
  ${jalousie_common_inc}
  ../test/SaveBuffer.h
  ../adc_readout/test/TestUDPServer.h)
set(JalousieBaseTest_SRC
  JalousieBase.cpp
  JalousieBaseTest.cpp
  ${jalousie_common_src}
  ../test/SaveBuffer.cpp
  ../adc_readout/test/TestUDPServer.cpp)
create_test_executable(JalousieBaseTest)
target_compile_definitions(JalousieBaseTest
  PRIVATE TEST_JSON_PATH="${CMAKE_CURRENT_SOURCE_DIR}/${TEST_JSON_PATH}/")
target_compile_definitions(JalousieBaseTest
PRIVATE TEST_CSV_PATH="${CMAKE_CURRENT_SOURCE_DIR}/${TEST_CSV_PATH}/")

add_subdirectory(generators)
