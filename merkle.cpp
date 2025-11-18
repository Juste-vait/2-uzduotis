#include <bitcoin/system.hpp>
#include <iostream>

bc::hash_digest create_merkle(bc::hash_list& merkle)
{
    // Stop if hash list is empty or contains one element
    if (merkle.empty())
        return bc::null_hash;
    else if (merkle.size() == 1)
        return merkle[0];

    while (merkle.size() > 1)
    {
        if (merkle.size() % 2 != 0)
            merkle.push_back(merkle.back());

        bc::hash_list new_merkle;

        for (auto it = merkle.begin(); it != merkle.end(); it += 2)
        {
            bc::data_chunk concat_data(bc::hash_size * 2);
            auto concat = bc::serializer<decltype(concat_data.begin())>(concat_data.begin());
            concat.write_hash(*it);
            concat.write_hash(*(it + 1));

            bc::hash_digest new_root = bc::bitcoin_hash(concat_data);

            new_merkle.push_back(new_root);
        }

        merkle = new_merkle;

        std::cout << "Current merkle hash list:\n";
        for (const auto& h : merkle)
            std::cout << "  " << bc::encode_base16(h) << std::endl;
        std::cout << std::endl;
    }

    return merkle[0];
}

int main() {
    bc::hash_list tx_hashes {{
        bc::hash_literal("b75ca3106ed100521aa50e3ec267a06431c6319538898b25e1b757a5736f5fb4"),
        bc::hash_literal("d41f5de48325e79070ccd3a23005f7a3b405f3ce1faa4df09f6d71770497e9d5"),
        bc::hash_literal("c2f59c6fc8e812f5f1f00c8a0a9ab1929c1e796788c57f49001b8006a824ea17"),
        bc::hash_literal("965f866bf8623bbf956c1b2aeec1efc1ad162fd428ab7fb89f128a0754ebbc32"),
    }};

    const bc::hash_digest merkle_root = create_merkle(tx_hashes);
    std::cout << "Merkle Root Hash: " << bc::encode_base16(merkle_root) << std::endl;
    return 0;
}
