/*
 *  Copyright 2012 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <glib.h>
#include <gtk/gtk.h>
#include <stdio.h>

#include "absl/flags/parse.h"
#include "api/scoped_refptr.h"
#include "examples/peerconnection/client/conductor.h"
#include "examples/peerconnection/client/flag_defs.h"
#include "examples/peerconnection/client/linux/main_wnd.h"
#include "examples/peerconnection/client/peer_connection_client.h"
#include "rtc_base/physical_socket_server.h"
#include "rtc_base/ssl_adapter.h"
#include "rtc_base/thread.h"
#include "system_wrappers/include/field_trial.h"
#include "test/field_trial.h"

class VideoRenderer : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
 public:
  VideoRenderer(webrtc::VideoTrackInterface* track_to_render,
                MainWndCallback* callback)
      : track_(track_to_render), callback_(callback) {
    track_->AddOrUpdateSink(this, rtc::VideoSinkWants());
  }
  ~VideoRenderer() { track_->RemoveSink(this); }
  void OnFrame(const webrtc::VideoFrame& frame) {
    callback_->OnFrameCallback(frame);
  }

 private:
  rtc::scoped_refptr<webrtc::VideoTrackInterface> track_;
  MainWndCallback* callback_;
};

class HeadlessClient : public MainWindow {
public:
    HeadlessClient(const char* server, int port, bool autoconnect, bool autocall) 
        : server_(server), port_(port), autoconnect_(autoconnect), 
          autocall_(autocall), callback_(nullptr) {}

    virtual void RegisterObserver(MainWndCallback* callback) override {
        callback_ = callback;
    }

    virtual void SwitchToConnectUI() override {
        if (autoconnect_) {
          callback_->StartLogin(server_, port_);
        }
    }

    virtual void SwitchToPeerList(const Peers& peers) override {
        if (autocall_ && !peers.empty()) {
            auto peer = peers.begin();
            callback_->ConnectToPeer(peer->first);
        }
    }

    // Implement other required MainWindow virtual functions with minimal logic
    virtual bool IsWindow() override { return true; }
    virtual void SwitchToStreamingUI() override {}
    virtual void MessageBox(const char* caption, const char* text, bool is_error) override { RTC_LOG(LS_INFO) << caption << ": " << text; }
    virtual MainWindow::UI current_ui() override { return STREAMING; }
    virtual void StartLocalRenderer(webrtc::VideoTrackInterface* local_video) override { local_renderer_.reset(new VideoRenderer(local_video, callback_)); }
    virtual void StopLocalRenderer() override { local_renderer_.reset(); }
    virtual void StartRemoteRenderer(webrtc::VideoTrackInterface* remote_video) override { remote_renderer_.reset(new VideoRenderer(remote_video, callback_)); }
    virtual void StopRemoteRenderer() override { remote_renderer_.reset();}
    virtual void QueueUIThreadCallback(int msg_id, void* data) override {
        callback_->UIThreadCallback(msg_id, data);
    }

private:
    std::string server_;
    int port_;
    bool autoconnect_;
    bool autocall_;
    MainWndCallback* callback_;
    std::unique_ptr<VideoRenderer> remote_renderer_;
    std::unique_ptr<VideoRenderer> local_renderer_;
};

class CustomHeadlessSocketServer : public rtc::PhysicalSocketServer {
 public:
  explicit CustomHeadlessSocketServer(HeadlessClient* wnd)
      : wnd_(wnd), conductor_(nullptr), client_(nullptr) {}
  virtual ~CustomHeadlessSocketServer() {}

  void SetMessageQueue(rtc::Thread* queue) override { message_queue_ = queue; }

  void set_client(PeerConnectionClient* client) { client_ = client; }
  void set_conductor(Conductor* conductor) { conductor_ = conductor; }

  bool Wait(webrtc::TimeDelta max_wait_duration, bool process_io) override {
    if (!wnd_->IsWindow() && !conductor_->connection_active() && client_ != nullptr && !client_->is_connected()) {
      message_queue_->Quit();
    }
    return rtc::PhysicalSocketServer::Wait(webrtc::TimeDelta::Zero(), process_io);
  }

 protected:
  rtc::Thread* message_queue_;
  HeadlessClient* wnd_;
  Conductor* conductor_;
  PeerConnectionClient* client_;
};



class CustomSocketServer : public rtc::PhysicalSocketServer {
 public:
  explicit CustomSocketServer(GtkMainWnd* wnd)
      : wnd_(wnd), conductor_(NULL), client_(NULL) {}
  virtual ~CustomSocketServer() {}

  void SetMessageQueue(rtc::Thread* queue) override { message_queue_ = queue; }

  void set_client(PeerConnectionClient* client) { client_ = client; }
  void set_conductor(Conductor* conductor) { conductor_ = conductor; }

  // Override so that we can also pump the GTK message loop.
  // This function never waits.
  bool Wait(webrtc::TimeDelta max_wait_duration, bool process_io) override {
    // Pump GTK events.
    // TODO(henrike): We really should move either the socket server or UI to a
    // different thread.  Alternatively we could look at merging the two loops
    // by implementing a dispatcher for the socket server and/or use
    // g_main_context_set_poll_func.
    while (gtk_events_pending())
      gtk_main_iteration();

    if (!wnd_->IsWindow() && !conductor_->connection_active() &&
        client_ != NULL && !client_->is_connected()) {
      message_queue_->Quit();
    }
    return rtc::PhysicalSocketServer::Wait(webrtc::TimeDelta::Zero(),
                                           process_io);
  }

 protected:
  rtc::Thread* message_queue_;
  GtkMainWnd* wnd_;
  Conductor* conductor_;
  PeerConnectionClient* client_;
};

// Run with GUI
int main(int argc, char* argv[]) {
  gtk_init(&argc, &argv);
// g_type_init API is deprecated (and does nothing) since glib 2.35.0, see:
// https://mail.gnome.org/archives/commits-list/2012-November/msg07809.html
#if !GLIB_CHECK_VERSION(2, 35, 0)
  g_type_init();
#endif
// g_thread_init API is deprecated since glib 2.31.0, see release note:
// http://mail.gnome.org/archives/gnome-announce-list/2011-October/msg00041.html
#if !GLIB_CHECK_VERSION(2, 31, 0)
  g_thread_init(NULL);
#endif

  absl::ParseCommandLine(argc, argv);

  // InitFieldTrialsFromString stores the char*, so the char array must outlive
  // the application.
  const std::string forced_field_trials =
      absl::GetFlag(FLAGS_force_fieldtrials);
  webrtc::field_trial::InitFieldTrialsFromString(forced_field_trials.c_str());

  // Abort if the user specifies a port that is outside the allowed
  // range [1, 65535].
  if ((absl::GetFlag(FLAGS_port) < 1) || (absl::GetFlag(FLAGS_port) > 65535)) {
    printf("Error: %i is not a valid port.\n", absl::GetFlag(FLAGS_port));
    return -1;
  }

  const std::string server = absl::GetFlag(FLAGS_server);

  bool is_caller = absl::GetFlag(FLAGS_autocall);

  GtkMainWnd wnd(server.c_str(), absl::GetFlag(FLAGS_port),
                true,
                absl::GetFlag(FLAGS_autocall),
                is_caller);
  wnd.Create();

  CustomSocketServer socket_server(&wnd);
  rtc::AutoSocketServerThread thread(&socket_server);

  rtc::InitializeSSL();
  // Must be constructed after we set the socketserver.
  PeerConnectionClient client;
  auto conductor = rtc::make_ref_counted<Conductor>(&client, &wnd);
  socket_server.set_client(&client);
  socket_server.set_conductor(conductor.get());

  thread.Run();

  // gtk_main();
  wnd.Destroy();

  rtc::CleanupSSL();
  return 0;
}


// // // Run in Headless
// int main(int argc, char* argv[]) {
// // g_type_init API is deprecated (and does nothing) since glib 2.35.0, see:
// // https://mail.gnome.org/archives/commits-list/2012-November/msg07809.html
// #if !GLIB_CHECK_VERSION(2, 35, 0)
//   g_type_init();
// #endif
// // g_thread_init API is deprecated since glib 2.31.0, see release note:
// // http://mail.gnome.org/archives/gnome-announce-list/2011-October/msg00041.html
// #if !GLIB_CHECK_VERSION(2, 31, 0)
//   g_thread_init(NULL);
// #endif

//   absl::ParseCommandLine(argc, argv);

//   // InitFieldTrialsFromString stores the char*, so the char array must outlive
//   // the application.
//   const std::string forced_field_trials =
//       absl::GetFlag(FLAGS_force_fieldtrials);
//   webrtc::field_trial::InitFieldTrialsFromString(forced_field_trials.c_str());

//   // Abort if the user specifies a port that is outside the allowed
//   // range [1, 65535].
//   if ((absl::GetFlag(FLAGS_port) < 1) || (absl::GetFlag(FLAGS_port) > 65535)) {
//     printf("Error: %i is not a valid port.\n", absl::GetFlag(FLAGS_port));
//     return -1;
//   }

//   const std::string server = absl::GetFlag(FLAGS_server);

//   HeadlessClient wnd(server.c_str(), absl::GetFlag(FLAGS_port), true, absl::GetFlag(FLAGS_autocall));

//   CustomHeadlessSocketServer socket_server(&wnd);
//   rtc::AutoSocketServerThread thread(&socket_server);
  
//   rtc::InitializeSSL();
  
//   PeerConnectionClient connection;
//   auto conductor = rtc::make_ref_counted<Conductor>(&connection, &wnd);
//   socket_server.set_conductor(conductor.get());
//   socket_server.set_client(&connection);

//   wnd.RegisterObserver(conductor.get());
//   wnd.SwitchToConnectUI();
  
//   thread.Run();
  
//   rtc::CleanupSSL();
//   return 0;
// }