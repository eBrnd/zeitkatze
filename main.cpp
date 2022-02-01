#include  "zeitkatze.hpp"

#include <iostream>
#include <string>
#include <atomic>
#include <memory> // make_unique


// define external variables and operator
bool color_enabled {true};
std::atomic<bool> interrupted {false};

std::ostream& operator<<(std::ostream& oss, Color c) {
    if (color_enabled) {
        switch (c) {
            case Color::Normal: oss << "\033[0m"; break;
            case Color::Cat: oss << "\033[34m"; break;
            case Color::Cat_hold: oss << "\033[34;1m"; break;
            case Color::Total: oss << "\033[37;1m"; break;
            case Color::Running: oss << "\033[32m"; break;
            case Color::Running_lap: oss << "\033[33m"; break;
            case Color::Split: oss << "\033[32;1m"; break;
            case Color::Split_lap: oss << "\033[33;1m"; break;
            default: break;
        }
    }
    return oss;
}

void interrupt(int) {
    interrupted = true;
}




int main()
{
    auto z = Zeitkatze();        
    z.run();
}
