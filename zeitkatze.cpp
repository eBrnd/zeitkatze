#include <chrono>
#include <cmath>
#include <csignal>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <poll.h>
#include <sstream>
#include <unistd.h>
#include <vector>

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::steady_clock;
using std::setfill;
using std::setw;

class Zeitkatze {
  public:
    Zeitkatze() : split_printed(false), start(steady_clock::now()) { }
    ~Zeitkatze() { print_split_time(cats[cats.size() - 1]); std::cout << std::endl; }

    void print_split_time(const std::string& msg) {
      std::cout << "\r" << msg << "   " << format_seconds(elapsed(), 4) << std::flush;
      split_printed = true;
    }

    void print_current_time() {
      if (split_printed) {
        std::cout << std::endl;
        split_printed = false;
      }
      std::cout << "\r" << cats[0] << "   " << format_seconds(elapsed(), 2) << std::flush;
    }

    double elapsed() {
      duration<double> time_span = duration_cast<duration<double>>(steady_clock::now() - start);
      return time_span.count();
    }

    std::string format_seconds(double seconds, unsigned precision = 2) {
      double full_seconds = floor(seconds);
      double fractional_seconds = seconds - full_seconds;

      double minutes = floor(full_seconds / 60.0);

      unsigned min = static_cast<unsigned>(minutes);
      unsigned sec = static_cast<unsigned>(full_seconds) % 60;
      unsigned frs = static_cast<unsigned>(fractional_seconds * pow(10, precision));

      std::ostringstream oss;
      if (min > 0)
        oss << min << ":";
      oss << setfill('0') << setw(2) << sec << "." << setw(precision) << frs;

      return oss.str();
    }

    std::string some_cat() {
      return cats[static_cast<unsigned>(elapsed() * 100) % (cats.size() - 2) + 1];
    }

    static const std::vector<const char*> cats;

  private:
    bool split_printed;
    steady_clock::time_point start;
};

const std::vector<const char*> Zeitkatze::cats({ "=(^.^)=", "=(o.o)=", "=(^.^)\"", "=(x.x)=",
    "=(o.o)m", " (o,o) ", "=(0.0)=", "=(@.@)=", "=(*.*)=", "=(-.-)=", "=(v.v)=", "=(o.O)=",
    "=[˙.˙]=", "=(~.~)=", "=(ˇ.ˇ)=", "=(=.=)=" });

// oh so globally
const double EXIT_TIMEOUT = 0.8;
Zeitkatze z;
bool running = true;
double last_interrupt = -EXIT_TIMEOUT;

void interrupt(int sig) {
  if (z.elapsed() - last_interrupt < EXIT_TIMEOUT)
    running = false;
  else
    z.print_split_time(z.some_cat());

  last_interrupt = z.elapsed();
}

int main(int argc, char** argv) {
  signal(SIGINT, interrupt);
  fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
  pollfd fds[] = { { STDIN_FILENO, POLLIN, 0 } };
  unsigned char x = 0;

  while(running) {
    if (poll(fds, 1, 42) == 1) {
      if ((fds[0].revents & POLLIN) && read(0, &x, 1) == 0)
        return 0;
    }

    if (z.elapsed() - last_interrupt > EXIT_TIMEOUT)
      z.print_current_time();
  }

  return 0;
}
