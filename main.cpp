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
static optional<int> MAX_BLOCKS_TO_MINE = nullopt;

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

static string merkle_root_from_ids(vector<string> ids) {
    if (ids.empty()) return string(64, '0');

    while (ids.size() > 1) {
        vector<string> next;
        next.reserve((ids.size() + 1) / 2);

        for (size_t i = 0; i < ids.size(); i += 2) {
            string L = ids[i];
            string R = (i + 1 < ids.size()) ? ids[i + 1] : ids[i];
            next.push_back(custom_hash(L + R));
        }

        ids.swap(next);
    }

    return ids[0];
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

    Transaction(const string& s, const string& r, int64_t a) : sender(s), receiver(r), amount(a) {
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
    
        void generate_users(int n) {
            users.reserve(n);
            for (int i = 0; i < n; ++i) {
                string name = "User_" + fmt_index(i, 4);
                string pk = "pk_" + custom_hash(name).substr(0, 16);
                int64_t balance = rand_int(100, 1'000'000);
                users.emplace(pk, User{name, pk, balance});
                user_keys.push_back(pk);
            }
            cout << "[USERS] Sugeneruota: " << users.size() << " vartotojų\n" << endl;
        }

        void generate_transactions(int64_t n) {
            pending_transactions.reserve(pending_transactions.size() + n);
            for (int64_t i = 0; i < n; ++i) {
                string sender = random_key();
                string receiver = random_key();
                while (receiver == sender) receiver = random_key();
                int64_t max_send = max<int64_t>(1, users[sender].balance / 10);
                int64_t amount = rand_int(1, max_send);
                pending_transactions.emplace_back(sender, receiver, amount);
            }
            cout << "[TX] Sugeneruota: " << pending_transactions.size() << " laukiančių transakcijų\n" << endl;
        }  
        
        optional<Block> mine_next_block(int txs_per_block = TXS_PER_BLOCK) {
            if (pending_transactions.empty()) {
                cout << "[MINE] Nėra laukiančių transakcijų.\n";
                return nullopt;
            }
    
            int k = min<int>(txs_per_block, (int)pending_transactions.size());
            vector<Transaction> txs = sample_transactions(k);
    
            vector<string> ids;
            for (auto& tx : txs) {
                ids.push_back(tx.transaction_id);
            }
            string txs_hash = merkle_root_from_ids(ids);
    
            string prev_hash = last_block_hash();
            BlockHeader header{
                prev_hash,
                current_time_seconds(),
                VERSION_,
                txs_hash,
                DIFFICULTY_PREFIX,
                0
            };
            Block block(header, txs);
    
            cout << "[MINE] Kasinėjamas blokas #" << blocks.size() << " su " << txs.size() << " TX...\n";
    
            auto start = chrono::steady_clock::now();
            uint64_t attempts = 0;
            string h;
    
            while (true) {
                h = block.compute_hash();
                ++attempts;
                if (starts_with(h, DIFFICULTY_PREFIX)) {
                    auto took = chrono::duration<double>(chrono::steady_clock::now() - start).count();
                    cout << "[MINE] IŠKASTA! Hash=" << h << "  per " << attempts << " bandymų, " << fixed << setprecision(2) << took << "s\n";
                    break;
                }
                ++block.header.nonce;
            }
    
            if (block.header.prev_block_hash != last_block_hash()) {
                cerr << "[ERROR] Grandinė pasikeitė — prev hash nebeatitinka.\n";
                return nullopt;
                }
    
            apply_transactions(block.transactions);
            erase_used_transactions(block.transactions);
    
            blocks.push_back(std::move(block));
            cout << "[CHAIN] Blokas #" << (blocks.size()-1) << " pridėtas. Likusių TX: " << pending_transactions.size() << "\n" <<endl;
            return blocks.back();
        }

        string last_block_hash() const {
            return blocks.back().compute_hash();
        }

        string info() const {
            ostringstream oss;
            oss << "Blockchain(height=" << (blocks.size()-1) << ", users=" << users.size() << ", pending=" << pending_transactions.size() << ")";
            return oss.str();
        }

        void run_all() {
            cout << "[INFO] " << info() << "\n" << endl;
            cout << "[TARGET] Target: '" << DIFFICULTY_PREFIX << "...' (hash prefiksas)\n" << endl;
            cout << "------------------------------------------------------------------------------------\n" << endl;
            int mined = 0;
            while (!pending_transactions.empty()) {
                if (MAX_BLOCKS_TO_MINE.has_value() && mined >= *MAX_BLOCKS_TO_MINE) {
                    cout << "[STOP] Pasiekėme demonstracinį limitą: " << *MAX_BLOCKS_TO_MINE << " blokų.\n";
                    break;
                }
                auto b = mine_next_block(TXS_PER_BLOCK);
                if (!b.has_value()) break;
                ++mined;
            }
            cout << "[DONE] " << info() << "\n";
            cout << "Paskutinio bloko hash: " << last_block_hash() << "\n";
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
            cout << "[GENESIS] Sukurtas genesis blokas. Hash = " << genesis.compute_hash() << "\n" << endl;
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
            for (auto& tx : txs) {
                string expect = custom_hash(tx.sender + "|" + tx.receiver + "|" + to_string(tx.amount));

                if (tx.amount <= 0) {
                    cout << "[SKIP] Siunčiama suma turi būti teigiama: " << tx.amount << "\n";
                    continue;
                }

                if (!users.count(tx.sender) || !users.count(tx.receiver)) {
                    cout << "[SKIP] Neegzistuojantis siuntėjas arba gavėjas: " << tx.sender << " / " << tx.receiver << "\n";
                    continue;
                }

                if (tx.transaction_id != expect) {
                    cout << "[SKIP] Neteisingas TX ID: " << tx.transaction_id << "\n";
                    continue;
                }

                if (users[tx.sender].balance < tx.amount) {
                    cout << "[SKIP] Nepakanka lėšų: " << tx.sender << " turi " << users[tx.sender].balance << ", bando siųsti " << tx.amount << "\n";
                    continue;
                }

                if (tx.sender == tx.receiver) {
                    cout << "[SKIP] Siuntėjas ir gavėjas negali būti tas pats: " << tx.sender << "\n";
                    continue;
                }
        
                users[tx.sender].balance -= tx.amount;
                users[tx.receiver].balance += tx.amount;
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

        Block make_candidate_block(int k) {
            int take = min<int>(k, (int)pending_transactions.size());
            vector<Transaction> txs = sample_transactions(take);

            vector<string> ids; ids.reserve(txs.size());
            for (auto& tx : txs) ids.push_back(tx.transaction_id);
            string txs_hash = merkle_root_from_ids(ids);

            BlockHeader header{
                last_block_hash(), 
                current_time_seconds(),
                VERSION_,   
                txs_hash,          
                DIFFICULTY_PREFIX,
                0  
            };

            return Block(header, std::move(txs));
        }

        optional<Block> mine_candidates(vector<Block>& candidates, double seconds) {
            using clock = std::chrono::steady_clock;
            auto start = clock::now();
            auto deadline = start + std::chrono::duration<double>(seconds);

            uint64_t attempts = 0;

            while (clock::now() < deadline) {
                for (auto& b : candidates) {
                    if (clock::now() >= deadline) break;

                    string h = b.compute_hash();
                    ++attempts;

                    if (starts_with(h, DIFFICULTY_PREFIX)) {
                        double took = std::chrono::duration<double>(clock::now() - start).count();
                        cout << "[MINE] Kandidatas IŠKASTAS! Hash=" << h << " (nonce=" << b.header.nonce << ") per " << attempts << " bandymų, " << fixed << setprecision(2) << took << "s\n";
                        return b;
                    }

                    ++b.header.nonce;
                }
            }

            return nullopt;
        }


    };
    

int main() {
    cout << " Supaprastintas Blockchain " << VERSION_ << "\n" << endl;
    Blockchain bc;
    bc.generate_users(USERS_COUNT);
    bc.generate_transactions(TX_COUNT);
    bc.run_all();
    return 0;
}
