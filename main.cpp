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

int main() {
    cout << " Supaprastintas Blockchain " << VERSION_ << endl;
    return 0;
}
