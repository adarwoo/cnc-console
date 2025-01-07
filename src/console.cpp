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
    void on_get_sw_status()
    {
        Datagram::set_size(2);
        Datagram::pack<uint8_t>(1);
        Datagram::pack(mux::get_switch_status());
    }

    /// @brief  Get the currently pushed active key
    void on_get_active_key()
    {
        Datagram::set_size(2);
        Datagram::pack<uint8_t>(2);
        Datagram::pack<uint16_t>(mux::get_active_key_code());
    }

    /// @brief Set the leds
    /// @param data New leds values
    void on_write_leds(uint16_t data)
    {
        mux::set_leds(data);
        Datagram::set_size(6);
    }

    void on_read_leds(uint8_t addr, uint8_t qty)
    {
        if (qty > (12 - addr))
        {
            Datagram::reply_error(modbus::error_t::illegal_data_value);
        } else {
            uint16_t value = mux::get_leds();

            // If address is 0, keep all, if 1 remove the first etc..
            value >>= addr;

            // Mask to keep the count
            value &= (1 << qty) - 1;

            if (qty > 8)
            {
                Datagram::pack<uint8_t>(2);
                Datagram::pack<uint8_t>(value >> 8);
                Datagram::pack<uint8_t>(value & 0xff);
            }
            else
            {
                Datagram::pack<uint8_t>(1);
                Datagram::pack<uint8_t>(value & 0xff);
            }

            if (qty > (3 - addr))
            {
                Datagram::reply_error(modbus::error_t::illegal_data_value);
            }
            else
            {
                Datagram::pack(value);
            }
        }
    }

    /// Custom package. Set the leds and return the push buttons and switches
    /// This is the most efficient transfer.
    /// Format: 37 100 <leds=16> <crc=16> <== 37 100 <sw=8> <key=8> <crc=16>
    /// Size: 6 + 6 (+8 for the frame) = 20T < 2ms@115200
    /// @param leds 
    void on_custom(uint16_t leds) {
        mux::set_leds(leds);
        Datagram::set_size(2);
        Datagram::pack(mux::get_switch_status());
        Datagram::pack(mux::get_active_key_code());        
    }

    void on_beep()
    {
        piezzo_play(150, "B4");
    }
}