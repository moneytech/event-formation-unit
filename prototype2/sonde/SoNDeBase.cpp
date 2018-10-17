/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include <sonde/SoNDeBase.h>
#include <common/EV42Serializer.h>
#include <common/Producer.h>
#include <common/Trace.h>
#include <libs/include/Socket.h>
#include <libs/include/TSCTimer.h>
#include <sonde/ideas/Data.h>

SONDEIDEABase::SONDEIDEABase(BaseSettings const &settings, struct SoNDeSettings & localSettings)
     : Detector("SoNDe detector using IDEA readout", settings),
       SoNDeSettings(localSettings) {

  Stats.setPrefix("efu.sonde");

  XTRACE(INIT, ALW, "Adding stats");
  // clang-format off
  Stats.create("input.rx_packets",                mystats.rx_packets);
  Stats.create("input.rx_bytes",                  mystats.rx_bytes);
  Stats.create("input.dropped",                   mystats.fifo_push_errors);
  Stats.create("processing.idle",                 mystats.rx_idle1);
  Stats.create("processing.rx_events",            mystats.rx_events);
  Stats.create("processing.rx_geometry_errors",   mystats.rx_geometry_errors);
  Stats.create("processing.rx_seq_errors",        mystats.rx_seq_errors);
  Stats.create("output.tx_bytes",                 mystats.tx_bytes);
  /// \todo below stats are common to all detectors and could/should be moved
  Stats.create("kafka_produce_fails", mystats.kafka_produce_fails);
  Stats.create("kafka_ev_errors", mystats.kafka_ev_errors);
  Stats.create("kafka_ev_others", mystats.kafka_ev_others);
  Stats.create("kafka_dr_errors", mystats.kafka_dr_errors);
  Stats.create("kafka_dr_others", mystats.kafka_dr_noerrors);
  // clang-format on
  std::function<void()> inputFunc = [this]() { SONDEIDEABase::input_thread(); };
  Detector::AddThreadFunction(inputFunc, "input");

  std::function<void()> processingFunc = [this]() {
    SONDEIDEABase::processing_thread();
  };
  Detector::AddThreadFunction(processingFunc, "processing");

  XTRACE(INIT, ALW, "Creating %d SONDE Rx ringbuffers of size %d",
         eth_buffer_max_entries, eth_buffer_size);
  eth_ringbuf = new RingBuffer<eth_buffer_size>(
      eth_buffer_max_entries + 11); /** \todo testing workaround */
  assert(eth_ringbuf != 0);
}

void SONDEIDEABase::input_thread() {
  /** Connection setup */
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(),
                         EFUSettings.DetectorPort);
  UDPReceiver sondedata(local);
  sondedata.setBufferSizes(0, EFUSettings.DetectorRxBufferSize);
  sondedata.printBufferSizes();
  sondedata.setRecvTimeout(0, 100000); // secs, usecs, 1/10 second

  for (;;) {
    int rdsize;
    unsigned int eth_index = eth_ringbuf->getDataIndex();

    /** this is the processing step */
    eth_ringbuf->setDataLength(eth_index, 0);
    if ((rdsize = sondedata.receive(eth_ringbuf->getDataBuffer(eth_index),
                                    eth_ringbuf->getMaxBufSize())) > 0) {
      mystats.rx_packets++;
      mystats.rx_bytes += rdsize;
      eth_ringbuf->setDataLength(eth_index, rdsize);

      if (input2proc_fifo.push(eth_index) == false) {
        mystats.fifo_push_errors++;
      } else {
        eth_ringbuf->getNextBuffer();
      }
    }

    // Checking for exit
    if (not runThreads) {
      XTRACE(INPUT, ALW, "Stopping input thread.");
      return;
    }
  }
}

void SONDEIDEABase::processing_thread() {
  SoNDeGeometry geometry;

  IDEASData ideasdata(&geometry, SoNDeSettings.fileprefix);
  EV42Serializer flatbuffer(kafka_buffer_size, "SONDE");
  Producer eventprod(EFUSettings.KafkaBroker, "SKADI_detector");
  flatbuffer.setProducerCallback(
      std::bind(&Producer::produce2<uint8_t>, &eventprod, std::placeholders::_1));

  unsigned int data_index;

  TSCTimer produce_timer;
  while (1) {
    if ((input2proc_fifo.pop(data_index)) == false) {
      mystats.rx_idle1++;
      usleep(10);

    } else {

      auto len = eth_ringbuf->getDataLength(data_index);
      if (len == 0) {
        mystats.fifo_synch_errors++;
      } else {
        int events =
            ideasdata.parse_buffer(eth_ringbuf->getDataBuffer(data_index), len);

        mystats.rx_geometry_errors += ideasdata.errors;
        mystats.rx_events += ideasdata.events;
        mystats.rx_seq_errors = ideasdata.ctr_outof_sequence;

        if (events > 0) {
          for (int i = 0; i < events; i++) {
            XTRACE(PROCESS, DEB, "flatbuffer.addevent[i: %d](t: %d, pix: %d)",
                   i, ideasdata.data[i].time, ideasdata.data[i].pixel_id);
            mystats.tx_bytes += flatbuffer.addEvent(ideasdata.data[i].time,
                                                    ideasdata.data[i].pixel_id);
          }
        }

        if (produce_timer.timetsc() >=
            EFUSettings.UpdateIntervalSec * 1000000 * TSC_MHZ) {
          mystats.tx_bytes += flatbuffer.produce();

          /// Kafka stats update - common to all detectors
          /// don't increment as producer keeps absolute count
          mystats.kafka_produce_fails = eventprod.stats.produce_fails;
          mystats.kafka_ev_errors = eventprod.stats.ev_errors;
          mystats.kafka_ev_others = eventprod.stats.ev_others;
          mystats.kafka_dr_errors = eventprod.stats.dr_errors;
          mystats.kafka_dr_noerrors = eventprod.stats.dr_noerrors;
          produce_timer.now();
        }
      }
    }
    if (not runThreads) {
      XTRACE(INPUT, ALW, "Stopping input thread.");
      return;
    }
  }
}
