#include <Socket.h>
#include <Timer.h>
#include <chrono>
#include <inttypes.h>
#include <iostream>
#include <stdio.h>

typedef std::chrono::high_resolution_clock Clock;

int main(int argc, char *argv[]) {
  uint64_t rx_total = 0;
  uint64_t rx = 0;
  uint64_t rxp = 0;
  const int intervalUs = 1000000;
  const int B1M = 1000000;

  Socket::Endpoint local("0.0.0.0", 9000);
  UDPServer NMX(local);
  NMX.printbuffers();
  NMX.setbuffers(0, 500000);
  NMX.printbuffers();

  Timer upd;
  auto usecs = upd.timeus();

  for (;;) {
    rx += NMX.receive();

    if (rx > 0)
      rxp++;

    if ((rxp % 100) == 0)
      usecs = upd.timeus();

    if (usecs >= intervalUs) {
      rx_total += rx;
      printf("Rx rate: %.2f Mbps, rx %" PRIu64 " MB (total: %" PRIu64
             " MB) %ld usecs\n",
             rx * 8.0 / (usecs / 1000000.0) / B1M, rx / B1M, rx_total / B1M,
             usecs);
      rx = 0;
      upd.now();
      usecs = upd.timeus();
    }
  }
}