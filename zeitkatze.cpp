#include <chrono>
#include <csignal>
#include <iostream>

using std::chrono::steady_clock;
using std::chrono::duration;
using std::chrono::duration_cast;

class Zeitkatze {
  public:
    Zeitkatze() : start(steady_clock::now()) { }
    ~Zeitkatze() { print(); }

    void print() { std::cout << elapsed() << std::endl; }
    double elapsed() {
      duration<double> time_span = duration_cast<duration<double>>(steady_clock::now() - start);
      return time_span.count(); }
  private:
    steady_clock::time_point start;
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
