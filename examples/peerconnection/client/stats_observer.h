// #ifndef WEBRTC_EXAMPLES_PEERCONNECTION_CLIENT_STATS_OBSERVER_H_
// #define WEBRTC_EXAMPLES_PEERCONNECTION_CLIENT_STATS_OBSERVER_H_

// #include "api/peer_connection_interface.h"
// #include "api/stats/rtc_stats_collector_callback.h"
// #include "rtc_base/logging.h"
// #include "api/stats/rtcstats_objects.h"

// class StatsObserver : public webrtc::RTCStatsCollectorCallback {
//  public:
//   // Factory method to create the observer
//   static rtc::scoped_refptr<StatsObserver> Create() {
//     return rtc::scoped_refptr<StatsObserver>(
//         new rtc::RefCountedObject<StatsObserver>());
//   }

//   void OnStatsDelivered(
//       const rtc::scoped_refptr<const webrtc::RTCStatsReport>& report) override {
//     RTC_LOG(LS_INFO) << "PeerConnection Stats Report received, timestamp: "
//                      << report->timestamp().ms();

//     // Process each stat
//     for (const auto& stat : *report) {
//       ProcessStat(stat);
//     }
//   }

//  private:
//   void ProcessStat(const webrtc::RTCStats& stat) {
//     const std::string type = stat.type();
//     if (type == "inbound-rtp") {
//       ProcessInboundRTPStats(stat);
//     } else if (type == "outbound-rtp") {
//       ProcessOutboundRTPStats(stat);
//     } else if (type == "candidate-pair") {
//       ProcessICECandidatePairStats(stat);
//     }
//   }

//   void ProcessInboundRTPStats(const webrtc::RTCStats& stat) {
//     // Get attributes using the new API
//     auto packetsReceived = stat.GetAttribute(stat.cast_to<webrtc::RTCInboundRtpStreamStats>().packets_received);
//     auto bytesReceived = stat.GetAttribute(stat.cast_to<webrtc::RTCInboundRtpStreamStats>().bytes_received);
//     auto packetsLost = stat.GetAttribute(stat.cast_to<webrtc::RTCInboundRtpStreamStats>().packets_lost);
//     auto jitter = stat.GetAttribute(stat.cast_to<webrtc::RTCInboundRtpStreamStats>().jitter);
    
//     RTC_LOG(LS_INFO) << "Inbound RTP Stats:"
//                      << "\n  Packets Received: " 
//                      << (packetsReceived.has_value() ? packetsReceived.value() : 0)
//                      << "\n  Bytes Received: " 
//                      << (bytesReceived.has_value() ? bytesReceived.value() : 0)
//                      << "\n  Packets Lost: " 
//                      << (packetsLost.has_value() ? packetsLost.value() : 0)
//                      << "\n  Jitter: " 
//                      << (jitter.has_value() ? jitter.value() : 0);

//     // Get video-specific attributes
//     auto kind = stat.GetAttribute(stat.cast_to<webrtc::RTCInboundRtpStreamStats>().kind);
//     if (kind.has_value() && kind.value() == "video") {
//       auto framesDecoded = stat.GetAttribute(stat.cast_to<webrtc::RTCInboundRtpStreamStats>().frames_decoded);
//       auto frameWidth = stat.GetAttribute(stat.cast_to<webrtc::RTCInboundRtpStreamStats>().frame_width);
//       auto frameHeight = stat.GetAttribute(stat.cast_to<webrtc::RTCInboundRtpStreamStats>().frame_height);
      
//       RTC_LOG(LS_INFO) << "  Frames Decoded: " 
//                        << (framesDecoded.has_value() ? framesDecoded.value() : 0)
//                        << "\n  Frame Width: " 
//                        << (frameWidth.has_value() ? frameWidth.value() : 0)
//                        << "\n  Frame Height: " 
//                        << (frameHeight.has_value() ? frameHeight.value() : 0);
//     }
//   }

//   void ProcessOutboundRTPStats(const webrtc::RTCStats& stat) {
//     auto packetsSent = stat.GetAttribute(stat.cast_to<webrtc::RTCOutboundRtpStreamStats>().packets_sent);
//     auto bytesSent = stat.GetAttribute(stat.cast_to<webrtc::RTCOutboundRtpStreamStats>().bytes_sent);
    
//     RTC_LOG(LS_INFO) << "Outbound RTP Stats:"
//                      << "\n  Packets Sent: " 
//                      << (packetsSent.has_value() ? packetsSent.value() : 0)
//                      << "\n  Bytes Sent: " 
//                      << (bytesSent.has_value() ? bytesSent.value() : 0);

//     auto kind = stat.GetAttribute(stat.cast_to<webrtc::RTCOutboundRtpStreamStats>().kind);
//     if (kind.has_value() && kind.value() == "video") {
//       auto framesEncoded = stat.GetAttribute(stat.cast_to<webrtc::RTCOutboundRtpStreamStats>().frames_encoded);
//       auto frameWidth = stat.GetAttribute(stat.cast_to<webrtc::RTCOutboundRtpStreamStats>().frame_width);
//       auto frameHeight = stat.GetAttribute(stat.cast_to<webrtc::RTCOutboundRtpStreamStats>().frame_height);
      
//       RTC_LOG(LS_INFO) << "  Frames Encoded: " 
//                        << (framesEncoded.has_value() ? framesEncoded.value() : 0)
//                        << "\n  Frame Width: " 
//                        << (frameWidth.has_value() ? frameWidth.value() : 0)
//                        << "\n  Frame Height: " 
//                        << (frameHeight.has_value() ? frameHeight.value() : 0);
//     }
//   }

//   void ProcessICECandidatePairStats(const webrtc::RTCStats& stat) {
//     auto state = stat.GetAttribute(stat.cast_to<webrtc::RTCIceCandidatePairStats>().state);
//     auto bytesSent = stat.GetAttribute(stat.cast_to<webrtc::RTCIceCandidatePairStats>().bytes_sent);
//     auto bytesReceived = stat.GetAttribute(stat.cast_to<webrtc::RTCIceCandidatePairStats>().bytes_received);
//     auto currentRoundTripTime = stat.GetAttribute(stat.cast_to<webrtc::RTCIceCandidatePairStats>().current_round_trip_time);

//     if (state.has_value()) {
//       RTC_LOG(LS_INFO) << "ICE Candidate Pair Stats:"
//                        << "\n  State: " << state.value()
//                        << "\n  Current Round Trip Time: " 
//                        << (currentRoundTripTime.has_value() ? currentRoundTripTime.value() : 0)
//                        << "\n  Total Bytes Sent: " 
//                        << (bytesSent.has_value() ? bytesSent.value() : 0)
//                        << "\n  Total Bytes Received: " 
//                        << (bytesReceived.has_value() ? bytesReceived.value() : 0);
//     }
//   }
// };

// // Helper function to periodically collect stats
// void StartStatsCollection(
//     rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection,
//     int interval_ms = 1000) {
//   rtc::scoped_refptr<StatsObserver> stats_observer = StatsObserver::Create();
  
//   rtc::Thread::Current()->PostDelayedTask(
//       [peer_connection, stats_observer, interval_ms]() {
//         peer_connection->GetStats(stats_observer.get());
//         // Schedule the next collection
//         StartStatsCollection(peer_connection, interval_ms);
//       },
//       webrtc::TimeDelta::Millis(interval_ms));
// }

// #endif  // WEBRTC_EXAMPLES_PEERCONNECTION_CLIENT_STATS_OBSERVER_H_

#ifndef WEBRTC_EXAMPLES_PEERCONNECTION_CLIENT_STATS_OBSERVER_H_
#define WEBRTC_EXAMPLES_PEERCONNECTION_CLIENT_STATS_OBSERVER_H_

#include "api/peer_connection_interface.h"
#include "api/stats/rtc_stats_collector_callback.h"
#include "rtc_base/logging.h"
#include "api/stats/rtcstats_objects.h"

class StatsObserver : public webrtc::RTCStatsCollectorCallback {
 public:
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
    } else if (type == "outbound-rtp") {
      ProcessOutboundRTPStats(stat);
    } else if (type == "candidate-pair") {
      ProcessICECandidatePairStats(stat);
    }
  }

  template<typename T>
  T GetAttributeValueOr(const webrtc::Attribute& attr, T defaultValue) {
    return attr.has_value() ? attr.get<T>() : defaultValue;
  }

  void ProcessInboundRTPStats(const webrtc::RTCStats& stat) {
    const auto& inbound = stat.cast_to<webrtc::RTCInboundRtpStreamStats>();
    auto packetsReceived = stat.GetAttribute(inbound.packets_received);
    auto bytesReceived = stat.GetAttribute(inbound.bytes_received);
    auto packetsLost = stat.GetAttribute(inbound.packets_lost);
    auto jitter = stat.GetAttribute(inbound.jitter);
    
    RTC_LOG(LS_INFO) << "Inbound RTP Stats:"
                     << "\n  Packets Received: " 
                     << GetAttributeValueOr(packetsReceived, 0UL)
                     << "\n  Bytes Received: " 
                     << GetAttributeValueOr(bytesReceived, 0UL)
                     << "\n  Packets Lost: " 
                     << GetAttributeValueOr(packetsLost, 0)
                     << "\n  Jitter: " 
                     << GetAttributeValueOr(jitter, 0.0);

    auto kind = stat.GetAttribute(inbound.kind);
    if (kind.has_value() && kind.get<std::string>() == "video") {
      auto framesDecoded = stat.GetAttribute(inbound.frames_decoded);
      auto frameWidth = stat.GetAttribute(inbound.frame_width);
      auto frameHeight = stat.GetAttribute(inbound.frame_height);
      
      RTC_LOG(LS_INFO) << "  Frames Decoded: " 
                       << GetAttributeValueOr(framesDecoded, 0U)
                       << "\n  Frame Width: " 
                       << GetAttributeValueOr(frameWidth, 0U)
                       << "\n  Frame Height: " 
                       << GetAttributeValueOr(frameHeight, 0U);
    }
  }

  void ProcessOutboundRTPStats(const webrtc::RTCStats& stat) {
    const auto& outbound = stat.cast_to<webrtc::RTCOutboundRtpStreamStats>();
    auto packetsSent = stat.GetAttribute(outbound.packets_sent);
    auto bytesSent = stat.GetAttribute(outbound.bytes_sent);
    
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