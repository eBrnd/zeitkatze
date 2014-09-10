#include <ctime>
#include <csignal>
#include <iostream>

class Zeitkatze {
  public:
    Zeitkatze() : start(clock()) { }
    ~Zeitkatze() { print(); }

    void print() { std::cout << elapsed() << std::endl; }
    double elapsed() { return static_cast<double>(clock() - start) / static_cast<double>(CLOCKS_PER_SEC); }
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
