#include <asx/pca9555.hpp>
#include <boost/sml.hpp>

#include <alert.h>
#include "mux.hpp"

using namespace asx;
using namespace asx::i2c;
using namespace std::chrono;
using namespace boost::sml;

namespace console {
   namespace mux {
      struct start {};
      struct i2c_ready {};
      struct polling {};

      static constexpr uint8_t io0_dir = 0b11000000;
      static constexpr uint8_t io1_dir = 0b11111111;
      static constexpr uint16_t io_dir = io0_dir << 8 | io1_dir;

      uint16_t frame_buffer = 0;

      uint16_t integrator[3];
      uint16_t key_status;

      auto iomux_left = PCA9555(0);
      auto iomux_right = PCA9555(1);

      reactor::Handle react_on_i2c_ready;
      reactor::Handle react_on_poll;

      struct InitPCA {
         auto operator()() {

            return make_transition_table(
               *"idle"_s           + event<start>     / [] {iomux_left.set_dir(io_dir, react_on_i2c_ready); }                    = "init1"_s
               , "init1"_s         + event<i2c_ready> / [] {iomux_right.set_dir(io_dir, react_on_i2c_ready); }                   = "init2"_s
               , "init2"_s         + event<i2c_ready> / [] {iomux_left.set_pol(io_dir, react_on_i2c_ready); }                    = "init3"_s
               , "init3"_s         + event<i2c_ready> / [] {iomux_right.set_pol(io_dir, react_on_i2c_ready);
                                                            react_on_poll.repeat(2ms); }                                         = "wait_for_poll"_s
               , "wait_for_poll"_s + event<polling>   / [] {iomux_left.set_value(frame_buffer, react_on_i2c_ready); }        = "set_left"_s
               , "set_left"_s      + event<i2c_ready> / [] {iomux_right.set_value(frame_buffer << 8, react_on_i2c_ready); }    = "set_right"_s
               , "set_right"_s     + event<i2c_ready> / [] {iomux_left.read(react_on_i2c_ready); }                               = "get_left"_s
               , "get_left"_s      + event<i2c_ready> / [] {iomux_right.read(react_on_i2c_ready); }                              = "get_right"_s
               , "get_right"_s     + event<i2c_ready> = "wait_for_poll"_s
            );
         }
      };

      sm<InitPCA> i2c_sequencer;

      auto on_i2c_ready(status_code_t code) {
         alert_and_stop_if(code != status_code_t::STATUS_OK);

         if ( i2c_sequencer.is("get_left"_s) ) {
            uint16_t value = iomux_left.get_value();
         } else if ( i2c_sequencer.is("get_right"_s) ) {
            uint16_t value = iomux_right.get_value();
         }

         i2c_sequencer.process_event(i2c_ready{});
      }

      auto on_poll_input() {
         i2c_sequencer.process_event(polling{});
      }

      void init() {
         i2c::Master::init(400_KHz);

         react_on_i2c_ready = reactor::bind(on_i2c_ready);
         react_on_poll = reactor::bind(on_poll_input);

         i2c_sequencer.process_event(start{});
      }

      void set(uint16_t value) {
         frame_buffer = value;
      }
   }
}
