/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <algorithm>
#include <common/EFUStats.h>
#include <time.h>

EFUStats::EFUStats() {
  statdb = new TCPClient("127.0.0.1", 2003);
  clear();
}

void EFUStats::clear() {
  std::fill_n((char *)&stats, sizeof(stats), 0);
  stats_old = stats;
  usecs_elapsed.now();
}

void EFUStats::set_mask(unsigned int mask) { report_mask = mask; }


void EFUStats::report() {
  if (report_mask)
    printf("%5" PRIu64, runtime.timeus() / 1000000);

  if (report_mask & 0x01)
    packet_stats();

  if (report_mask & 0x02)
    event_stats();

  if (report_mask & 0x04)
    fifo_stats();

  if (report_mask)
    printf("\n");

  sendstats();

    stats_old = stats;
    usecs_elapsed.now();
}

void EFUStats::sendstats() {
  char buffer[1000];
  int unixtime = (int)time(NULL);

  int len = sprintf(buffer, "efu2.input.rx_bytes %" PRIu64 " %d\n", stats.rx_bytes, unixtime);
  statdb->senddata(buffer, len);

  len = sprintf(buffer, "efu2.input.rx_packets %" PRIu64 " %d\n", stats.rx_packets, unixtime);
  statdb->senddata(buffer, len);

  len = sprintf(buffer, "efu2.input.i2pfifo_dropped %" PRIu64 " %d\n", stats.fifo1_push_errors, unixtime);
  statdb->senddata(buffer, len);

  len = sprintf(buffer, "efu2.input.i2pfifo_free %" PRIu64 " %d\n", stats.fifo1_free, unixtime);
  statdb->senddata(buffer, len);
  //
  //
  len = sprintf(buffer, "efu2.processing.rx_readouts %" PRIu64 " %d\n", stats.rx_readouts, unixtime);
  statdb->senddata(buffer, len);

  len = sprintf(buffer, "efu2.processing.rx_error_bytes %" PRIu64 " %d\n", stats.rx_error_bytes, unixtime);
  statdb->senddata(buffer, len);

  len = sprintf(buffer, "efu2.processing.rx_discards %" PRIu64 " %d\n", stats.rx_discards, unixtime);
  statdb->senddata(buffer, len);

  len = sprintf(buffer, "efu2.processing.rx_idle %" PRIu64 " %d\n", stats.rx_idle1, unixtime);
  statdb->senddata(buffer, len);

  len = sprintf(buffer, "efu2.processing.geom_err %" PRIu64 " %d\n", stats.geometry_errors, unixtime);
  statdb->senddata(buffer, len);

  len = sprintf(buffer, "efu2.processing.p2ofifo_dropped %" PRIu64 " %d\n", stats.fifo2_push_errors, unixtime);
  statdb->senddata(buffer, len);

  len = sprintf(buffer, "efu2.processing.p2ofifo_free %" PRIu64 " %d\n", stats.fifo2_free, unixtime);
  statdb->senddata(buffer, len);
  //
  //
  len = sprintf(buffer, "efu2.output.rx_events %" PRIu64 " %d\n", stats.rx_events, unixtime);
  statdb->senddata(buffer, len);

  len = sprintf(buffer, "efu2.output.rx_idle %" PRIu64 " %d\n", stats.rx_idle2, unixtime);
  statdb->senddata(buffer, len);

  len = sprintf(buffer, "efu2.output.tx_bytes %" PRIu64 " %d\n", stats.tx_bytes, unixtime);
  statdb->senddata(buffer, len);
}


void EFUStats::packet_stats() {
    auto usecs = usecs_elapsed.timeus();
    uint64_t ipps = (stats.rx_packets - stats_old.rx_packets) * 1000000 / usecs;
    uint64_t iMbps = 8 * (stats.rx_bytes - stats_old.rx_bytes) / usecs;
    uint64_t pkeps = (stats.rx_readouts - stats_old.rx_readouts) * 1000 / usecs;
    uint64_t oMbps = 8 * (stats.tx_bytes - stats_old.tx_bytes) / usecs;

    printf(" | I - %12" PRIu64 " B, %8" PRIu64 " pkt/s, %5" PRIu64 " Mb/s"
           " | P - %5" PRIu64 " kev/s"
           " | O - %5" PRIu64 " Mb/s",
           stats.rx_bytes, ipps, iMbps, pkeps, oMbps);
  }

void EFUStats::event_stats() {
    auto usecs = usecs_elapsed.timeus();
    uint64_t pkeps = (stats.rx_readouts - stats_old.rx_readouts) * 1000 / usecs;

    printf(" | I - %12" PRIu64 " pkts"
           " | P - %12" PRIu64 " events, %8" PRIu64 " kev/s, %12" PRIu64
           " discards, %12" PRIu64 " pix_err, %12" PRIu64 " errors",
           stats.rx_packets, stats.rx_readouts, pkeps, stats.rx_discards, stats.geometry_errors,
           stats.rx_error_bytes);
  }

void EFUStats::fifo_stats() {
    printf(" | Fifo I - %6" PRIu64 " free, %10" PRIu64 " pusherr"
           " | P - %12" PRIu64 " fifo free, %10" PRIu64 " pusherr",
           stats.fifo1_free, stats.fifo1_push_errors, stats.fifo2_free, stats.fifo2_push_errors);
  }