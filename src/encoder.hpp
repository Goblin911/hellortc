#include <iostream>
#include <glog/logging.h>
#include <vector>
#include <x264.h>

class Encoder {
  public:
      bool encode(const char* y, const char* u, const char* v, std::vector<uint8_t>& out) {
        int luma_size = 1024 * 768;
        std::memcpy(_pic.img.plane[0], y, luma_size);
        std::memcpy(_pic.img.plane[1], u, luma_size);
        std::memcpy(_pic.img.plane[2], v, luma_size);
        x264_nal_t *nal;
        int i_nal;
        int i_frame_size = x264_encoder_encode(_h, &nal, &i_nal, &_pic, &_pic_out);
        CHECK(i_frame_size >= 0);
        std::vector<uint8_t> res(nal->p_payload, nal->p_payload + i_frame_size);
        out = res;
        return _pic_out.b_keyframe; 
    }

    void init() {
        // set params 
        CHECK(x264_param_default_preset(&_param, "ultrafast", "zerolatency") == 0);
        _param.i_csp = X264_CSP_I444;
        _param.i_width = 1024;
        _param.i_height = 768;
        _param.b_vfr_input = 0;
        _param.b_repeat_headers = 0;
        _param.b_annexb = 0;
        //CHECK(x264_param_apply_profile(&_param, "baseline") == 0);

        // alloc pic
        CHECK(x264_picture_alloc(&_pic, _param.i_csp, _param.i_width,
                                 _param.i_height) == 0);

        // open encoder
        _h = x264_encoder_open(&_param);
        CHECK(_h);
        LOG(INFO) << x264_encoder_maximum_delayed_frames(_h);
    }


  private:
    x264_t *_h;
    x264_param_t _param;
    x264_picture_t _pic;
    x264_picture_t _pic_out;
};
