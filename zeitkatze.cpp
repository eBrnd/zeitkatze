#include <ctime>
#include <csignal>
#include <iostream>

class Zeitkatze {
  public:
    Zeitkatze() : start(clock()) { }
    ~Zeitkatze() { std::cout << (static_cast<double>(clock() - start) / static_cast<double>(CLOCKS_PER_SEC)) << std::endl; }
  private:
    clock_t start;
};

// oh so globally
bool running = true;

void interrupt(int sig) {
  running = false;
}

int main(int argc, char** argv) {
  Zeitkatze z;
  signal(SIGINT, interrupt);

  while(running);

  return 0;
}
