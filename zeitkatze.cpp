#include <chrono>
#include <csignal>
#include <iostream>
#include <poll.h>
#include <vector>
#include <iomanip>

using std::chrono::steady_clock;
using std::chrono::duration;
using std::chrono::duration_cast;

class Zeitkatze {
  public:
    Zeitkatze() : split_printed(false), start(steady_clock::now()) {
      std::cout << std::fixed << std::setprecision(2);
    }
    ~Zeitkatze() { print_split_time(cats[2]); std::cout << std::endl; }

    void print_split_time(const std::string& msg) {
      std::cout << "\r" << msg << "   "
          << std::setprecision(4) << elapsed() << std::setprecision(2)
          << std::flush;
      split_printed = true;
    }

    void print_current_time() {
      if (split_printed) {
        std::cout << std::endl;
        split_printed = false;
      }
      std::cout << "\r" << cats[0] << "   " << elapsed() << std::flush;
    }

    double elapsed() {
      duration<double> time_span = duration_cast<duration<double>>(steady_clock::now() - start);
      return time_span.count();
    }

    static const std::vector<const char*> cats;
  private:
    bool split_printed;
    steady_clock::time_point start;
};

const std::vector<const char*> Zeitkatze::cats({ "=(^.^)=", "=(o.o)=", "=(=.=)=" });

// oh so globally
const double EXIT_TIMEOUT = 0.8;
Zeitkatze z;
bool running = true;
double last_interrupt = -EXIT_TIMEOUT;

void interrupt(int sig) {
  if (z.elapsed() - last_interrupt < EXIT_TIMEOUT)
    running = false;
  else
    z.print_split_time(z.cats[1]);

  last_interrupt = z.elapsed();
}

int main(int argc, char** argv) {
  signal(SIGINT, interrupt);

  while(running) {
    poll(0, 0, 42);
    if (z.elapsed() - last_interrupt > EXIT_TIMEOUT)
      z.print_current_time();
  }

  return 0;
}
