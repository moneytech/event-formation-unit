

set(DynamicHistTest_INC
  ../DynamicHist.h
)
set(DynamicHistTest_SRC
  ../DynamicHist.cpp
  DynamicHistTest.cpp
)
create_test_executable(DynamicHistTest)

set(HistogramTest_SRC
  HistogramTest.cpp
  )
create_test_executable(HistogramTest)

set(HistogramSerializerTest_SRC
  HistogramSerializerTest.cpp
  )
create_test_executable(HistogramSerializerTest)

set(HitSerializerTest_SRC
  HitSerializerTest.cpp
  )
create_test_executable(HitSerializerTest)

# todo Monitor tests
