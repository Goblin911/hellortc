#include "rtc/common.hpp"
#include "rtc/global.hpp"
#include "rtc.hpp"
#include "rtc/peerconnection.hpp"
#include <chrono>
#include <fstream>
#include <glog/logging.h>
#include <iostream>
#include <string>
#include <json/json.h>
#include <thread>
#include "encoder.hpp"



void test_encoder() {
    Encoder enc;
    enc.init();

    const int n = 1024 * 768;
    char* y = new char[n];
    char* u = new char[n];
    char* v = new char[n];

    FILE* fp = fopen("in.yuv", "rb");
    while (true) {
        if (fread(y, 1, n, fp) != n)
            break;
        if (fread(u, 1, n, fp) != n)
            break;
        if (fread(v, 1, n, fp) != n)
            break;
        LOG(INFO) << "encode....";
        std::vector<uint8_t> res;
        enc.encode(y, u, v, res);
        LOG(INFO) << "done....";
        break;
    }
}

int main(int argc, char *argv[]) {

    LOG(INFO) << "start.";

    RTC rtc;
    rtc.init();
    Encoder enc;
    enc.init();

    Json::Reader reader;
    Json::Value root;
    reader.parse(std::cin, root);
    rtc.set_remote_Desc(root["sdp"].asString(), root["type"].asString());
    rtc.waiting();
    std::string filename = "in.yuv";
    std::ifstream is(filename, std::ios::binary);
    if (!is.is_open()) {
        LOG(INFO) << "failed to open " << filename << '\n';
    }
    std::vector<uint32_t> blob;
    uint32_t b = 0;
    {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(10000ms);
    }


    FILE* fp = fopen("in.yuv", "rb");
    const int n = 1024 * 768;
    char* y = new char[n];
    char* u = new char[n];
    char* v = new char[n];
    while (true) {
        if (fread(y, 1, n, fp) != n)
            break;
        if (fread(u, 1, n, fp) != n)
            break;
        if (fread(v, 1, n, fp) != n)
            break;
        LOG(INFO) << "encode....";
        std::vector<uint8_t> res;
        enc.encode(y, u, v, res);
        LOG(INFO) << "done....";
        break;
    }




    for (int i = 0; is.read((char *)(&b), sizeof(b));) {
        if (blob.size() > 2 && b == 16777216) {
            if (i < 5) {
                ++i;
                blob.push_back(b);
                continue;
            }
 //           dc->send((std::byte *)(blob.data()), blob.size() * 4);
            LOG(INFO) << "============== debug =============";
            for (uint32_t i : blob) {
                std::cerr << i << " ";
            }
            std::cerr << std::endl;
            LOG(INFO) << "============== debug =============";
            blob.clear();
            // break;
        }
        blob.push_back(b);
    }
    LOG(INFO) << "end";
}
