#ifndef ZEITKATZE_HPP
#define ZEITKATZE_HPP 
#include "zeitkatze_base.hpp"
#include <memory> // make_unique()
#include <termios.h> // struct termios, tcgetattr()
#include <poll.h> // poll()
#include <csignal> // signal()
#include <unistd.h> // read()
#include <fcntl.h> //fcntl()
#include <atomic> // atomic<T>
#include <chrono> // chrono::duration, chrono::duration_cast
#include <cmath> // pow(), floor()
#include <iostream> // ostream()
#include <sstream>
#include <vector> // vector<T>
#include <string> // string
#include <iomanip> // setw(), setfill()

// using - TODO: in .cpp
using std::chrono::duration;
using std::chrono::duration_cast;
using std::setfill;
using std::setw;


//-----------------------------------------------------------------------------
// External variable declarations
extern bool color_enabled;
extern std::atomic<bool> interrupted;
extern void interrupt(int);

//-----------------------------------------------------------------------------
// Types/enums/operators
typedef std::vector<std::string> CatVector;
typedef std::vector<std::string>::size_type CatIndex;
using std::chrono::steady_clock;
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

extern std::ostream& operator<<(std::ostream& oss, Color c); 




class Zeitkatze: public ZeitkatzeBase
{
public:
    Zeitkatze():
        enable_color_(true),
        split_printed_(false),
        start_(steady_clock::now()),
        last_lap_(start_),
        last_line_len_(0),
        precision_(2)
            { init(enable_color_); }
    Zeitkatze(bool enable_color):
        enable_color_(enable_color),
        split_printed_(false),
        start_(steady_clock::now()),
        last_lap_(start_),
        last_line_len_(0),
        precision_(2)
            { init(enable_color); }
    Zeitkatze(bool enable_color, unsigned precision):
        enable_color_(enable_color),
        split_printed_(false),
        start_(steady_clock::now()),
        last_lap_(start_),
        last_line_len_(0),
        precision_(precision)
            { init(enable_color); }
    virtual void init(bool enable_color);
    virtual void run();
    bool enable_color() const { return enable_color_; }

private:
    // methods
    // members
    // how many decimals when formating seconds in Zeitkatze instance
    unsigned prec_;
    const double kExitTimeout_ = 0.8;
    bool running_ = true;
    // Print a new line before the end_time. Should be done after ^C^C but not after ^D
    bool print_newline_ = false;
    double last_interrupt_ = -kExitTimeout_;
    // zeitkatze
    bool split_printed_, had_lap_;
    steady_clock::time_point start_, last_lap_;
    unsigned last_line_len_;
    // how many decimals to use when measuring time
    const unsigned precision_;
    bool enable_color_;
    const CatVector kCats_ = { "=(^.^)=", "=(o.o)=", "=(^.^)\"", "=(x.x)=",
    "=(o.o)m", " (o,o) ", "=(0.0)=", "=(@.@)=", "=(*.*)=", "=(-.-)=", "=(v.v)=", "=(o.O)=",
    "=[˙.˙]=", "=(~.~)=", "=(ˇ.ˇ)=", "=(=.=)=" };
		CatIndex some_cat_index();
		void print_split_time() { print_time(some_cat_index(), Color::Split); }
		void print_end_time() { print_time(kCats_.size() - 1, Color::Total); }
		void print_time(const CatIndex cat_index, const Color color); 
		void print_current_time();
		double elapsed();
		std::string format_seconds(double seconds);
		void reset_laps();

};


void Zeitkatze::print_time(const CatIndex cat_index, const Color color) {
    steady_clock::time_point now(steady_clock::now());
    std::stringstream sbuf;
    sbuf << Color::Cat_hold << kCats_[cat_index] << Color::Cat_hold << "   " << color << format_seconds(elapsed()) << Color::Normal
        << "  (" << Color::Split_lap << format_seconds(duration_cast<duration<double>>(now - last_lap_).count()) << Color::Normal
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
    sbuf << Color::Cat << kCats_[0] << "   " << Color::Running << format_seconds(elapsed()) << Color::Normal;
    if (had_lap_) {
        auto current_lap = duration_cast<duration<double>>(steady_clock::now() - last_lap_);
        sbuf << "  (" << Color::Running_lap << format_seconds(current_lap.count()) << Color::Normal << ")";
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

std::string Zeitkatze::format_seconds(double seconds) {
    double full_seconds = floor(seconds);
    double fractional_seconds = seconds - full_seconds;
    double minutes = floor(full_seconds / 60.0);
    unsigned min = static_cast<unsigned>(minutes);
    unsigned sec = static_cast<unsigned>(full_seconds) % 60;
    unsigned frs = static_cast<unsigned>(fractional_seconds * pow(10, precision_));

    std::ostringstream oss;
    if (min > 0)
        oss << min << ":";
    oss << setfill('0') << setw(2) << sec << "." << setw(precision_) << frs;

    return oss.str();
}


CatIndex Zeitkatze::some_cat_index() { return static_cast<CatIndex>
    (elapsed() * 100) % (kCats_.size() - 2) + 1;
}

void Zeitkatze::reset_laps() {
    last_lap_ = steady_clock::now();
    had_lap_ = false;
}

void Zeitkatze::init(bool enable_color) {
	color_enabled = enable_color;
	char* color_env = getenv("ZEITKATZE_COLOR");
	if (color_env != nullptr && std::string(color_env) == "0")
		color_enabled = false;

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



void Zeitkatze::run() {
	pollfd fds[] = { { STDIN_FILENO, POLLIN, 0 } };
	unsigned char x = 0;
	while(running_) {
		if (poll(fds, 1, 42) == 1) {
			if ((fds[0].revents & POLLIN) && read(0, &x, 1) == 1) {
				switch (x) {
					case '\n':
					case '\r':
						print_split_time();
						break;
					case 'r':
						reset_laps();
						break;
					case 4: // ^D
						running_ = false;
				}
			}
		}
		if (elapsed() - last_interrupt_ > kExitTimeout_)
			print_current_time();

		if (interrupted) {
			if (elapsed() - last_interrupt_ < kExitTimeout_) {
				running_ = false;
				print_newline_ = true;
			} else {
				print_split_time();
			}
			last_interrupt_ = elapsed();
			interrupted = false;
		}
	}

	if (print_newline_)
		std::cout << std::endl;
	print_end_time();
	std::cout << std::endl;
}

#endif /* ZEITKATZE_HPP */
