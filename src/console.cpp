#include <asx/reactor.hpp>
#include <asx/uart.hpp>

#include "datagram.hpp"
#include "mux.hpp"

#include <piezzo.h>

using namespace asx;

namespace console {
    /** Define the Usart to use by the rs485 */
    using UartConfig = uart::CompileTimeConfig<
        115200,                       // Baudrate
        asx::uart::width::_8,         // Width standard 8 bits per frame
        asx::uart::parity::even,      // Even parity (standard for Modbus)
        asx::uart::stop::_1,          // Unique stop bit (standard in Modbus)
        // Force RS485 mode and one wire (Rx pin is not used)
        asx::uart::rs485 | asx::uart::onewire
    >;

    using Uart = asx::uart::Uart<1, UartConfig>;    

    using modbus_slave = asx::modbus::Slave<Datagram, Uart>;

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