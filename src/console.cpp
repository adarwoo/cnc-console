/**
 * Handles all modbus requests
 * From setting the leds, reading the switch or the active key, or sounding the buzzer
 */
#include <piezzo.h>

#include "console.hpp"
#include "mux.hpp"

using namespace asx;

namespace console
{
    /// @brief  Read 4 bits for the switch
    void on_get_sw_status(uint8_t addr, uint8_t qty) {
       // Validate quantity against the available number of LEDs
       if (addr + qty > 4) {
           Datagram::reply_error(modbus::error_t::illegal_data_value);
           return;
       }

       Datagram::pack<uint8_t>(1);  // Number of bytes
       Datagram::pack<uint8_t>((mux::get_switch_status() >> addr) & ((1 << qty) - 1));
    }

    /// @brief  Get the currently pushed active key
    void on_get_active_key() {
        Datagram::set_size(2);
        Datagram::pack<uint8_t>(2);
        Datagram::pack<uint16_t>(mux::get_active_key_code());
    }

   void on_write_leds_8(uint8_t addr, uint8_t qty, uint8_t x, uint8_t data) {
      if (addr + qty > 12) {
         Datagram::reply_error(modbus::error_t::illegal_data_value);
      } else {
         for (uint8_t i=addr; i < addr + qty; ++i) {
            mux::set_led(i, data & 1);
            data >>= 9;
         }
      }

      Datagram::set_size(6);
   }

   void on_write_leds_12(uint8_t addr, uint8_t qty, uint8_t x, uint16_t data) {
      if (addr + qty > 12) {
         Datagram::reply_error(modbus::error_t::illegal_data_value);
      } else {
         on_write_leds_8(addr, 8, x, data>>8);
         on_write_leds_8(addr+8, qty-8, x, data & 0xff);
      }
   }

   void on_write_single_led(uint8_t index, uint16_t value) {
      mux::set_led(index, value == 0xFF00);
   }

   void on_read_leds(uint8_t addr, uint8_t qty) {
       // Validate quantity against the available number of LEDs
       if (addr + qty > 12) {
           Datagram::reply_error(modbus::error_t::illegal_data_value);
           return;
       }

       uint16_t value = 0;

       // Build the value by reading the LEDs
       for (uint8_t i = 0; i < qty; ++i) {
           value |= (mux::get_led(addr + i) ? 1 : 0) << i;
       }

       // Determine the size of the response
       uint8_t byte_count = (qty > 8) ? 2 : 1;

       Datagram::set_size(2); // Include the byte count itself in the size
       Datagram::pack<uint8_t>(byte_count);
       Datagram::pack<uint8_t>(value & 0xFF);

       if (byte_count > 1) {
           Datagram::pack<uint8_t>((value >> 8) & 0xFF);
       }
   }

    /// Custom package. Set the leds and return the push buttons and switches
    /// This is the most efficient transfer.
    /// Format: 37 100 <leds=16> <crc=16> <== 37 100 <sw=8> <key=8> <crc=16>
    /// Size: 6 + 6 (+2ms for the frame) = 20T < 4ms@115200
    /// Periodic send every 20ms
    /// @param leds 12-bits with LED to change
    void on_custom(uint16_t leds) {
        mux::set_leds(leds);
        Datagram::set_size(2);
        Datagram::pack(mux::get_switch_status());
        Datagram::pack(mux::get_active_key_code());
    }


   void on_read_holding(uint16_t addr, uint16_t qty) {
      Datagram::set_size(2);
      Datagram::pack<uint8_t>(qty * 2);
      for (uint16_t i=addr; i<addr+qty; ++i) {
         uint16_t value = 0;
         if ( i == 10 ) {
            value = 99;
         }
         Datagram::pack<uint16_t>(value);
      }
   }

    void on_write_holding(uint16_t addr, uint16_t value) {
      if ( addr == 10 ) {
         switch(value) {
            case 0: break;
            case 1: piezzo_play(150, "B4"); break;
            case 2: piezzo_play(150, "C4"); break;
            case 3: piezzo_play(150, "D4"); break;
            default:
               Datagram::reply_error(modbus::error_t::illegal_data_value);
               break;
         }
      } else {
         Datagram::reply_error(modbus::error_t::illegal_data_value);
      }
    }
}