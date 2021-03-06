/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial LoKI readouts with variable number
/// of sections and data elements per section.
//===----------------------------------------------------------------------===//

#pragma once

#include <loki/readout/DataParser.h>

/// \brief Fill out specified buffer with LoKI readouts
uint16_t lokiReadoutDataGen(uint16_t DataSections, uint16_t DataElements, uint8_t Rings,
     uint8_t * Buffer, uint16_t MaxSize, uint32_t SeqNum);
