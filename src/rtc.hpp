#include "rtc/common.hpp"
#include "rtc/peerconnection.hpp"
#include <glog/logging.h>
#include <iterator>
#include <json/json.h>
#include <memory>
#include <rtc/rtc.hpp>

class RTC {
  public:
    void init() {
    rtc::InitLogger(rtc::LogLevel::Info);
        _config = std::make_shared<rtc::Configuration>();;
        _pc = std::make_shared<rtc::PeerConnection>(*_config);

        _pc->onStateChange([this](rtc::PeerConnection::State state) {
            _status.store(state);
            LOG(INFO) << "State: " << state;
        });

        _pc->onGatheringStateChange(
            [this](rtc::PeerConnection::GatheringState state) {
                LOG(INFO) << "Gathering state: " << state;
                if (state == rtc::PeerConnection::GatheringState::Complete) {
                    auto description = _pc->localDescription();
                    Json::Value root;
                    root["type"] = description->typeString();
                    root["sdp"] = std::string(description.value());
                    _local_description = root.toStyledString(); // unsafe, but I'm lazy
                    LOG(INFO) << _local_description;
                }
            });

        _dc = _pc->createDataChannel("test");

        _dc->onOpen([]() { LOG(INFO) << "Open"; });

        _dc->onMessage([](std::variant<rtc::binary, rtc::string> message) {
            if (std::holds_alternative<rtc::string>(message)) {
                LOG(INFO) << "Received: " << std::get<rtc::string>(message);
            }
        });
        _pc->setLocalDescription();
    }

    std::string localDescription() {
        while (_local_description == "") {
            LOG(INFO) << "waiting local desc...";
            {
                using namespace std::chrono_literals;
                std::this_thread::sleep_for(100ms);
            }
        }
        return _local_description;
    }

    rtc::PeerConnection::State state() {
        return _status;
    }

    void waiting() {
        while (_status != rtc::PeerConnection::State::Connected) {
            {
                using namespace std::chrono_literals;
                std::this_thread::sleep_for(1000ms);
            }
            LOG(INFO) << "waiting connecting....";
        }

    }

    void set_remote_Desc(std::string sdp, std::string type = "answer") {
        _pc->setRemoteDescription(rtc::Description(sdp, type));
    }

    void send(rtc::binary&& data) {
        CHECK(_status == rtc::PeerConnection::State::Connected);
        _dc->send(std::move(data));
    }

    std::atomic<rtc::PeerConnection::State> _status;
    std::string _local_description;
    std::shared_ptr<rtc::Configuration> _config;
    std::shared_ptr<rtc::PeerConnection> _pc;
    std::shared_ptr<rtc::DataChannel> _dc;
};
