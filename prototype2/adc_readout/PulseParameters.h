/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Simple struct for containing pulse data.
///
//===----------------------------------------------------------------------===//

#pragma once

#include "AdcParse.h"
#include "AdcTimeStamp.h"

/// \brief Stores data about a pulse from the ADC system, including the source.
struct PulseParameters {
  ChannelID Identifier{0, 0};
  /// Background value at location of peak
  int BackgroundLevel{0};
  /// Maximum value of peak
  int PeakLevel{0};
  /// Difference between peak and background at peak
  int PeakAmplitude{0};
  /// Area under the curve with background subtracted
  int PeakArea{0};
  /// Timestamp of maximum of peak
  RawTimeStamp PeakTimestamp;
  /// Timestamp where line goes above threshold
  RawTimeStamp ThresholdTimestamp;
};
