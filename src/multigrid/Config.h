/// Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <multigrid/AbstractBuilder.h>
#include <multigrid/geometry/DetectorMappings.h>
#include <multigrid/reduction/Reduction.h>
#include <memory>

namespace Multigrid {

struct Config {
  Config() = default;
  explicit Config(const std::string& jsonfile, std::string dump_path = "");

  DetectorMappings mappings;
  std::shared_ptr<AbstractBuilder> builder;
  Reduction reduction;

  std::string debug() const;
};

}
