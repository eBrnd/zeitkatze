#include "zeitkatze.hpp"
#include "zeitkatze_runner.hpp"
#include <iostream>
#include <string>
#include <atomic>


bool color_enabled {true};
std::atomic<bool> interrupted {false};

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

void interrupt(int) {
	interrupted = true;
}


int main(int argc, char** argv) {
	if (argc > 1) {
		if (std::string(argv[1]) == "-c" || std::string(argv[1]) == "--color") {
			color_enabled = true;
		} else if (std::string(argv[1]) == "-n" || std::string(argv[1]) == "--no-color") {
			color_enabled = false;
		} else {
			std::cout << "Zeitkatze" << std::endl;
			std::cout << std::endl;
			std::cout << "		time cat -- literally" << std::endl;
			std::cout << std::endl;
			std::cout << "Usage: zeitkatze [-c | -n | --color | --no-color]" << std::endl;
			std::cout << std::endl;
			std::cout << "Ctrl-c for split/lap time, Ctrl-cc or Ctrl-d to stop." << std::endl;
			std::cout << std::endl;
			std::cout << "-c, --color		 Enable colored output (default)." << std::endl;
			std::cout << "-n, --no-color	Disable colored output." << std::endl;
			std::cout << "								If both arguments are present, the first one counts." << std::endl;
			std::cout << "								(overrides ZEITKATZE_COLOR environment variable (set to" << std::endl;
			std::cout << "								\"0\" for no color))" << std::endl;
                        return 0;
        }
    }

	auto z = std::make_unique<ZeitkatzeRunner>(color_enabled);
	z->run();
	return 0;
}
