/** Copyright (C) 2016 European Spallation Source */

#include <CSPECChanConv.h>
#include <gtest/gtest.h>

using namespace std;

class CspecChanConvTest : public ::testing::Test {
protected:
  CSPECChanConv conv;
};

/** Test cases below */
TEST_F(CspecChanConvTest, Constructor) {
  for (int i = 0; i < CSPECChanConv::adcsize; i++) {
    ASSERT_EQ(conv.getWireId(i), 0) << "wrong wire id conversion at adc value "
                                    << i << endl;
    ASSERT_EQ(conv.getGridId(i), 0) << "wrong grid id conversion at adc value "
                                    << i << endl;
  }
}

TEST_F(CspecChanConvTest, InvalidCalibrationParms) {
  int ret = conv.makewirecal(20000, 20500, 128);
  ASSERT_EQ(ret, -1);

  ret = conv.makewirecal(2000, 500, 128);
  ASSERT_EQ(ret, -1);

  ret = conv.makewirecal(2000, 2000, 128);
  ASSERT_EQ(ret, -1);

  ret = conv.makewirecal(0, 127, 128);
  ASSERT_EQ(ret, -1);
}

TEST_F(CspecChanConvTest, GenerateCalibration) {
  int ret = conv.makewirecal(400, 2000, 128);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(conv.getWireId(0), CSPECChanConv::adcsize - 1);
  ASSERT_EQ(conv.getWireId(399), CSPECChanConv::adcsize - 1);
  ASSERT_EQ(conv.getWireId(400), 0);
  ASSERT_EQ(conv.getWireId(2000), 128);
  ASSERT_EQ(conv.getWireId(2001), CSPECChanConv::adcsize - 1);
  ASSERT_EQ(conv.getWireId(CSPECChanConv::adcsize - 1),
            CSPECChanConv::adcsize - 1);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
