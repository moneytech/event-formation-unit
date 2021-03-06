/** Copyright (C) 2016-2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
/// Processing pipeline for CSPEC instrument (Multi-Grid detector using
/// Mesytec readout)
///
//===----------------------------------------------------------------------===//

#include <multigrid/MultigridBase.h>
#include <multigrid/geometry/PlaneMappings.h>

#include <common/Producer.h>
#include <common/TimeString.h>
#include <efu/Parser.h>
#include <efu/Server.h>

#include <common/Socket.h>
#include <common/TSCTimer.h>
#include <common/Timer.h>

#include <common/Trace.h>
#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

#include <common/Log.h>
#include "MultigridBase.h"
//#undef TRC_MASK
//#define TRC_MASK 0

// \todo MJC's workstation - not reliable
static constexpr int TscMHz{2900};

MultigridBase::MultigridBase(BaseSettings const &settings, MultigridSettings const &LocalSettings)
    : Detector("CSPEC", settings), ModuleSettings(LocalSettings) {

  Stats.setPrefix(EFUSettings.GraphitePrefix, EFUSettings.GraphiteRegion);

  LOG(INIT, Sev::Info, "Adding stats");
  // clang-format off
  Stats.create("rx_packets", Counters.rx_packets);
  Stats.create("rx_bytes", Counters.rx_bytes);
  Stats.create("rx_idle", Counters.rx_idle);
  Stats.create("readouts_total", Counters.readouts_total);
  Stats.create("rx_discarded_bytes", Counters.parser_discarded_bytes);
  Stats.create("parser_triggers", Counters.parser_triggers);
  Stats.create("builder_glitch_rejects", Counters.builder_glitch_rejects);
  Stats.create("builder_filter_rejects", Counters.builder_filter_rejects);
  Stats.create("builder_geometry_errors", Counters.builder_geometry_errors);
  Stats.create("hits_total", Counters.hits_total);
  Stats.create("hits_time_seq_err", Counters.hits_time_seq_err);
  Stats.create("hits_bad_plane", Counters.hits_bad_plane);
  Stats.create("hits_used", Counters.hits_used);
  Stats.create("wire_clusters", Counters.wire_clusters);
  Stats.create("grid_clusters", Counters.grid_clusters);
  Stats.create("events_total", Counters.events_total);
  Stats.create("events_multiplicity_rejects", Counters.events_multiplicity_rejects);
  Stats.create("events_bad", Counters.events_bad);
  Stats.create("events_geometry_err", Counters.events_geometry_err);
  Stats.create("events_time_err", Counters.events_time_err);
  Stats.create("tx_events", Counters.tx_events);
  Stats.create("tx_bytes", Counters.tx_bytes);

  /// \todo below stats are common to all detectors and could/should be moved
  Stats.create("kafka.produce_fails", Counters.kafka_produce_fails);
  Stats.create("kafka.ev_errors", Counters.kafka_ev_errors);
  Stats.create("kafka.ev_others", Counters.kafka_ev_others);
  Stats.create("kafka.dr_errors", Counters.kafka_dr_errors);
  Stats.create("kafka.dr_others", Counters.kafka_dr_noerrors);
  // clang-format on

  LOG(INIT, Sev::Info, "Stream monitor data = {}",
      (ModuleSettings.monitor ? "YES" : "no"));
  if (!ModuleSettings.FilePrefix.empty())
    LOG(INIT, Sev::Info, "Dump h5 data in path: {}",
        ModuleSettings.FilePrefix);

  std::function<void()> inputFunc = [this]() { MultigridBase::mainThread(); };
  Detector::AddThreadFunction(inputFunc, "main");
}

bool MultigridBase::init_config() {
  LOG(INIT, Sev::Info, "MG Config file: {}", ModuleSettings.ConfigFile);
  mg_config = Multigrid::Config(ModuleSettings.ConfigFile);

  LOG(INIT, Sev::Info, "Multigrid Config\n{}", mg_config.debug());
  if (ModuleSettings.monitor) {
    monitor = Monitor(EFUSettings.KafkaBroker, "CSPEC", "multigrid");
    monitor.init_hits(readout_entries);
    monitor.init_histograms(std::numeric_limits<uint16_t>::max());
  }
  return true;
}

void MultigridBase::process_events(EV42Serializer &ev42serializer) {

  Counters.hits_time_seq_err = mg_config.reduction.stats.time_seq_errors;
  Counters.hits_bad_plane = mg_config.reduction.stats.invalid_planes;
  Counters.wire_clusters = mg_config.reduction.stats.wire_clusters;
  Counters.grid_clusters = mg_config.reduction.stats.grid_clusters;
  Counters.events_total = mg_config.reduction.stats.events_total;
  Counters.events_multiplicity_rejects = mg_config.reduction.stats.events_multiplicity_rejects;
  Counters.hits_used = mg_config.reduction.stats.hits_used;
  Counters.events_bad = mg_config.reduction.stats.events_bad;
  Counters.events_geometry_err = mg_config.reduction.stats.events_geometry_err;

  for (auto &event : mg_config.reduction.out_queue) {

    if (event.pixel_id == 0) {
      Counters.pulses++;

      if (HavePulseTime) {
        uint64_t PulsePeriod = event.time - ev42serializer.pulseTime();
        ShortestPulsePeriod = std::min(ShortestPulsePeriod, PulsePeriod);
      }
      HavePulseTime = true;

      if (ev42serializer.eventCount())
        Counters.tx_bytes += ev42serializer.produce();
      ev42serializer.pulseTime(event.time);

//            XTRACE(PROCESS, DEB, "New pulse time: %u   shortest pulse period: %u",
//                   ev42serializer.pulseTime(), ShortestPulsePeriod);
    } else {

      auto time = static_cast<uint32_t>(event.time - ev42serializer.pulseTime());
//            XTRACE(PROCESS, DEB, "Event: pixel: %d, time: %d ", pixel, time);
      if (!HavePulseTime || (event.time < ev42serializer.pulseTime())) {
        XTRACE(PROCESS, DEB, "Event before pulse");
        Counters.events_time_err++;
      } else if (time > (1.00004 * ShortestPulsePeriod)) {
        XTRACE(PROCESS, DEB, "Event out of pulse range");
        Counters.events_time_err++;
      } else {
//              XTRACE(PROCESS, DEB, "Event good");
        Counters.tx_events++;
        Counters.tx_bytes += ev42serializer.addEvent(time, event.pixel_id);
      }
    }
  }
  mg_config.reduction.out_queue.clear();
}

void MultigridBase::mainThread() {
  if (!init_config())
    return;

  /// Connection setup
  Socket::Endpoint
      local(EFUSettings.DetectorAddress.c_str(), EFUSettings.DetectorPort); //Change name or add more comments
  XTRACE(INIT, DEB, "server: %s, port %d", EFUSettings.DetectorAddress.c_str(), EFUSettings.DetectorPort);
  UDPReceiver cspecdata(local);
  cspecdata.setBufferSizes(EFUSettings.TxSocketBufferSize,
                           EFUSettings.RxSocketBufferSize);
  cspecdata.checkRxBufferSizes(EFUSettings.RxSocketBufferSize);
  cspecdata.printBufferSizes();
  cspecdata.setRecvTimeout(0, one_tenth_second_usecs); /// secs, usecs

  Producer event_producer(EFUSettings.KafkaBroker, "C-SPEC_detector");
  auto Produce = [&event_producer](auto DataBuffer, auto Timestamp) {
    event_producer.produce(DataBuffer, Timestamp);
  };

  EV42Serializer ev42serializer(kafka_buffer_size, "multigrid", Produce);

  ev42serializer.pulseTime(0);

  uint8_t buffer[eth_buffer_size];

  TSCTimer report_timer;
  while (true) {
    ssize_t ReadSize{0};
    if ((ReadSize = cspecdata.receive(buffer, eth_buffer_size)) > 0) {
      Counters.rx_packets++;
      Counters.rx_bytes += ReadSize;
//      XTRACE(PROCESS, DEB, "Processed UDP packet of size: %d", ReadSize);

      mg_config.builder->parse(Buffer<uint8_t>(buffer, static_cast<size_t>(ReadSize)));

      Counters.readouts_total = mg_config.builder->stats_readouts_total;
      Counters.parser_discarded_bytes = mg_config.builder->stats_discarded_bytes;
      Counters.parser_triggers = mg_config.builder->stats_trigger_count;
      Counters.builder_glitch_rejects = mg_config.builder->stats_bus_glitch_rejects;
      Counters.builder_filter_rejects = mg_config.builder->stats_readout_filter_rejects;
      Counters.builder_geometry_errors = mg_config.builder->stats_digital_geom_errors;

      if (!mg_config.builder->ConvertedData.empty()) {
        Counters.hits_total += mg_config.builder->ConvertedData.size();

        if (monitor.hit_serializer || monitor.histograms) {
          Hit transformed;
          for (const auto &hit : mg_config.builder->ConvertedData) {
            transformed = mg_config.mappings.absolutify(hit);
            if (monitor.hit_serializer)
              monitor.hit_serializer->addEntry(transformed.plane + 1,
                                               transformed.coordinate, transformed.time,
                                               transformed.weight);
            if (monitor.histograms) {
              if (transformed.plane == Multigrid::wire_plane)
                monitor.histograms->bin_x(transformed.coordinate, transformed.weight);
              else if (transformed.plane == Multigrid::grid_plane)
                monitor.histograms->bin_y(transformed.coordinate, transformed.weight);
            }
          }
        }
        mg_config.reduction.ingest(mg_config.builder->ConvertedData);

        mg_config.reduction.process_queues(false);
        process_events(ev42serializer);
      }
    } else {
      Counters.rx_idle++;
    }

    /// Force periodic flushing
    if (report_timer.timetsc() >= EFUSettings.UpdateIntervalSec * 1000000 * TscMHz) {
      Counters.tx_bytes += ev42serializer.produce();
      monitor.produce_now();

      /// Kafka stats update - common to all detectors
      /// don't increment as producer keeps absolute count
      Counters.kafka_produce_fails = event_producer.stats.produce_fails;
      Counters.kafka_ev_errors = event_producer.stats.ev_errors;
      Counters.kafka_ev_others = event_producer.stats.ev_others;
      Counters.kafka_dr_errors = event_producer.stats.dr_errors;
      Counters.kafka_dr_noerrors = event_producer.stats.dr_noerrors;

      report_timer.now();
    }

    /// Checking for exit
    if (not runThreads) {
      LOG(PROCESS, Sev::Info, "Stop requested. Flushing queues.");
      /// flush anything that remains

      LOG(PROCESS, Sev::Info, "Pipeline status\n" + mg_config.reduction.status("", false));

      mg_config.reduction.process_queues(true);
      process_events(ev42serializer);

      Counters.tx_bytes += ev42serializer.produce();
      monitor.produce_now();
      LOG(PROCESS, Sev::Info, "Pipeline flushed. Stopping processing thread.");
      return;
    }
  }
}
