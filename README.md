# Metro

Periodic function runner

- `thx::Merto` :: Fundamental class implementation.
- `thx::metro` :: Singleton version above.
- `thx::ThreadedMetro` :: Threaded version. Each event will have their own thread.
- `thx::threaded_metro` :: Singleton version above.

## example
```cpp
#include <iostream>

#include "Metro.hpp"

void foo() {
    std::cout << "foo" << std::endl;
}

void bar() {
    std::cout << "bar" << std::endl;
}

void add_func() {
    auto& metro = thx::metro::get_instance();
    metro.add_ms(1000, [&]() {
        std::cout << "add func." << std::endl;
    });
}

int main() {
    auto& metro = thx::metro::get_instance();

    metro.add(foo).set_interval_ms(500);
    metro.add(bar).set_interval_ms(500).set_delay_ms(250);

    add_func();

    metro.reset();

    while(true) {
        metro(); 
    }

    return 0;
}
```