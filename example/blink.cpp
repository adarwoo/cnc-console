#include <asx/chrono.hpp>
#include <asx/reactor.hpp>
#include <asx/ioport.hpp>

using namespace asx;
using namespace asx::ioport;

auto constexpr MY_LED = PinDef{B, 1};

auto flash_led() -> void {
   Pin(MY_LED).toggle();
}

auto main() -> int {
   using namespace std::chrono;

   Pin(MY_LED).init(dir_t::out);
   reactor::init();
   reactor::bind(flash_led).repeat(1s);
   reactor::run();
}
