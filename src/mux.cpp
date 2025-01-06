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
      static constexpr uint8_t key_mask = 0b00111111;
      static constexpr uint16_t io_dir = io0_dir << 8 | io1_dir;

      uint16_t frame_buffer = 0;

      struct KeyIntegrator {
         uint8_t previous[3];
         uint8_t current;
      } integrator[2] = {0};

      /// @brief Actual key being active (with filter and shift)
      uint8_t active_key = 0; // 1 to 13 (1-6 / 7 / 8-14)
      bool clear_nkeys = false;

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

      void integrate_keys(uint8_t side, uint8_t current) {
         KeyIntegrator &i = integrator[side];
         
         i.previous[0] = i.previous[1]; i.previous[1] = i.previous[2]; i.previous[2] = current;
         
         uint8_t debounced_on = (i.previous[0] | i.previous[1] | i.previous[2]);
         uint8_t debounced_off = (i.previous[0] & i.previous[1] & i.previous[2]);

         i.current = (i.current & debounced_on) | debounced_off;
      }

      auto on_i2c_ready(status_code_t code) {
         alert_and_stop_if(code != status_code_t::STATUS_OK);

         // If reading - integrate the keys
         if ( i2c_sequencer.is("get_left"_s) ) {
            integrate_keys(0, iomux_left.get_value() & key_mask);
         } else if ( i2c_sequencer.is("get_right"_s) ) {
            integrate_keys(1, iomux_right.get_value() & key_mask);

            // Regroup the keys on 1 bytes 6 left + door + shift
            bool shift = integrator[1].current & 0b010000;
            bool door = integrator[1].current & 0b100000;

            // Create a view of all the push buttons (not accounting for the shift key)
            uint8_t all_keys = (integrator[0].current & 0b111111) << 1 | (door?1:0);

            // Calculate active keys - factoring the shift key
            if ( all_keys != 0 ) {
               if ( clear_nkeys ) {
                  // Do nothing
               } else if ( active_key == 0 ) {
                  // Make sure only 1 bits set
                  if ( (all_keys & (all_keys - 1)) == 0 ) {
                     active_key = __builtin_ctzl(all_keys) + (shift?6:0) + 1;
                  }
               } else {
                  // The key remains valid if it is pushed (ignore the shift)
                  uint8_t key_bit = ( active_key < 6 ) ? active_key - 1 : active_key - 7;

                  if ( not (all_keys & key_bit) ) {  // No longer pressed
                     clear_nkeys = true;
                  }
               }
            } else {
               clear_nkeys = false;
               active_key = 0;
            }
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

      void set_leds(uint16_t value) {
         frame_buffer = value;
      }

      // Return the active key
      uint8_t get_active_key_code() {
         return active_key;
      }

      uint8_t get_switch_status() {
         return integrator[1].current & 0x0f;
      }
   }
}
