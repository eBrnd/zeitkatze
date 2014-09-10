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
Zeitkatze z;
bool running = true;

void interrupt(int sig) {
  const double EXIT_TIMEOUT = 0.8;
  static double last_interrupt = -EXIT_TIMEOUT;
  z.print();

  if (z.elapsed() - last_interrupt < EXIT_TIMEOUT)
    running = false;

  last_interrupt = z.elapsed();
}

int main(int argc, char** argv) {
  signal(SIGINT, interrupt);

  while(running);

  return 0;
}
