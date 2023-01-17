#include <iostream>
#include <vector>
#include <cxxopts.hpp>

#include "libspp/libspp.h"

int main(int argc, char *argv[]) {
  cxxopts::Options options("sppunpack", "Unpack a CCSDS SPP packet stream from stdin to stdout");
  options.add_options()
    ("h,help", "Print usage")
    ;

  auto result = options.parse(argc, argv);

  // Show help menu
  if (result.count("help")) {
    std::cerr << options.help() << '\n';
    exit(0);
  }

  SPPPacket packet;
  while (std::cin >> packet) {
    std::vector<std::byte> data = packet.data();
    for (auto&& it : data) {
      std::cout << static_cast<const uint8_t>(it);
    }
  }
}
