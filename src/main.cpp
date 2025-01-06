/*
 * Relay modbus device main entry point.
 * The relays are initialised by the static constructor
 */
#include <chrono>
#include <asx/reactor.hpp>
#include "mux.hpp"

using namespace asx;
using namespace std::chrono;


void seq() {
   static uint16_t c = 1;

   console::mux::set_leds(c);

   if ( c == 0x40 ) {
      c = 0x0100;
   } else if ( c == 0x4000 ) {
      c = 1;
   }

   c <<= 1;
}



int main()
{
   console::mux::init();
   reactor::bind(seq).repeat(500ms);

   // Run the reactor/scheduler
   reactor::run();
}
