#include "zeitkatze.hpp"

using std::chrono::duration;
using std::chrono::duration_cast;
using std::setfill;
using std::setw;

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

std::string Zeitkatze::format_seconds(double seconds, unsigned precision) {
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
