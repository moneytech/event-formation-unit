#=============================================================================
# Tests
#=============================================================================

set(MGConfigTest_SRC
  ConfigTest.cpp
  ../Config.cpp
  )
set(MGConfigTest_INC
  ../Config.h
  )
set(MGConfigTest_LIB
  MgGeometry
  MgMesytec
  MgReduction
  )
create_test_executable(MGConfigTest)
target_compile_definitions(MGConfigTest
  PRIVATE TEST_JSON_PATH="${TEST_JSON_PATH}")

set(MultigridBaseTest_SRC
  MultigridBaseTest.cpp
  ../Config.cpp
  ../MultigridBase.cpp
  ../../adc_readout/test/TestUDPServer.cpp
  )
set(MultigridBaseTest_INC
  ../Config.h
  ../MultigridBase.h
  ../../adc_readout/test/TestUDPServer.h
  )
set(MultigridBaseTest_LIB
  MgGeometry
  MgMesytec
  MgReduction
  )
create_test_executable(MultigridBaseTest)
target_compile_definitions(MultigridBaseTest
  PRIVATE TEST_JSON_PATH="${TEST_JSON_PATH}")
