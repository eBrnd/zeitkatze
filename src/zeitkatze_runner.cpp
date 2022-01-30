#include "zeitkatze_runner.hpp"

void ZeitkatzeRunner::init(bool enable_color) {
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
			if (zeitkatze->elapsed() - last_interrupt_ < kExitTimeout_) {
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
