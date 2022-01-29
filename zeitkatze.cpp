#include <atomic>
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
#include <string>

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::steady_clock;
using std::setfill;
using std::setw;

typedef std::vector<std::string> CatVector;
typedef std::vector<std::string>::size_type CatIndex;

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

static bool color_enabled = true;

// oh so globally
static std::atomic<bool> interrupted(false);

static void interrupt(int) {
    interrupted = true;
}

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


class Zeitkatze {
    public:
        Zeitkatze() : split_printed_(false), start_(steady_clock::now()), last_lap_(start_), last_line_len_(0) { }
        void print_split_time() { print_time(some_cat_index(), Color::Split); }
        void print_end_time() { print_time(kCats.size() - 1, Color::Total); }
        void print_time(const CatIndex cat_index, const Color color); 
        void print_current_time();
        double elapsed();
        std::string format_seconds(double seconds, unsigned precision = 2);
        CatIndex some_cat_index();
        void reset_laps();
        static const CatVector kCats;

    private:
        bool split_printed_, had_lap_;
        steady_clock::time_point start_, last_lap_;
        unsigned last_line_len_;
};


        void Zeitkatze::print_time(const CatIndex cat_index, const Color color) {
            steady_clock::time_point now(steady_clock::now());
            std::stringstream sbuf;
            sbuf << Color::Cat_hold << kCats[cat_index] << Color::Cat_hold << "   " << color << format_seconds(elapsed(), 4) << Color::Normal
                << "  (" << Color::Split_lap << format_seconds(duration_cast<duration<double>>(now - last_lap_).count(), 4) << Color::Normal
                << ")";
            std::string&& line = sbuf.str();
            std::cout << "\r" << std::string(last_line_len_, ' ')
                << "\r" << line << std::flush;
            last_lap_ = now;
            split_printed_ = true;
            had_lap_ = true;
            last_line_len_ = line.size();
        }

        void Zeitkatze::print_current_time() {
            if (split_printed_) {
                std::cout << std::endl;
                split_printed_ = false;
            }
            std::stringstream sbuf;
            sbuf << Color::Cat << kCats[0] << "   " << Color::Running << format_seconds(elapsed(), 2) << Color::Normal;
            if (had_lap_) {
                auto current_lap = duration_cast<duration<double>>(steady_clock::now() - last_lap_);
                sbuf << "  (" << Color::Running_lap << format_seconds(current_lap.count(), 2) << Color::Normal << ")";
            }
            std::string&& line = sbuf.str();
            std::cout << "\r" << std::string(last_line_len_, ' ')
                << "\r" << line << std::flush;
            last_line_len_ = line.size();
        }

        double Zeitkatze::elapsed() {
            duration<double> time_span = duration_cast<duration<double>>(steady_clock::now() - start_);
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


        CatIndex Zeitkatze::some_cat_index() { return static_cast<CatIndex>
            (elapsed() * 100) % (kCats.size() - 2) + 1;
        }

        void Zeitkatze::reset_laps() {
            last_lap_ = steady_clock::now();
            had_lap_ = false;
        }



class ZeitkatzeRunner {
    public:
        ZeitkatzeRunner () { init(); }
        void run();

    private:
        // methods
        void init();
        // members
        // TODO: arguments
        std::unique_ptr<Zeitkatze> zeitkatze = std::make_unique<Zeitkatze>();
        const double kExitTimeout_ = 0.8;
        bool running_ = true;
        // Print a new line before the end_time. Should be done after ^C^C but not after ^D
        bool print_newline_ = false;
        bool interrupted_;
        bool color_enabled_ = true;
        double last_interrupt_ = -kExitTimeout_;
};

void ZeitkatzeRunner::init() {
    char* color_env = getenv("ZEITKATZE_COLOR");
    if (color_env != nullptr && std::string(color_env) == "0")
        color_enabled_ = false;

    signal(SIGINT, interrupt);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

    struct termios tio;
    if (tcgetattr(1, &tio) == 0) {
        tio.c_lflag &= ~(ICANON);
        tio.c_cc[VMIN] = 0;
        tio.c_cc[VTIME] = 0;
        tcsetattr(1, TCSANOW, &tio);
    }
}


void ZeitkatzeRunner::run() {
    pollfd fds[] = { { STDIN_FILENO, POLLIN, 0 } };
    unsigned char x = 0;
    while(running_) {
        if (poll(fds, 1, 42) == 1) {
            if ((fds[0].revents & POLLIN) && read(0, &x, 1) == 1) {
                switch (x) {
                    case '\n':
                    case '\r':
                        zeitkatze->print_split_time();
                        break;
                    case 'r':
                        zeitkatze->reset_laps();
                        break;
                    case 4: // ^D
                        running_ = false;
                }
            }
        }
        if (zeitkatze->elapsed() - last_interrupt_ > kExitTimeout_)
            zeitkatze->print_current_time();

        if (interrupted) {
            if (zeitkatze->elapsed() - last_interrupt_ < kExitTimeout_)
            {
                running_ = false;
                print_newline_ = true;
            } else {
                zeitkatze->print_split_time();
            }
            last_interrupt_ = zeitkatze->elapsed();
            interrupted = false;
        }
    }

    if (print_newline_)
        std::cout << std::endl;
    zeitkatze->print_end_time();
    std::cout << std::endl;
}



const CatVector Zeitkatze::kCats({ "=(^.^)=", "=(o.o)=", "=(^.^)\"", "=(x.x)=",
        "=(o.o)m", " (o,o) ", "=(0.0)=", "=(@.@)=", "=(*.*)=", "=(-.-)=", "=(v.v)=", "=(o.O)=",
        "=[˙.˙]=", "=(~.~)=", "=(ˇ.ˇ)=", "=(=.=)=" });



int main(int argc, char** argv) {
    auto z = std::make_unique<ZeitkatzeRunner>();
    z->run();
    return 0;
}
