#include <piezzo.h>

#include "console.hpp"
#include "mux.hpp"


using namespace asx;

namespace console {
    /// @brief  Read 4 bits for the switch
    void on_get_sw_status() {
        Datagram::set_size(2);
        Datagram::pack<uint8_t>(1);
        Datagram::pack(mux::get_switch_status());
    }

    /// @brief  Get the currently pushed active key
    void on_get_active_key() {
        Datagram::set_size(2);
        Datagram::pack<uint8_t>(2);
        Datagram::pack<uint16_t>(mux::get_active_key_code());
    }

    /// @brief Set the leds
    /// @param data New leds values
    void on_write_leds(uint16_t data) {
        mux::set_leds(data);
        Datagram::set_size(6);
    }

    void on_beep() {
        piezzo_play(192, "C");
    }
}