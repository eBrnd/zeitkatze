#include <chrono>
#include <cmath>
#include <csignal>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <poll.h>
#include <sstream>
#include <termios.h>
#include <unistd.h>
#include <vector>

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::steady_clock;
using std::setfill;
using std::setw;


enum class Color {
  Normal,
  Cat,
  Cat_hold,
  Total,
  Running,
  Running_lap,
  Split,
  Split_lap
};

std::ostream& operator<<(std::ostream& oss, Color c) {
  switch(c) {
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
  return oss;
}

class Zeitkatze {
  public:
    Zeitkatze() : split_printed(false), start(steady_clock::now()), last_lap(start) { }
    ~Zeitkatze() { print_end_time(); std::cout << std::endl; }

    void print_split_time() {
      print_time(some_cat(), Color::Split);
    }

    void print_end_time() {
      print_time(cats[cats.size() - 1], Color::Total);
    }

    void print_time(const std::string& msg, const Color color) {
      steady_clock::time_point now(steady_clock::now());
      std::stringstream sbuf;
      sbuf << Color::Cat_hold << msg << Color::Cat_hold << "   " << color << format_seconds(elapsed(), 4) << Color::Normal
        << "  (" << Color::Split_lap << format_seconds(duration_cast<duration<double>>(now - last_lap).count(), 4) << Color::Normal
        << ")";
      std::string&& line = sbuf.str();
      std::cout << "\r" << std::string(last_line_len, ' ')
        << "\r" << line << std::flush;
      last_lap = now;
      split_printed = true;
      had_lap = true;
      last_line_len = line.size();
    }

    void print_current_time() {
      if (split_printed) {
        std::cout << std::endl;
        split_printed = false;
      }
      std::stringstream sbuf;
      sbuf << Color::Cat << cats[0] << "   " << Color::Running << format_seconds(elapsed(), 2) << Color::Normal;
      if (had_lap) {
        auto current_lap = duration_cast<duration<double>>(steady_clock::now() - last_lap);
        sbuf << "  (" << Color::Running_lap << format_seconds(current_lap.count(), 2) << Color::Normal << ")";
      }
      std::string&& line = sbuf.str();
      std::cout << "\r" << std::string(last_line_len, ' ')
        << "\r" << line << std::flush;
      last_line_len = line.size();
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
      std::ostringstream oss;
      oss << Color::Cat_hold << cats[static_cast<unsigned>(elapsed() * 100) % (cats.size() - 2) + 1] << Color::Normal;
      return oss.str();
    }

    void reset_laps() {
      last_lap = steady_clock::now();
      had_lap = false;
    }

    static const std::vector<const char*> cats;

  private:
    bool split_printed, had_lap;
    steady_clock::time_point start, last_lap;
    unsigned last_line_len;
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
    z.print_split_time();

  last_interrupt = z.elapsed();
}

int main(int argc, char** argv) {
  signal(SIGINT, interrupt);
  fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
  pollfd fds[] = { { STDIN_FILENO, POLLIN, 0 } };
  unsigned char x = 0;

  struct termios tio;

  if (tcgetattr(1, &tio) == 0) {
    tio.c_lflag &= ~(ECHO | ICANON);
    tio.c_cc[VMIN] = 0;
    tio.c_cc[VTIME] = 0;
    tcsetattr(1, TCSANOW, &tio);
  }

  while(running) {
    if (poll(fds, 1, 42) == 1) {
      if ((fds[0].revents & POLLIN) && read(0, &x, 1) == 1) {
        switch (x) {
        case '\n':
        case '\r':
          z.print_split_time();
          break;

        case 'r':
          z.reset_laps();
          break;

        case 4: // ^D
          return 0;
        }
      }
    }

    if (z.elapsed() - last_interrupt > EXIT_TIMEOUT)
      z.print_current_time();
  }

  return 0;
}
