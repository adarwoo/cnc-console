/*
 * Console modbus device main entry point.
 */
#include <asx/reactor.hpp>

#include "console.hpp"
#include "mux.hpp"


int main()
{
   console::modbus_slave::init();
   console::mux::init();

   // Run the reactor/scheduler
   asx::reactor::run();
}
