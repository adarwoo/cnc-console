/*
 * Relay modbus device main entry point.
 * The relays are initialised by the static constructor
 */
#include <asx/reactor.hpp>


int main()
{
   using namespace asx;

   // Run the reactor/scheduler
   reactor::run();
}
