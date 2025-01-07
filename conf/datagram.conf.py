#!/usr/bin/env python3
from modbus_rtu_slave_rc import *  # Import everything from modbus_generator

# Coils = LED+ (write multiple only)
# Discrete inputs = Switch state
# Input register = Active key

# Keys
# --------------
#  0 : None
#  1 : Start
#  2 : Stop
#  3 : Homing
#  4 : Goto0
#  5 : Park
#  6 : Chuck
#  7 : Door
#  8 : Shift + Start
#  9 : Shift + Stop
# 10 : Shift + Homing
# 11 : Shift + Goto0
# 12 : Shift + Park
# 13 : Shift + Chuck
# 14 : Shift + Door

# Switches
# --------------
# 0b0001 OvDoor
# 0b0010 OvDust
# 0b0100 OvCool
# 0b1000 OvFree

# LEDs
# --------------
# 0x0001 Start
# 0x0002 Stop
# 0x0004 Homing
# 0x0008 Goto0
# 0x0010 Park
# 0x0020 Chuck
# 0x0100 OvDoor
# 0x0200 OvDust
# 0x0400 OvCool
# 0x0800 OvFree
# 0x1000 Door
# 0x2000 Shift

Modbus({
    "buffer_size": 8,
    "namespace": "console",

    "callbacks": {
        "on_get_sw_status" : [],
        "on_read_leds"     : [(u8, "addr"), (u8, "qty")],        
        "on_write_leds"    : [(u16, "data")],
        "on_get_active_key": [],
        "on_beep"          : [],
        "on_custom"        : [(u16, "leds")]
    },

    "device@37": [
        # Read the push button and switches state
        (READ_DISCRETE_INPUTS,  u16(0, alias="from"),
                                u16(4, alias="qty"), # Byte count
                                "on_get_sw_status"),

        (READ_COILS,            u16(0, 11, alias="from"),
                                u16(1, 12, alias="qty"),
                                "on_read_leds"),

        (WRITE_MULTIPLE_COILS,  u16(0, alias="from"),
                                u16(16, alias="qty"),
                                u8(2, alias="bytecount"),
                                u16(alias="data"),
                                "on_write_leds"),

        # Returns the active key
        (READ_INPUT_REGISTERS,  u16(0, alias="from"),
                                u16(1, alias="qty"),
                                "on_get_active_key"),

        (WRITE_SINGLE_REGISTER, u16(1), u16(0,1), "on_beep"),

        (CUSTOM,                u16(alias="leds"), "on_custom"),
    ]
})
