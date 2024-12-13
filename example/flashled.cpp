#include <asx/chrono.hpp>
#include <asx/sysclock.hpp>
#include <asx/reactor.hpp>
#include <asx/ioport.hpp>

using namespace asx;

namespace {
   using namespace asx::sysclock;

   auto sysclock = sysclock<20MHz>::init();
}

namespace {
   using namespace asx::ioport;

   auto led1 = Pin<A, 2>::init(dir_t::out);
}

auto flash_led() -> void {
   led1.toggle();
}

auto main() -> int {
   using namespace asx::chrono;

   reactor::init();
   reactor::bind(flash_led).repeat(1s);
   reactor::run();
}
