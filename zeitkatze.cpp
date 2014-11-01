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

class Colors {
  public:
    enum class ColorEnum {
      Normal,
      Cat,
      Cat_hold,
      Total,
      Running,
      Running_lap,
      Split,
      Split_lap
    };

    static std::string getColor(ColorEnum c) {
      switch(c) {
        case ColorEnum::Normal: return "\033[0m";
        case ColorEnum::Cat: return "\033[34m";
        case ColorEnum::Cat_hold: return "\033[34;1m";
        case ColorEnum::Total: return "\033[37;1m";
        case ColorEnum::Running: return "\033[32m";
        case ColorEnum::Running_lap: return "\033[33m";
        case ColorEnum::Split: return "\033[32;1m";
        case ColorEnum::Split_lap: return "\033[33;1m";
        default: return "";
      }
    }
};

class Zeitkatze {
  public:
    Zeitkatze() : split_printed(false), start(steady_clock::now()), last_lap(start) { }
    ~Zeitkatze() { print_split_time(cats[cats.size() - 1], Colors::getColor(Colors::ColorEnum::Total)); std::cout << std::endl; }

    void print_split_time(const std::string& msg, const std::string& color = Colors::getColor(Colors::ColorEnum::Split)) {
      steady_clock::time_point now(steady_clock::now());
      std::stringstream sbuf;
      sbuf << Colors::getColor(Colors::ColorEnum::Cat_hold) << msg << Colors::getColor(Colors::ColorEnum::Cat_hold) << "   " << color << format_seconds(elapsed(), 4) << Colors::getColor(Colors::ColorEnum::Normal)
        << "  (" << Colors::getColor(Colors::ColorEnum::Split_lap) << format_seconds(duration_cast<duration<double>>(now - last_lap).count(), 4) << Colors::getColor(Colors::ColorEnum::Normal)
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
      sbuf << Colors::getColor(Colors::ColorEnum::Cat) << cats[0] << "   " << Colors::getColor(Colors::ColorEnum::Running) << format_seconds(elapsed(), 2) << Colors::getColor(Colors::ColorEnum::Normal);
      if (had_lap) {
        auto current_lap = duration_cast<duration<double>>(steady_clock::now() - last_lap);
        sbuf << "  (" << Colors::getColor(Colors::ColorEnum::Running_lap) << format_seconds(current_lap.count(), 2) << Colors::getColor(Colors::ColorEnum::Normal) << ")";
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
      return Colors::getColor(Colors::ColorEnum::Cat_hold) + cats[static_cast<unsigned>(elapsed() * 100) % (cats.size() - 2) + 1] + Colors::getColor(Colors::ColorEnum::Normal);
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
    z.print_split_time(z.some_cat());

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
          z.print_split_time(z.some_cat());
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
