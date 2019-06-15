/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Multigrid electronics
 * Handles mappings between (digitizer, channels) and (x,y,z) coordinates.
 *
 */

#pragma once
#include <multigrid/geometry/ModuleGeometry.h>

#include <cinttypes>
#include <string>
#include <vector>

namespace Multigrid {

class MGSeqGeometry : public ModuleChannelMappings {
public:

  // Configuration
  void swap_wires(bool s);
  void swap_grids(bool s);
  void max_channel(uint16_t g);
  bool swap_wires() const;
  bool swap_grids() const;

  void max_wire(uint16_t w);
  uint16_t max_wire() const;

  // Implementation

  uint16_t max_channel() const override;

  /** @brief identifies which channels are wires, from drawing by Anton */
  bool isWire(uint8_t VMM, uint16_t channel) const override;

  /** @brief identifies which channels are grids, from drawing by Anton */
  bool isGrid(uint8_t VMM, uint16_t channel) const override;

  /** @brief returns wire */
  uint16_t wire(uint8_t VMM, uint16_t channel) const override;

  /** @brief returns grid */
  uint16_t grid(uint8_t VMM, uint16_t channel) const override;

  std::string debug(std::string prefix) const override;

protected:
  static void swap(uint16_t &channel);

  uint16_t max_channel_{120};
  uint16_t max_wire_{80};

  bool swap_wires_{false};
  bool swap_grids_{false};
};

void from_json(const nlohmann::json &j, MGSeqGeometry &g);

}

