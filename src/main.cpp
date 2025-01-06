/*
 * Console modbus device main entry point.
 */
#include <asx/reactor.hpp>

#include <piezzo.h>
#include "console.hpp"
#include "mux.hpp"

/** Arcade tune */
constexpr auto arcade_tune = "C,3 R C E G E G E D R D F A2~A3 B G E B G E B G E C' R B, C'~C1";

int main()
{
   console::modbus_slave::init();
   console::mux::init();
   piezzo_init();

#ifdef NDEBUG
   // Play some arcade tune from memory
   piezzo_play(190, arcade_tune);
#endif   

   // Run the reactor/scheduler
   asx::reactor::run();
}
