# asx
ASX is the core library for building C and C++ application for the ATTiny2 family based on a reactor pattern.

The reactor pattern allow binding arbitry functions to have them called in the 'main' namespace, as opposed to inside interrupt context.
With the reactor, asynchronous events can be managed collaboratively without the need for synchronisation nor the risk of race.

You are still encourage to use interrupts, but the interrupt should simply save the data, and notify the reactor (ring the bell!).

The actually handling of the data takes place in main.

The downside of the reactor is the lack of control over the latency.
To ease the problem, the reactor has priorities, so reactor with higher importance gets served first.

The worse case latency is always going to be the longest possible handler.

So the reactor works nicely when handling is quick.
For functions that take up more time, the handling should be sliced.

The reactor integrates a timer, so repeating reactors or delayed are possible.
Timer can also be safely stopped without the fear of a race conditions.

Finally, the ASX framework comes with all the goodies from the ASF project - but for the tiny2 as well as the full standard C++ library.

It is designed to compile with a C++17 or C++20 compiler.

Example code:

```C++
  #include <asx/sysclock.hpp>
  #include <asx/reactor.hpp>
  #include <asx/ioport.hpp>
  #include <chrono>

  using namespace asx;

  namespace {
     using namespace asx::sysclock;
     using sysclock = sysclock<20MHz>;
  }
 
  namespace {
     using namespace asx::ioport;
     auto led1 = ioport<A, 2, output | pullup>{0};
  }

  auto flash_led() -> void {
     led1.toggle();
  }
  
  auto main() -> int {
     using namespace std::chrono;
     sysclock::init();
     reactor::init();

     reactor.bind(flash_led).repeat(1s);

     reactor::run();
  }
```
  
   
    
  
