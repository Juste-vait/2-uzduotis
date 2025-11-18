#include <bitcoin/system.hpp>
#include <iostream>

int main() {
    std::cout << "libbitcoin version: " 
              << LIBBITCOIN_SYSTEM_VERSION << "\n";

    libbitcoin::data_chunk data{'h','e','l','l','o'};

    auto hash = libbitcoin::bitcoin_hash(data);
    std::cout << "SHA256(SHA256(\"hello\")) = "
              << libbitcoin::encode_base16(hash) << "\n";

    return 0;
}
