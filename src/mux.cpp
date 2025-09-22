/**
 * Handles the pin multiplexing
 * The bus is sampled every 2ms where the LEDs are updated and the keys sampled.
 * The key are consolidated using a 3 cycle integrator, then consolidated into a single key.
 */
#include <asx/pca9555.hpp>
#include <asx/ioport.hpp>

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

      static constexpr auto leds_per_side = uint8_t{6};
      static constexpr auto io_msk = uint8_t{0b00111111};
      static constexpr auto shift_msk = uint8_t{1U << 5};
      static constexpr auto door_msk = uint8_t{1U << 4};
      static constexpr auto pol_right_msk = uint8_t{0b1111}; // Flip the switch

      /// @brief Holds the current value for the LEDs
      uint8_t frame_buffer[] = {io_msk, io_msk};

      struct KeyIntegrator {
         uint8_t previous[3];
         uint8_t current;
      } integrator[2] = {0};

      /// @brief Actual key being active (with filter and shift)
      uint8_t active_key = 0; // 1 to 13 (1-6 / 7 / 8-14)
      bool clear_nkeys = false;

      auto iomux_left = PCA9555(0);
      auto iomux_right = PCA9555(1);

      // Handlers
      reactor::Handle react_on_poll;
      void on_i2c_ready(status_code_t code);

      struct InitPCA {
         auto operator()() {

            return make_transition_table(
               * "idle"_s          + event<start>     / [] {iomux_left.set_value<0>(io_msk, on_i2c_ready); }          = "init1"_s
               , "init1"_s         + event<i2c_ready> / [] {iomux_right.set_value<0>(io_msk, on_i2c_ready); }         = "init2"_s
               , "init2"_s         + event<i2c_ready> / [] {iomux_left.set_dir<0>(~io_msk, on_i2c_ready); }           = "init3"_s
               , "init3"_s         + event<i2c_ready> / [] {iomux_right.set_dir<0>(~io_msk, on_i2c_ready); }          = "init4"_s
               , "init4"_s         + event<i2c_ready> / [] {iomux_left.set_pol<1>(0, on_i2c_ready); }                 = "init5"_s
               , "init5"_s         + event<i2c_ready> / [] {iomux_right.set_pol<1>(pol_right_msk, on_i2c_ready);
                                                            react_on_poll.repeat(2ms); }                              = "wait_for_poll"_s
               , "wait_for_poll"_s + event<polling>   / [] {iomux_left.set_value<0>(frame_buffer[0], on_i2c_ready); } = "set_left"_s
               , "set_left"_s      + event<i2c_ready> / [] {iomux_right.set_value<0>(frame_buffer[1], on_i2c_ready); }= "set_right"_s
               , "set_right"_s     + event<i2c_ready> / [] {iomux_left.read<1>(on_i2c_ready); }                       = "get_left"_s
               , "get_left"_s      + event<i2c_ready> / [] {iomux_right.read<1>(on_i2c_ready); }                      = "get_right"_s
               , "get_right"_s     + event<i2c_ready>                                                                 = "wait_for_poll"_s
            );
         }
      };

      //
      i2c::Master::chain()

      sm<InitPCA> i2c_sequencer;

      void integrate_keys(uint8_t side, uint8_t current) {
         KeyIntegrator &i = integrator[side];

         i.previous[0] = i.previous[1]; i.previous[1] = i.previous[2]; i.previous[2] = current;

         uint8_t debounced_on = (i.previous[0] | i.previous[1] | i.previous[2]);
         uint8_t debounced_off = (i.previous[0] & i.previous[1] & i.previous[2]);

         i.current = (i.current & debounced_on) | debounced_off;
      }

      void on_i2c_ready(status_code_t code) {
         alert_and_stop_if(code != status_code_t::STATUS_OK);

         // If reading - integrate the keys
         if ( i2c_sequencer.is("get_left"_s) ) {
            integrate_keys(0, iomux_left.get_value<uint8_t>() & io_msk);
         } else if ( i2c_sequencer.is("get_right"_s) ) {
            integrate_keys(1, iomux_right.get_value<uint8_t>() & io_msk);

            // Regroup the keys on 1 bytes 6 left + door + shift
            bool shift = integrator[1].current & shift_msk;
            bool door = integrator[1].current & door_msk;

            // Create a view of all the push buttons (not accounting for the shift key)
            uint8_t all_keys = integrator[0].current | (door?0b1000000:0);

            // Calculate active keys - factoring the shift key
            if ( all_keys != 0 ) {
               if ( clear_nkeys ) {
                  // Do nothing
               } else if ( active_key == 0 ) {
                  // Make sure only 1 bits set
                  if ( (all_keys & (all_keys - 1)) == 0 ) {
                     active_key = __builtin_ctzl(all_keys) + (shift?7:0) + 1;
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
         react_on_poll = reactor::bind(on_poll_input);

         i2c::Master::init(400_KHz);
         i2c_sequencer.process_event(start{});
      }

      /// @brief Set the leds as one continuous buffer of 12 leds
      /// @param value 16 bits holder the 12bits
      void set_leds(uint16_t value) {
         frame_buffer[0] = (value>>6) & io_msk;
         frame_buffer[1] = value & io_msk;
      }

      /// @brief Get the leds as a 12-bits values
      uint16_t get_leds() {
         return frame_buffer[0]<<6 | frame_buffer[1];
      }

      bool get_led(uint8_t index) {
         if ( index < 6 ) {
            return (frame_buffer[0] >> index) & 1;
         }
         return (frame_buffer[1] >> (index-6)) & 1;
      }

      void set_led(uint8_t index, bool on) {
        uint8_t *frame = &frame_buffer[index / leds_per_side];
        uint8_t mask = 1 << (index % leds_per_side);
        if (on) {
            *frame |= mask;
        } else {
            *frame &= (~mask);
        }
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
