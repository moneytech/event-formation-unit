/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief class for configuration of multi blade detector settings
/// reading from json file.
//===----------------------------------------------------------------------===//

#pragma once

#include <mbcaen/MB16Detector.h>
#include <string>
#include <vector>

class MBConfig {
public:
  enum class InstrumentGeometry {Estia, Freia};

  struct Digitiser {
    int index; // order in which they are positioned in VME crate
    int digid; // digitiser id
  };

  ///
  MBConfig(){};

  /// \brief get configuration from file
  explicit MBConfig(std::string jsonfile);

  bool isConfigLoaded() {return IsConfigLoaded;}

  /// Name of the json configuratio file to load
  std::string ConfigFile{""};

  /// Specify the instrument geometry
  InstrumentGeometry instrument{InstrumentGeometry::Estia};

  /// Specify the digital geometry
  MB16Detector * detector{nullptr};

  /// local readout timestamp resolution
  uint32_t TimeTickNS{16};

  /// for now just hold a vector of the digitisers, \todo later
  /// incorporate in the digital geometry
  std::vector<struct Digitiser> Digitisers;

private:

  /// \brief helper function to load and parse json file
  void loadConfigFile();

  /// Set to true by loadConfigFile() if all is well
  bool IsConfigLoaded{false};
};
