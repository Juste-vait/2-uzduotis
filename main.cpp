#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <random>
#include <algorithm>
#include <chrono>
#include <numeric>
#include <cmath>

using namespace std;

static const string VERSION_ = "v0.1";
static const int USERS_COUNT = 1000;
static const int64_t TX_COUNT = 10000;
static const int TXS_PER_BLOCK = 100;
static const string DIFFICULTY_PREFIX = "000";
static const uint64_t RNG_SEED = 42;

static bool starts_with(const string& s, const string& pref) {
    return s.size() >= pref.size() && equal(pref.begin(), pref.end(), s.begin());
}

static string to_hex16(uint64_t x) {
    stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(16) << std::nouppercase << x;
    return ss.str();
}

string custom_hash(const string& input) {
    uint64_t part0 = 0x1234567890abcdefULL;
    uint64_t part1 = 0xfedcba0987654321ULL;
    uint64_t part2 = 0x0f1e2d3c4b5a6978ULL;
    uint64_t part3 = 0x89abcdef01234567ULL;

    for (unsigned char b : input) {
        part0 = part0 + b;
        part1 = part1 + part0 * 3;
        part2 = part2 + part1 + (b * 7);
        part3 = part3 + part2 + (part0 * 2);
    }

    for (int i = 0; i < 10; i++) {
        part0 = (part0 ^ (part1 * 3)) * 0x4e97b2d8f3c1a5e3ULL;
        part1 = (part1 ^ (part2 * 5)) * 0xa3d5f79c482eb16fULL;
        part2 = (part2 ^ (part3 * 7)) * 0x5b18e4c7d92fa06eULL;
        part3 = (part3 ^ (part0 * 9)) * 0xc74a9e21f05bd83cULL;
    }

    stringstream ss;
    ss << hex << setfill('0') << setw(16) << part0 << setw(16) << part1 << setw(16) << part2 << setw(16) << part3;
    return ss.str();
}

struct User {
    string name;
    string public_key;
    int64_t balance;
};

struct Transaction {
    string sender;
    string receiver;
    int64_t amount;
    string transaction_id;

    Transaction(const string& s, const string& r, int64_t a)
        : sender(s), receiver(r), amount(a) {
        string base = sender + "|" + receiver + "|" + to_string(amount);
        transaction_id = custom_hash(base);
    }
};

int main() {
    cout << " Supaprastintas Blockchain " << VERSION_ << endl;
    return 0;
}
