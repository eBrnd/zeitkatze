#ifndef ZEITKATZE_RUNNER_HPP
#define ZEITKATZE_RUNNER_HPP 

#include "zeitkatze.hpp"
#include <memory> // make_unique()
#include <termios.h> // struct termios, tcgetattr()
#include <poll.h> // poll()
#include <csignal> // signal()
#include <unistd.h> // read()
#include <iomanip> // setw(), setfill()
#include <fcntl.h> //fcntl()


class ZeitkatzeRunner {
	public:
		ZeitkatzeRunner () { init(true); }
		ZeitkatzeRunner (bool enable_color) { init(enable_color); }
		void run();

	private:
		// methods
		void init(bool enable_color);
		// members
		std::unique_ptr<Zeitkatze> zeitkatze = std::make_unique<Zeitkatze>();
		const double kExitTimeout_ = 0.8;
		bool running_ = true;
		// Print a new line before the end_time. Should be done after ^C^C but not after ^D
		bool print_newline_ = false;
		double last_interrupt_ = -kExitTimeout_;
};

#endif /* ZEITKATZE_RUNNER_HPP */
