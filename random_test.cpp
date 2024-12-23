#include <iostream>
#include "random_lib/random_generator.h"

int main()
{
    RandomGenerator generator(2048, 1024, "random.key");

    std::vector<unsigned char> randomBytes = generator.getRandomBytes(20);
    std::cout << "Random bytes: ";
    for (unsigned char byte : randomBytes)
    {
        std::cout << std::hex << (int)byte << " ";
    }
    std::cout << std::dec << std::endl;

    std::string randomHex = generator.getRandomHexBytes(32);
    std::cout << "Random hex string: " << randomHex << std::endl;
    generator.saveKey("random.key");
    return 0;
}