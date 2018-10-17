/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Unit tests.
 */

#include <sonde/SoNDeBase.h>
#include <sonde/SoNDeBaseTestData.h>
#include <../prototype2/adc_readout/test/TestUDPServer.h>
#include <test/TestBase.h>

class SONDEIDEABaseStandIn : public SONDEIDEABase {
public:
  SONDEIDEABaseStandIn(BaseSettings Settings, struct SoNDeSettings ReadoutSettings)
      : SONDEIDEABase(Settings, ReadoutSettings){};
  ~SONDEIDEABaseStandIn() {};
  using Detector::Threads;
  using SONDEIDEABase::mystats;
};

class SoNDeBaseTest : public ::testing::Test {
public:
  virtual void SetUp() {
    Settings.DetectorRxBufferSize = 100000;
    Settings.MinimumMTU = 1500;
  }
  virtual void TearDown() {}

  BaseSettings Settings;
  SoNDeSettings LocalSettings;
};

TEST_F(SoNDeBaseTest, Constructor) {
  SONDEIDEABaseStandIn Readout(Settings, LocalSettings);
  EXPECT_EQ(Readout.mystats.rx_packets, 0);
  EXPECT_EQ(Readout.mystats.rx_events, 0);
}


TEST_F(SoNDeBaseTest, DataReceive) {
  SONDEIDEABaseStandIn Readout(Settings, LocalSettings);
  Readout.startThreads();
  std::chrono::duration<std::int64_t, std::milli> SleepTime{400};
  std::this_thread::sleep_for(SleepTime);
  TestUDPServer Server(43126, Settings.DetectorPort, (unsigned char *)&sondedata[0], sondedata.size());
  Server.startPacketTransmission(1, 100);
  std::this_thread::sleep_for(SleepTime);
  Readout.stopThreads();
  EXPECT_EQ(Readout.mystats.rx_packets, 1);
  EXPECT_EQ(Readout.mystats.rx_bytes, sondedata.size());
  EXPECT_EQ(Readout.mystats.rx_events, 250); // number of readouts == events
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
