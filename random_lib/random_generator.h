#ifndef RANDOM_GENERATOR_H
#define RANDOM_GENERATOR_H

#include <string>
#include <vector>
#include <wolfssl/wolfcrypt/random.h>

class RandomGenerator
{
public:
    RandomGenerator(int keySize, int bufferSize, const std::string &passwordFile);
    ~RandomGenerator();

    void fillBuffer(int bytes);
    std::vector<unsigned char> getRandomBytes(int size);
    std::string getRandomHexBytes(int size);

private:
    WC_RNG rng;
    std::vector<unsigned char> buffer;
    int bufferLeft;
    int bufferRight;
    int bufferTotal;

    void prng_seed_from_file(const std::string &password_filename);
    int get_random_bytes(void *buf, int n);
    int bbs_init(int bits, const std::string &password_filename, int password_length);
    int bbs_next_random_byte();
    void bbs_free();
};

#endif
