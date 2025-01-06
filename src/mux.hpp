#pragma once

/// API for managing the I/O expanders

namespace console {
   namespace mux {
      void init();

      // Set the LEDs values
      void set_leds(uint16_t value);
      
      ///< Get the actively pushed key (stabilized, accounting for shift value)
      uint8_t get_active_key_code();
      
      ///< Get the switches status
      uint8_t get_switch_status();
   }
}
