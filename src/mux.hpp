#pragma once

/// API for managing the I/O expanders

namespace console {
   namespace mux {
      void init();
      // Set the LEDs values
      void set(uint16_t value);
      // Get the switches stabalized value
      uint16_t get();
   }
}
