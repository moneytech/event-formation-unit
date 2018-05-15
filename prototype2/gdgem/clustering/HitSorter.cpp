#include <gdgem/clustering/HitSorter.h>
#include <algorithm>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

HitSorter::HitSorter(SRSTime time, SRSMappings chips, uint16_t ADCThreshold,
                           double maxTimeGap) :
    pTime(time), pChips(chips),
    pADCThreshold(ADCThreshold),
    hits(pTime, maxTimeGap) {

}

//====================================================================================================================
void HitSorter::insert(Readout readout) {

  double triggerTimestamp_ns =
      pTime.trigger_timestamp_ns(readout.srs_timestamp + readout.bonus_timestamp);
  if (old_trigger_timestamp_ns_ != triggerTimestamp_ns) {
    stats_trigger_count++;

    double delta_trigger_ns = triggerTimestamp_ns - old_trigger_timestamp_ns_;

    hits.subsequent_trigger(delta_trigger_ns <= pTime.trigger_period_ns());

    analyze();
  }
  old_trigger_timestamp_ns_ = triggerTimestamp_ns;

  // TODO: Move this to parser?
  if (readout.over_threshold || (readout.adc >= pADCThreshold)) {
    hits.store(pChips.get_plane(readout), pChips.get_strip(readout),
               readout.adc, pTime.chip_time_ns(readout.bcid, readout.tdc));
    // TODO: who adds chipTime + trigger time? queue?
  }
}

void HitSorter::flush() {
  //flush both buffers in queue
  // TODO: subsequent trigger? How do we know?
  analyze();
  analyze();
}

void HitSorter::analyze() {
  hits.sort_and_correct();
  if (clusterer)
    clusterer->cluster(hits.hits());
}

