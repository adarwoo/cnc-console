/**
 * This file was generated to create a state machine for processing
 * uart data used for a modbus RTU. It should be included by
 * the modbus_rtu_slave.cpp file only which will create a full rtu slave device.
 */
#include <logger.h>
#include <stdint.h>
#include <asx/modbus_rtu.hpp>

namespace console {
    // All callbacks registered
    void on_get_sw_status(uint8_t addr, uint8_t qty);
    void on_read_leds(uint8_t addr, uint8_t qty);
    void on_write_leds_8(uint8_t addr, uint8_t qty, uint8_t, uint8_t data);
    void on_write_leds_12(uint8_t addr, uint8_t qty, uint8_t, uint16_t data);
    void on_get_active_key();
    void on_beep();
    void on_custom(uint16_t leds);
    void on_write_single_led(uint8_t index, uint16_t value);

    // All states to consider
    enum class state_t : uint8_t {
        IGNORE = 0,
        ERROR = 1,
        DEVICE_ADDRESS,
        DEVICE_37,
        DEVICE_37_READ_DISCRETE_INPUTS,
        DEVICE_37_READ_DISCRETE_INPUTS_from,
        DEVICE_37_READ_DISCRETE_INPUTS_from__ON_GET_SW_STATUS__CRC,
        RDY_TO_CALL__ON_GET_SW_STATUS,
        DEVICE_37_WRITE_SINGLE_COIL,
        DEVICE_37_WRITE_SINGLE_COIL_from,
        DEVICE_37_WRITE_SINGLE_COIL_from__ON_WRITE_SINGLE_LED__CRC,
        RDY_TO_CALL__ON_WRITE_SINGLE_LED,
        DEVICE_37_READ_COILS,
        DEVICE_37_READ_COILS_from,
        DEVICE_37_READ_COILS_from__ON_READ_LEDS__CRC,
        RDY_TO_CALL__ON_READ_LEDS,
        DEVICE_37_WRITE_MULTIPLE_COILS,
        DEVICE_37_WRITE_MULTIPLE_COILS_start,
        DEVICE_37_WRITE_MULTIPLE_COILS_start_qty,
        DEVICE_37_WRITE_MULTIPLE_COILS_start_qty_bytecount,
        DEVICE_37_WRITE_MULTIPLE_COILS_start_qty_bytecount__ON_WRITE_LEDS_8__CRC,
        RDY_TO_CALL__ON_WRITE_LEDS_8,
        DEVICE_37_WRITE_MULTIPLE_COILS_start_qty_1,
        DEVICE_37_WRITE_MULTIPLE_COILS_start_qty_1_bytecount,
        DEVICE_37_WRITE_MULTIPLE_COILS_start_qty_1_bytecount__ON_WRITE_LEDS_12__CRC,
        RDY_TO_CALL__ON_WRITE_LEDS_12,
        DEVICE_37_READ_INPUT_REGISTERS,
        DEVICE_37_READ_INPUT_REGISTERS_from,
        DEVICE_37_READ_INPUT_REGISTERS_from__ON_GET_ACTIVE_KEY__CRC,
        RDY_TO_CALL__ON_GET_ACTIVE_KEY,
        DEVICE_37_WRITE_SINGLE_REGISTER,
        DEVICE_37_WRITE_SINGLE_REGISTER_1,
        DEVICE_37_WRITE_SINGLE_REGISTER_1__ON_BEEP__CRC,
        RDY_TO_CALL__ON_BEEP,
        DEVICE_37_CUSTOM,
        DEVICE_37_CUSTOM__ON_CUSTOM__CRC,
        RDY_TO_CALL__ON_CUSTOM
    };

    class Datagram {
        using error_t = asx::modbus::error_t;

        ///< Adjusted buffer to only receive the largest amount of data possible
        inline static uint8_t buffer[16];
        ///< Number of characters in the buffer
        inline static uint8_t cnt;
        ///< Number of characters to send
        inline static uint8_t frame_size;
        ///< Error code
        inline static error_t error;
        ///< State
        inline static state_t state;
        ///< CRC for the datagram
        inline static asx::modbus::Crc crc{};

        static inline auto ntoh(const uint8_t offset) -> uint16_t {
            return (static_cast<uint16_t>(buffer[offset]) << 8) | static_cast<uint16_t>(buffer[offset + 1]);
        }

        static inline auto ntohl(const uint8_t offset) -> uint32_t {
            return
                (static_cast<uint32_t>(buffer[offset]) << 24) |
                (static_cast<uint32_t>(buffer[offset+1]) << 16) |
                (static_cast<uint32_t>(buffer[offset+2]) << 8) |
                static_cast<uint16_t>(buffer[offset+3]);
        }

    public:
        // Status of the datagram
        enum class status_t : uint8_t {
            GOOD_FRAME = 0,
            NOT_FOR_ME = 1,
            BAD_CRC = 2
        };

        static void reset() noexcept {
            cnt=0;
            crc.reset();
            error = error_t::ok;
            state = state_t::DEVICE_ADDRESS;
        }

        static status_t get_status() noexcept {
            if (state == state_t::IGNORE) {
                return status_t::NOT_FOR_ME;
            }

            return crc.check() ? status_t::GOOD_FRAME : status_t::BAD_CRC;
        }

        static void process_char(const uint8_t c) noexcept {
            LOG_TRACE("DGRAM", "Char: 0x%.2x, index: %d, state: %d", c, cnt, (uint8_t)state);

            if (state == state_t::IGNORE) {
                return;
            }

            crc(c);

            if (state != state_t::ERROR) {
                // Store the frame
                buffer[cnt++] = c; // Store the data
            }

            switch(state) {
            case state_t::ERROR:
                break;
            case state_t::DEVICE_ADDRESS:
                if ( c == 37 ) {
                    state = state_t::DEVICE_37;
                } else {
                    error = error_t::ignore_frame;
                    state = state_t::IGNORE;
                }
                break;
            case state_t::DEVICE_37:
                if ( c == 2 ) {
                    state = state_t::DEVICE_37_READ_DISCRETE_INPUTS;
                } else if ( c == 5 ) {
                    state = state_t::DEVICE_37_WRITE_SINGLE_COIL;
                } else if ( c == 1 ) {
                    state = state_t::DEVICE_37_READ_COILS;
                } else if ( c == 15 ) {
                    state = state_t::DEVICE_37_WRITE_MULTIPLE_COILS;
                } else if ( c == 4 ) {
                    state = state_t::DEVICE_37_READ_INPUT_REGISTERS;
                } else if ( c == 6 ) {
                    state = state_t::DEVICE_37_WRITE_SINGLE_REGISTER;
                } else if ( c == 101 ) {
                    state = state_t::DEVICE_37_CUSTOM;
                } else {
                    error = error_t::illegal_function_code;
                    state = state_t::ERROR;
                }
                break;
            case state_t::DEVICE_37_READ_DISCRETE_INPUTS:
                if ( cnt == 4 ) {
                    auto c = ntoh(cnt-2);

                    if ( c <= 3 ) {
                        state = state_t::DEVICE_37_READ_DISCRETE_INPUTS_from;
                    } else {
                        error = error_t::illegal_data_value;
                        state = state_t::ERROR;
                    }
                }
                break;
            case state_t::DEVICE_37_READ_DISCRETE_INPUTS_from:
                if ( cnt == 6 ) {
                    auto c = ntoh(cnt-2);

                    if ( c >= 1 and c <= 4 ) {
                        state = state_t::DEVICE_37_READ_DISCRETE_INPUTS_from__ON_GET_SW_STATUS__CRC;
                    } else {
                        error = error_t::illegal_data_value;
                        state = state_t::ERROR;
                    }
                }
                break;
            case state_t::DEVICE_37_READ_DISCRETE_INPUTS_from__ON_GET_SW_STATUS__CRC:
                if ( cnt == 8 ) {
                    state = state_t::RDY_TO_CALL__ON_GET_SW_STATUS;
                }
                break;
            case state_t::DEVICE_37_WRITE_SINGLE_COIL:
                if ( cnt == 4 ) {
                    auto c = ntoh(cnt-2);

                    if ( c <= 11 ) {
                        state = state_t::DEVICE_37_WRITE_SINGLE_COIL_from;
                    } else {
                        error = error_t::illegal_data_value;
                        state = state_t::ERROR;
                    }
                }
                break;
            case state_t::DEVICE_37_WRITE_SINGLE_COIL_from:
                if ( cnt == 6 ) {
                    auto c = ntoh(cnt-2);

                    if ( c == 0xff00 || c == 0x0 ) {
                        state = state_t::DEVICE_37_WRITE_SINGLE_COIL_from__ON_WRITE_SINGLE_LED__CRC;
                    } else {
                        error = error_t::illegal_data_value;
                        state = state_t::ERROR;
                    }
                }
                break;
            case state_t::DEVICE_37_WRITE_SINGLE_COIL_from__ON_WRITE_SINGLE_LED__CRC:
                if ( cnt == 8 ) {
                    state = state_t::RDY_TO_CALL__ON_WRITE_SINGLE_LED;
                }
                break;
            case state_t::DEVICE_37_READ_COILS:
                if ( cnt == 4 ) {
                    auto c = ntoh(cnt-2);

                    if ( c <= 11 ) {
                        state = state_t::DEVICE_37_READ_COILS_from;
                    } else {
                        error = error_t::illegal_data_value;
                        state = state_t::ERROR;
                    }
                }
                break;
            case state_t::DEVICE_37_READ_COILS_from:
                if ( cnt == 6 ) {
                    auto c = ntoh(cnt-2);

                    if ( c >= 1 and c <= 12 ) {
                        state = state_t::DEVICE_37_READ_COILS_from__ON_READ_LEDS__CRC;
                    } else {
                        error = error_t::illegal_data_value;
                        state = state_t::ERROR;
                    }
                }
                break;
            case state_t::DEVICE_37_READ_COILS_from__ON_READ_LEDS__CRC:
                if ( cnt == 8 ) {
                    state = state_t::RDY_TO_CALL__ON_READ_LEDS;
                }
                break;
            case state_t::DEVICE_37_WRITE_MULTIPLE_COILS:
                if ( cnt == 4 ) {
                    auto c = ntoh(cnt-2);

                    if ( c <= 11 ) {
                        state = state_t::DEVICE_37_WRITE_MULTIPLE_COILS_start;
                    } else {
                        error = error_t::illegal_data_value;
                        state = state_t::ERROR;
                    }
                }
                break;
            case state_t::DEVICE_37_WRITE_MULTIPLE_COILS_start:
                if ( cnt == 6 ) {
                    auto c = ntoh(cnt-2);

                    if ( c >= 1 and c <= 8 ) {
                        state = state_t::DEVICE_37_WRITE_MULTIPLE_COILS_start_qty;
                    } else if ( c >= 9 and c <= 12 ) {
                        state = state_t::DEVICE_37_WRITE_MULTIPLE_COILS_start_qty_1;
                    } else {
                        error = error_t::illegal_data_value;
                        state = state_t::ERROR;
                    }
                }
                break;
            case state_t::DEVICE_37_WRITE_MULTIPLE_COILS_start_qty:
                if ( c == 1 ) {
                    state = state_t::DEVICE_37_WRITE_MULTIPLE_COILS_start_qty_bytecount;
                } else {
                    error = error_t::illegal_data_value;
                    state = state_t::ERROR;
                }
                break;
            case state_t::DEVICE_37_WRITE_MULTIPLE_COILS_start_qty_bytecount:
                state = state_t::DEVICE_37_WRITE_MULTIPLE_COILS_start_qty_bytecount__ON_WRITE_LEDS_8__CRC;
                break;
            case state_t::DEVICE_37_WRITE_MULTIPLE_COILS_start_qty_bytecount__ON_WRITE_LEDS_8__CRC:
                if ( cnt == 10 ) {
                    state = state_t::RDY_TO_CALL__ON_WRITE_LEDS_8;
                }
                break;
            case state_t::DEVICE_37_WRITE_MULTIPLE_COILS_start_qty_1:
                if ( c == 2 ) {
                    state = state_t::DEVICE_37_WRITE_MULTIPLE_COILS_start_qty_1_bytecount;
                } else {
                    error = error_t::illegal_data_value;
                    state = state_t::ERROR;
                }
                break;
            case state_t::DEVICE_37_WRITE_MULTIPLE_COILS_start_qty_1_bytecount:
                if ( cnt == 9 ) {
                    state = state_t::DEVICE_37_WRITE_MULTIPLE_COILS_start_qty_1_bytecount__ON_WRITE_LEDS_12__CRC;
                }
                break;
            case state_t::DEVICE_37_WRITE_MULTIPLE_COILS_start_qty_1_bytecount__ON_WRITE_LEDS_12__CRC:
                if ( cnt == 11 ) {
                    state = state_t::RDY_TO_CALL__ON_WRITE_LEDS_12;
                }
                break;
            case state_t::DEVICE_37_READ_INPUT_REGISTERS:
                if ( cnt == 4 ) {
                    auto c = ntoh(cnt-2);

                    if ( c == 0 ) {
                        state = state_t::DEVICE_37_READ_INPUT_REGISTERS_from;
                    } else {
                        error = error_t::illegal_data_value;
                        state = state_t::ERROR;
                    }
                }
                break;
            case state_t::DEVICE_37_READ_INPUT_REGISTERS_from:
                if ( cnt == 6 ) {
                    auto c = ntoh(cnt-2);

                    if ( c == 1 ) {
                        state = state_t::DEVICE_37_READ_INPUT_REGISTERS_from__ON_GET_ACTIVE_KEY__CRC;
                    } else {
                        error = error_t::illegal_data_value;
                        state = state_t::ERROR;
                    }
                }
                break;
            case state_t::DEVICE_37_READ_INPUT_REGISTERS_from__ON_GET_ACTIVE_KEY__CRC:
                if ( cnt == 8 ) {
                    state = state_t::RDY_TO_CALL__ON_GET_ACTIVE_KEY;
                }
                break;
            case state_t::DEVICE_37_WRITE_SINGLE_REGISTER:
                if ( cnt == 4 ) {
                    auto c = ntoh(cnt-2);

                    if ( c == 1 ) {
                        state = state_t::DEVICE_37_WRITE_SINGLE_REGISTER_1;
                    } else {
                        error = error_t::illegal_data_value;
                        state = state_t::ERROR;
                    }
                }
                break;
            case state_t::DEVICE_37_WRITE_SINGLE_REGISTER_1:
                if ( cnt == 6 ) {
                    auto c = ntoh(cnt-2);

                    if ( c <= 1 ) {
                        state = state_t::DEVICE_37_WRITE_SINGLE_REGISTER_1__ON_BEEP__CRC;
                    } else {
                        error = error_t::illegal_data_value;
                        state = state_t::ERROR;
                    }
                }
                break;
            case state_t::DEVICE_37_WRITE_SINGLE_REGISTER_1__ON_BEEP__CRC:
                if ( cnt == 8 ) {
                    state = state_t::RDY_TO_CALL__ON_BEEP;
                }
                break;
            case state_t::DEVICE_37_CUSTOM:
                if ( cnt == 4 ) {
                    state = state_t::DEVICE_37_CUSTOM__ON_CUSTOM__CRC;
                }
                break;
            case state_t::DEVICE_37_CUSTOM__ON_CUSTOM__CRC:
                if ( cnt == 6 ) {
                    state = state_t::RDY_TO_CALL__ON_CUSTOM;
                }
                break;
            case state_t::RDY_TO_CALL__ON_GET_SW_STATUS:
            case state_t::RDY_TO_CALL__ON_WRITE_SINGLE_LED:
            case state_t::RDY_TO_CALL__ON_READ_LEDS:
            case state_t::RDY_TO_CALL__ON_WRITE_LEDS_8:
            case state_t::RDY_TO_CALL__ON_WRITE_LEDS_12:
            case state_t::RDY_TO_CALL__ON_GET_ACTIVE_KEY:
            case state_t::RDY_TO_CALL__ON_BEEP:
            case state_t::RDY_TO_CALL__ON_CUSTOM:
            default:
                error = error_t::illegal_data_value;
                state = state_t::ERROR;
                break;
            }
        }

        static void reply_error( error_t err ) noexcept {
            buffer[1] |= 0x80;
            buffer[2] = (uint8_t)err;
            cnt = 3;
        }

        template<typename T>
        static void pack(const T& value) noexcept {
            if constexpr ( sizeof(T) == 1 ) {
                buffer[cnt++] = value;
            } else if constexpr ( sizeof(T) == 2 ) {
                buffer[cnt++] = value >> 8;
                buffer[cnt++] = value & 0xff;
            } else if constexpr ( sizeof(T) == 4 ) {
                buffer[cnt++] = value >> 24;
                buffer[cnt++] = value >> 16 & 0xff;
                buffer[cnt++] = value >> 8 & 0xff;
                buffer[cnt++] = value & 0xff;
            }
        }

        static inline void set_size(uint8_t size) {
            cnt = size;
        }

        /** Called when a T3.5 has been detected, in a good sequence */
        static void ready_reply() noexcept {
            frame_size = cnt; // Store the frame size
            cnt = 2; // Points to the function code
            

            switch(state) {
            case state_t::IGNORE:
                break;
            case state_t::DEVICE_ADDRESS:
            case state_t::DEVICE_37:
            case state_t::DEVICE_37_READ_DISCRETE_INPUTS:
            case state_t::DEVICE_37_READ_DISCRETE_INPUTS_from:
            case state_t::DEVICE_37_READ_DISCRETE_INPUTS_from__ON_GET_SW_STATUS__CRC:
            case state_t::DEVICE_37_WRITE_SINGLE_COIL:
            case state_t::DEVICE_37_WRITE_SINGLE_COIL_from:
            case state_t::DEVICE_37_WRITE_SINGLE_COIL_from__ON_WRITE_SINGLE_LED__CRC:
            case state_t::DEVICE_37_READ_COILS:
            case state_t::DEVICE_37_READ_COILS_from:
            case state_t::DEVICE_37_READ_COILS_from__ON_READ_LEDS__CRC:
            case state_t::DEVICE_37_WRITE_MULTIPLE_COILS:
            case state_t::DEVICE_37_WRITE_MULTIPLE_COILS_start:
            case state_t::DEVICE_37_WRITE_MULTIPLE_COILS_start_qty:
            case state_t::DEVICE_37_WRITE_MULTIPLE_COILS_start_qty_bytecount:
            case state_t::DEVICE_37_WRITE_MULTIPLE_COILS_start_qty_bytecount__ON_WRITE_LEDS_8__CRC:
            case state_t::DEVICE_37_WRITE_MULTIPLE_COILS_start_qty_1:
            case state_t::DEVICE_37_WRITE_MULTIPLE_COILS_start_qty_1_bytecount:
            case state_t::DEVICE_37_WRITE_MULTIPLE_COILS_start_qty_1_bytecount__ON_WRITE_LEDS_12__CRC:
            case state_t::DEVICE_37_READ_INPUT_REGISTERS:
            case state_t::DEVICE_37_READ_INPUT_REGISTERS_from:
            case state_t::DEVICE_37_READ_INPUT_REGISTERS_from__ON_GET_ACTIVE_KEY__CRC:
            case state_t::DEVICE_37_WRITE_SINGLE_REGISTER:
            case state_t::DEVICE_37_WRITE_SINGLE_REGISTER_1:
            case state_t::DEVICE_37_WRITE_SINGLE_REGISTER_1__ON_BEEP__CRC:
            case state_t::DEVICE_37_CUSTOM:
            case state_t::DEVICE_37_CUSTOM__ON_CUSTOM__CRC:
                error = error_t::illegal_data_value;
            case state_t::ERROR:
                buffer[1] |= 0x80; // Mark the error
                buffer[2] = (uint8_t)error; // Add the error code
                cnt = 3;
                break;
            case state_t::RDY_TO_CALL__ON_GET_SW_STATUS:
                on_get_sw_status(buffer[3], buffer[5]);
                break;
            case state_t::RDY_TO_CALL__ON_WRITE_SINGLE_LED:
                on_write_single_led(buffer[3], ntoh(4));
                break;
            case state_t::RDY_TO_CALL__ON_READ_LEDS:
                on_read_leds(buffer[3], buffer[5]);
                break;
            case state_t::RDY_TO_CALL__ON_WRITE_LEDS_8:
                on_write_leds_8(buffer[3], buffer[5], buffer[6], buffer[7]);
                break;
            case state_t::RDY_TO_CALL__ON_WRITE_LEDS_12:
                on_write_leds_12(buffer[3], buffer[5], buffer[6], ntoh(7));
                break;
            case state_t::RDY_TO_CALL__ON_GET_ACTIVE_KEY:
                on_get_active_key();
                break;
            case state_t::RDY_TO_CALL__ON_BEEP:
                on_beep();
                break;
            case state_t::RDY_TO_CALL__ON_CUSTOM:
                on_custom(ntoh(2));
                break;
            default:
                break;
            }

            // If the cnt is 2 - nothing was changed in the buffer - return it as is
            if ( cnt == 2 ) {
                // Framesize includes the previous CRC which still holds valid
                cnt = frame_size;
            } else {
                // Add the CRC
                crc.reset();
                auto _crc = crc.update(std::string_view{(char *)buffer, cnt});
                buffer[cnt++] = _crc & 0xff;
                buffer[cnt++] = _crc >> 8;
            }
        }

        static std::string_view get_buffer() noexcept {
            // Return the buffer ready to send
            return std::string_view{(char *)buffer, cnt};
        }
    }; // struct Processor
} // namespace modbus