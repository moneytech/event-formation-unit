/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief LoKI detector base plugin interface definition
///
//===----------------------------------------------------------------------===//
#pragma once

#include <common/Detector.h>
#include <common/EV42Serializer.h>
#include <common/RingBuffer.h>
#include <common/SPSCFifo.h>
#include <loki/Counters.h>
#include <loki/geometry/Config.h>
#include <loki/geometry/Calibration.h>
#include <loki/readout/DataParser.h>
#include <loki/readout/Readout.h>
#include <readout/ReadoutParser.h>
#include <readout/ESSTime.h>
#include <loki/geometry/TubeAmps.h>

namespace Loki {

struct LokiSettings {
  std::string ConfigFile{""}; ///< panel mappings
  std::string CalibFile{""}; ///< calibration file
  std::string FilePrefix{""}; ///< HDF5 file dumping
  bool DetectorImage2D{false}; ///< generate pixels for 2D detector (else 3D)
};

using namespace memory_sequential_consistent; // Lock free fifo

class LokiBase : public Detector {
public:
  LokiBase(BaseSettings const &Settings, struct LokiSettings &LocalLokiSettings);
  ~LokiBase() = default;
  void inputThread();
  void processingThread();
  void testImageUdder(EV42Serializer& FlatBuffer);

  /// \todo figure out the right size  of EthernetBufferMaxEntries
  static const int EthernetBufferMaxEntries {2000};
  static const int EthernetBufferSize {9000}; /// bytes
  static const int KafkaBufferSize {124000}; /// entries ~ 1MB

  // Ideally should match the CPU speed, but as this varies across
  // CPU versions we just select something in the 'middle'. This is
  // used to get an approximate time for periodic housekeeping so
  // it is not critical that this is precise.
  const int TSC_MHZ = 2900;

protected:
  /** Shared between input_thread and processing_thread*/
  CircularFifo<unsigned int, EthernetBufferMaxEntries> InputFifo;
  /// \todo the number 11 is a workaround
  RingBuffer<EthernetBufferSize> RxRingbuffer{EthernetBufferMaxEntries + 11};

  // From Counters.h
  struct Counters Counters;

  LokiSettings LokiModuleSettings;

  /// \brief Used in Processing thread
  // From geometry/Config.h
  Config LokiConfiguration;
  Calibration LokiCalibration;
  ReadoutParser ESSReadoutParser;
  DataParser LokiParser{Counters};
  TubeAmps Amp2Pos;
  ESSTime Time;
};

}
