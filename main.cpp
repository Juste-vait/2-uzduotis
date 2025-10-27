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

struct BlockHeader {
    string prev_block_hash;
    double timestamp;
    string version;
    string transactions_hash;
    string difficulty;
    uint64_t nonce = 0;

    string to_string() const {
        ostringstream oss;
        oss << prev_block_hash << '|' << std::fixed << setprecision(6) << timestamp << '|' << version << '|' << transactions_hash << '|' << difficulty << '|' << nonce;
        return oss.str();
    }
};

struct Block {
    BlockHeader header;
    vector<Transaction> transactions;

    explicit Block(BlockHeader h, vector<Transaction> txs) : header(std::move(h)), transactions(std::move(txs)) {}

    string compute_hash() const {
        return custom_hash(header.to_string());
    }
};

class Blockchain {
    public:
        Blockchain() : rng(RNG_SEED) {
            create_genesis_block();
        }
    
    
    private:
        vector<Block> blocks;
        unordered_map<string, User> users;
        vector<string> user_keys;
        vector<Transaction> pending_transactions;
    
        mt19937_64 rng;
    
        void create_genesis_block() {
            BlockHeader header{
                string(64, '0'),
                current_time_seconds(),
                VERSION_,
                string(64, '0'),
                DIFFICULTY_PREFIX,
                0
            };
            Block genesis(header, {});
            blocks.push_back(genesis);
            cout << "[GENESIS] Sukurtas genesis blokas. Hash = " << genesis.compute_hash() << "\n";
        }
    
        static string fmt_index(int x, int width) {
            ostringstream oss;
            oss << setw(width) << setfill('0') << x;
            return oss.str();
        }
    
        double current_time_seconds() const {
            using namespace std::chrono;
            auto now = system_clock::now().time_since_epoch();
            return duration<double>(now).count();
        }
    
        int64_t rand_int(int64_t a, int64_t b) {
            uniform_int_distribution<int64_t> dist(a, b);
            return dist(rng);
        }
    
        string random_key() {
            uniform_int_distribution<size_t> dist(0, user_keys.size() - 1);
            return user_keys[dist(rng)];
        }
    
        vector<Transaction> sample_transactions(int k) {
            vector<int> idx(pending_transactions.size());
            iota(idx.begin(), idx.end(), 0);
            shuffle(idx.begin(), idx.end(), rng);
            vector<Transaction> out;
            out.reserve(k);
            for (int i = 0; i < k; ++i) out.push_back(pending_transactions[idx[i]]);
            return out;
        }
    
        void apply_transactions(const vector<Transaction>& txs) {
            for (const auto& tx : txs) {
                auto& sender = users[tx.sender];
                auto& receiver = users[tx.receiver];
                if (sender.balance >= tx.amount) {
                    sender.balance -= tx.amount;
                    receiver.balance += tx.amount;
                } else {
    
                }
            }
        }
    
        void erase_used_transactions(const vector<Transaction>& used) {
            unordered_set<string> used_ids;
            used_ids.reserve(used.size()*2);
            for (auto& t : used) used_ids.insert(t.transaction_id);
    
            vector<Transaction> keep;
            keep.reserve(pending_transactions.size());
            for (auto& t : pending_transactions) {
                if (!used_ids.count(t.transaction_id)) keep.push_back(std::move(t));
            }
            pending_transactions.swap(keep);
        }
    };
    

int main() {
    cout << " Supaprastintas Blockchain " << VERSION_ << endl;
    return 0;
}
