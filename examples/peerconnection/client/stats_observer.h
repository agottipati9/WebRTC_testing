#ifndef WEBRTC_EXAMPLES_PEERCONNECTION_CLIENT_STATS_OBSERVER_H_
#define WEBRTC_EXAMPLES_PEERCONNECTION_CLIENT_STATS_OBSERVER_H_

#include "api/peer_connection_interface.h"
#include "api/stats/rtc_stats_collector_callback.h"
#include "rtc_base/logging.h"
#include "api/stats/rtcstats_objects.h"

class StatsObserver : public webrtc::RTCStatsCollectorCallback {
 public:
  double previous_timestamp = 0.0;
  double previous_bytes_received = 0.0;

  static rtc::scoped_refptr<StatsObserver> Create() {
    return rtc::scoped_refptr<StatsObserver>(
        new rtc::RefCountedObject<StatsObserver>());
  }

  void OnStatsDelivered(
      const rtc::scoped_refptr<const webrtc::RTCStatsReport>& report) override {
    RTC_LOG(LS_INFO) << "PeerConnection Stats Report received, timestamp: "
                     << report->timestamp().ms();

    for (const auto& stat : *report) {
      ProcessStat(stat);
    }
  }

 private:
  void ProcessStat(const webrtc::RTCStats& stat) {
    const std::string type = stat.type();
    if (type == "inbound-rtp") {
      ProcessInboundRTPStats(stat);
    } 
    // else if (type == "outbound-rtp") {
    //   ProcessOutboundRTPStats(stat);
    // } else if (type == "candidate-pair") {
    //   ProcessICECandidatePairStats(stat);
    // }
  }

  template<typename T>
  T GetAttributeValueOr(const webrtc::Attribute& attr, T defaultValue) {
    return attr.has_value() ? attr.get<T>() : defaultValue;
  }

  void ProcessInboundRTPStats(const webrtc::RTCStats& stat) {
    const auto& inbound = stat.cast_to<webrtc::RTCInboundRtpStreamStats>();

          RTC_LOG(LS_INFO) << "********LOGGING INBOUND TRANSPORT RTP STATS********";


 // transport metrics
      auto bytesReceived = stat.GetAttribute(inbound.bytes_received);
      auto packetsLost = stat.GetAttribute(inbound.packets_lost);
      auto jitter = stat.GetAttribute(inbound.jitter);
      auto lastPacketReceivedTS = stat.GetAttribute(inbound.last_packet_received_timestamp);

    // bitrate calculation
    if (bytesReceived.holds_alternative<uint64_t>() && 
        bytesReceived.has_value() && lastPacketReceivedTS.holds_alternative<double>() && lastPacketReceivedTS.has_value()) {
      double current_timestamp = lastPacketReceivedTS.get<double>();
      double current_bytes_received = bytesReceived.get<uint64_t>();
      double bitrate = std::max(0.0, (current_bytes_received - previous_bytes_received)) / 
          std::abs(current_timestamp - previous_timestamp) * 8 / 1000000;

        // RTC_LOG(LS_INFO) << "Inbound RTP Stats:"
        //                  << "\n  Bytes Received: " << current_bytes_received
        //                  << "\n  Timestamp: " << current_timestamp
        //                  << "\n  Previous Timestamp: " << previous_timestamp
        //                  << "\n  Timestamp Difference: " << current_timestamp - previous_timestamp
        //                  << "\n  Bitrate: " << bitrate << " Mbps";

        if (bitrate < 1e-6) { 
          RTC_LOG(LS_INFO) << "Receiving Rate: " << "NAN" << " Mbps";
        } else {
          RTC_LOG(LS_INFO) << "Receiving Rate: " << bitrate << " Mbps";  
        }
        previous_timestamp = current_timestamp;
        previous_bytes_received = current_bytes_received;
      } else {
        RTC_LOG(LS_INFO) << "Receiving Rate: 0 Mbps";
      }

      if (packetsLost.holds_alternative<uint32_t>() && 
          packetsLost.has_value()) {
        RTC_LOG(LS_INFO) << "Packets Lost: " << packetsLost.get<uint32_t>();
      } else {
        RTC_LOG(LS_INFO) << "Packets Lost: 0";
      }

      if (jitter.holds_alternative<double>() && 
          jitter.has_value()) {
        RTC_LOG(LS_INFO) << "Jitter: " << jitter.get<double>();
      } else {
        RTC_LOG(LS_INFO) << "Jitter: 0.0";
      }

    auto kind = stat.GetAttribute(inbound.kind);
    if (kind.has_value() && kind.get<std::string>() == "video") {
      RTC_LOG(LS_INFO) << "********LOGGING INBOUND VIDEO RTP STATS********";
      
      // video specific metrics
      auto framesPerSecond = stat.GetAttribute(inbound.frames_per_second);
      auto freezeCount = stat.GetAttribute(inbound.freeze_count);
      auto totalFreezeDuration = stat.GetAttribute(inbound.total_freezes_duration);
      auto totalInterFrameDelay = stat.GetAttribute(inbound.total_inter_frame_delay);

      if (framesPerSecond.holds_alternative<double>() && 
          framesPerSecond.has_value()) {
        RTC_LOG(LS_INFO) << "Frames per Second: " << framesPerSecond.get<double>();
      } else {
        RTC_LOG(LS_INFO) << "Frames per Second: NA";
      }

      if (freezeCount.holds_alternative<uint32_t>() && 
          freezeCount.has_value()) {
        RTC_LOG(LS_INFO) << "Freeze Rate: " << freezeCount.get<uint32_t>();
      } else {
        RTC_LOG(LS_INFO) << "Freeze Rate: 0";
      }

      if (totalFreezeDuration.holds_alternative<double>() && 
          totalFreezeDuration.has_value()) {
        RTC_LOG(LS_INFO) << "Total Freeze Duration: " << totalFreezeDuration.get<double>();
      } else {
        RTC_LOG(LS_INFO) << "Total Freeze Duration: 0.0";
      }

      if (totalInterFrameDelay.holds_alternative<double>() && 
          totalInterFrameDelay.has_value()) {
        RTC_LOG(LS_INFO) << "Total Inter Frame Delay: " << totalInterFrameDelay.get<double>();
      } else {
        RTC_LOG(LS_INFO) << "Total Inter Frame Delay: 0.0";
      }
    }
  }

  void ProcessOutboundRTPStats(const webrtc::RTCStats& stat) {
    const auto& outbound = stat.cast_to<webrtc::RTCOutboundRtpStreamStats>();
    auto packetsSent = stat.GetAttribute(outbound.packets_sent);
    auto bytesSent = stat.GetAttribute(outbound.bytes_sent);

    RTC_LOG(LS_INFO) << "********LOGGING OUTBOUND RTP STATS********";
    
    RTC_LOG(LS_INFO) << "Outbound RTP Stats:"
                     << "\n  Packets Sent: " 
                     << GetAttributeValueOr(packetsSent, 0UL)
                     << "\n  Bytes Sent: " 
                     << GetAttributeValueOr(bytesSent, 0UL);

    auto kind = stat.GetAttribute(outbound.kind);
    if (kind.has_value() && kind.get<std::string>() == "video") {
      auto framesEncoded = stat.GetAttribute(outbound.frames_encoded);
      auto frameWidth = stat.GetAttribute(outbound.frame_width);
      auto frameHeight = stat.GetAttribute(outbound.frame_height);
      
      RTC_LOG(LS_INFO) << "  Frames Encoded: " 
                       << GetAttributeValueOr(framesEncoded, 0U)
                       << "\n  Frame Width: " 
                       << GetAttributeValueOr(frameWidth, 0U)
                       << "\n  Frame Height: " 
                       << GetAttributeValueOr(frameHeight, 0U);
    }
  }

  void ProcessICECandidatePairStats(const webrtc::RTCStats& stat) {
    const auto& pair = stat.cast_to<webrtc::RTCIceCandidatePairStats>();
    auto state = stat.GetAttribute(pair.state);
    auto bytesSent = stat.GetAttribute(pair.bytes_sent);
    auto bytesReceived = stat.GetAttribute(pair.bytes_received);
    auto currentRoundTripTime = stat.GetAttribute(pair.current_round_trip_time);

    RTC_LOG(LS_INFO) << "********LOGGING ICE CANDIDATE PAIR STATS********";

    if (state.has_value()) {
      RTC_LOG(LS_INFO) << "ICE Candidate Pair Stats:"
                       << "\n  State: " << state.get<std::string>()
                       << "\n  Current Round Trip Time: " 
                       << GetAttributeValueOr(currentRoundTripTime, 0.0)
                       << "\n  Total Bytes Sent: " 
                       << GetAttributeValueOr(bytesSent, 0UL)
                       << "\n  Total Bytes Received: " 
                       << GetAttributeValueOr(bytesReceived, 0UL);
    }
  }
};

void StartStatsCollection(
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection,
    int interval_ms = 1000) {
  rtc::scoped_refptr<StatsObserver> stats_observer = StatsObserver::Create();
  
  rtc::Thread::Current()->PostDelayedTask(
      [peer_connection, stats_observer, interval_ms]() {
        peer_connection->GetStats(stats_observer.get());
        StartStatsCollection(peer_connection, interval_ms);
      },
      webrtc::TimeDelta::Millis(interval_ms));
}

#endif  // WEBRTC_EXAMPLES_PEERCONNECTION_CLIENT_STATS_OBSERVER_H_