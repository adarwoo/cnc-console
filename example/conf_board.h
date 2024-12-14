#pragma once
/**
 * Board configuration
 * Using macros which work with C and C++ io APIs, assign all I/O pins
 * of the board.
 * Use the macro/struct PinDef.
 * In C code, this macro creates an ioport_pin_t, in C++ a PinDef
 */
#ifdef __cplusplus
#include <asx/ioport.hpp>
#else
#include <ioport.h>
#endif

// Define spare pins used for tracing
#define TRACE_INFO PinDef(A, 3)
#define TRACE_WARN PinDef(A, 5)
#define TRACE_ERR  PinDef(B, 2)

#ifndef NDEBUG
   // Assign pins for tracing the reactor
#  define DEBUG_REACTOR_IDLE TRACE_WARN
#  define DEBUG_REACTOR_BUSY TRACE_INFO

   // Assign a pin for the alert function
#  define ALERT_OUTPUT_PIN TRACE_ERR
#endif

// Define the pin to blink
#define MY_LED PinDef(B, 0)
