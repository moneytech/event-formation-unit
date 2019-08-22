// Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Serialize neutron events by putting them into a flatbuffer.
///
//===----------------------------------------------------------------------===//
#include "EventSerializer.h"
#include <dtdb_adc_pulse_debug_generated.h>
#include <ev42_events_generated.h>
#include <flatbuffers/flatbuffers.h>
#include <vector>
#include <limits>

EventSerializer::EventSerializer(std::string SourceName, size_t BufferSize,
                                 std::chrono::milliseconds TransmitTimeout,
                                 ProducerBase *KafkaProducer, TimestampMode Mode)
    : Name(std::move(SourceName)), Timeout(TransmitTimeout), EventBufferSize(BufferSize),
      Producer(KafkaProducer), CMode(Mode) {
  SerializeThread = std::thread(&EventSerializer::serialiseFunction, this);
}

EventSerializer::~EventSerializer() {
  RunThread.store(false);
  if (SerializeThread.joinable()) {
    SerializeThread.join();
  }
}

void EventSerializer::addEvent(std::unique_ptr<EventData> Event) {
  EventQueue.enqueue(std::move(Event));
}

void EventSerializer::addReferenceTimestamp(std::uint64_t const Timestamp) {
  if (CMode != TimestampMode::TIME_REFERENCED) {
    return;
  }
  ReferenceTimeQueue.enqueue(Timestamp);
}

struct FBVector {
  FBVector(flatbuffers::FlatBufferBuilder &Builder, size_t MaxEvents) {
    Offset = Builder.CreateUninitializedVector(MaxEvents, sizeof(std::uint32_t),
                                               &DataPtr);
    Data = nonstd::span<std::uint32_t>(
        reinterpret_cast<std::uint32_t *>(DataPtr), MaxEvents);
    SizePtr = reinterpret_cast<flatbuffers::uoffset_t *>(
                  const_cast<std::uint8_t *>(DataPtr)) -
              1;
    *SizePtr = 0;
  }
  void clear() { *SizePtr = 0; }
  void push_back(std::uint32_t Value) {
    Data[*SizePtr] = Value;
    (*SizePtr)++;
  }
  flatbuffers::uoffset_t Offset{0};
  std::uint8_t *DataPtr{nullptr};
  nonstd::span<std::uint32_t> Data;
  flatbuffers::uoffset_t *SizePtr{nullptr};
};

using namespace std::chrono_literals;

struct EventTimestamps {
  std::uint64_t EventTimestamp;
  std::uint64_t ThresholdTimestamp;
  std::uint64_t PeakTimestamp;

};

void EventSerializer::serialiseFunction() {
  flatbuffers::FlatBufferBuilder Builder(sizeof(std::uint32_t) * EventBufferSize * 8 + 2048);
  // We do not want the buffer to be too small or the vector offset addresses will become invalid when creating them.
  auto SourceNameOffset = Builder.CreateString(Name);
  auto TimeOffset = FBVector(Builder, EventBufferSize);
  auto EventId = FBVector(Builder, EventBufferSize);
  auto Amplitude = FBVector(Builder, EventBufferSize);
  auto PeakArea = FBVector(Builder, EventBufferSize);
  auto Background = FBVector(Builder, EventBufferSize);
  auto ThresholdTime = FBVector(Builder, EventBufferSize);
  auto PeakTime = FBVector(Builder, EventBufferSize);

  auto AdcPulseDebugOffset = CreateAdcPulseDebug(
      Builder, Amplitude.Offset, PeakArea.Offset, Background.Offset,
      ThresholdTime.Offset, PeakTime.Offset);
  auto FBMessage = CreateEventMessage(
      Builder, SourceNameOffset, 1, 1, TimeOffset.Offset, EventId.Offset,
      FacilityData::AdcPulseDebug, AdcPulseDebugOffset.Union());
  FinishEventMessageBuffer(Builder, FBMessage);
  auto EventMessage = GetMutableEventMessage(Builder.GetBufferPointer());

  std::unique_ptr<EventData> NewEvent;
  std::uint64_t MessageId{0};
  std::uint32_t NumberOfEvents{0};
  std::uint64_t CurrentRefTime{0};
  std::chrono::system_clock::time_point FirstEventTime;
  auto ProduceFB = [&]() {
    EventMessage->mutate_message_id(MessageId++);
    EventMessage->mutate_pulse_time(CurrentRefTime);
    Producer->produce({Builder.GetBufferPointer(), Builder.GetSize()},
                      CurrentRefTime / 1000000);
    NumberOfEvents = 0;
    TimeOffset.clear();
    EventId.clear();
    Amplitude.clear();
    PeakArea.clear();
    Background.clear();
    ThresholdTime.clear();
    PeakTime.clear();
  };
  do {
    while (EventQueue.try_dequeue(NewEvent)) {
      if (NumberOfEvents != 0) {
        if (NewEvent->Timestamp - CurrentRefTime > std::numeric_limits<std::uint32_t>::max()) {
          ProduceFB();
        }
      }

      if (NumberOfEvents == 0) {
        CurrentRefTime = NewEvent->Timestamp;
        FirstEventTime = getCurrentTime();
      }
      TimeOffset.push_back(
          static_cast<std::uint32_t>(NewEvent->Timestamp - CurrentRefTime));
      ThresholdTime.push_back(NewEvent->ThresholdTime);
      PeakTime.push_back(NewEvent->PeakTime);
      EventId.push_back(NewEvent->EventId);
      Amplitude.push_back(NewEvent->Amplitude);
      PeakArea.push_back(NewEvent->PeakArea);
      Background.push_back(NewEvent->Background);

      ++NumberOfEvents;
      if (NumberOfEvents == EventBufferSize) {
        ProduceFB();
      }
    }
    std::this_thread::sleep_for(5ms);
    if (NumberOfEvents > 0 and
        getCurrentTime() > FirstEventTime + Timeout) {
      ProduceFB();
    }
  } while (RunThread);
  if (NumberOfEvents > 0) {
    ProduceFB();
  }
}