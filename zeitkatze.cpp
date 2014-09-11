#include <chrono>
#include <csignal>
#include <iostream>
#include <poll.h>

using std::chrono::steady_clock;
using std::chrono::duration;
using std::chrono::duration_cast;

class Zeitkatze {
  public:
    Zeitkatze() : start(steady_clock::now()) { }
    ~Zeitkatze() { print_split_time("Stop: "); }

    void print_split_time(const std::string& msg) {
      std::cout << "\r" << std::flush << msg << elapsed() << std::endl;
    }

    void print_current_time() {
      std::cout << "\r" << std::flush << elapsed();
     }

    double elapsed() {
      duration<double> time_span = duration_cast<duration<double>>(steady_clock::now() - start);
      return time_span.count();
    }

  private:
    steady_clock::time_point start;
};

// oh so globally
Zeitkatze z;
bool running = true;

void interrupt(int sig) {
  const double EXIT_TIMEOUT = 0.8;
  static double last_interrupt = -EXIT_TIMEOUT;

  if (z.elapsed() - last_interrupt < EXIT_TIMEOUT)
    running = false;
  else
    z.print_split_time("Split: ");

  last_interrupt = z.elapsed();
}

int main(int argc, char** argv) {
  signal(SIGINT, interrupt);

  while(running) {
    poll(0, 0, 50);
    z.print_current_time();
  }

  return 0;
}
