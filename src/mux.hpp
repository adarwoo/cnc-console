#pragma once

/// API for managing the I/O expanders

namespace console {
   namespace mux {
      void init();

      // Set the LEDs values as a bloc of 12 bits
      void set_leds(uint16_t value);

      // Get the LEDs values as a bloc of 12 bits
      uint16_t get_leds();

      // Get a LED value
      bool get_led(uint8_t index);

      // Set a single LED
      void set_led(uint8_t index, bool on);

      ///< Get the actively pushed key (stabilized, accounting for shift value)
      uint8_t get_active_key_code();

      ///< Get the switches status
      uint8_t get_switch_status();
   }
}
