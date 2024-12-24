#include "random_generator.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <cstdint>
#include <ctime>
#include <cstring>
#include <fcntl.h>

#ifdef _WIN32
#include <Windows.h>
#include <process.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <wolfssl/wolfcrypt/random.h>
#include <wolfssl/wolfcrypt/types.h>
#include <wolfssl/wolfcrypt/error-crypt.h>

RandomGenerator::RandomGenerator(int keySize, int bufferSize, const std::string &passwordFile)
    : bufferLeft(0), bufferRight(0), bufferTotal(0)
{
    if (bufferSize <= 0)
    {
        bufferSize = 256 << 20;
    }
    buffer.resize(bufferSize);

    if (wc_InitRng(&rng) != 0)
    {
        std::cerr << "[ERROR] Failed to initialize RNG" << std::endl;
        exit(1);
    }
    std::cerr << "[INFO] RNG initialized successfully" << std::endl;

    bbs_init(keySize, passwordFile, passwordFile.length());
    for (int i = 0; i <= 64; i++)
    {
        bbs_next_random_byte();
    }
}

RandomGenerator::~RandomGenerator()
{
    wc_FreeRng(&rng);
    std::cerr << "[INFO] RNG freed successfully" << std::endl;
}

void RandomGenerator::prng_seed_from_file(const std::string &password_filename)
{
    std::vector<unsigned char> file_data;

    // Проверка существования файла
    std::ifstream file(password_filename, std::ios::binary);
    if (!file)
    {
        std::cerr << "[INFO] File not found. Creating new file: " << password_filename << std::endl;

        // Файл не существует, создаем новый файл с случайными данными
        std::ofstream new_file(password_filename, std::ios::binary);
        if (!new_file)
        {
            std::cerr << "[ERROR] Failed to create file: " << password_filename << std::endl;
            exit(1);
        }

        // Генерация случайных данных
        file_data.resize(256); // Размер данных для создания файла (например, 256 байт)
        get_random_bytes(file_data.data(), file_data.size());

        // Записываем случайные данные в файл
        new_file.write(reinterpret_cast<char *>(file_data.data()), file_data.size());
        new_file.close();
        std::cerr << "[INFO] File created and populated with random data." << std::endl;
    }
    else
    {
        // Если файл существует, читаем его содержимое
        file.seekg(0, std::ios::end);
        size_t file_size = file.tellg();
        file.seekg(0, std::ios::beg);
        file_data.resize(file_size);
        file.read(reinterpret_cast<char *>(file_data.data()), file_size);
        file.close();
        std::cerr << "[INFO] Read " << file_size << " bytes from password file." << std::endl;
    }

    // Используем байты из файла для инициализации генератора
    if (wc_RNG_GenerateBlock(&rng, file_data.data(), file_data.size()) != 0)
    {
        std::cerr << "[ERROR] Failed to seed RNG with file data" << std::endl;
        exit(1);
    }

    std::cerr << "[INFO] RNG seeded with file data" << std::endl;
}

int RandomGenerator::get_random_bytes(void *buf, int n)
{
    int result = wc_RNG_GenerateBlock(&rng, (unsigned char *)buf, n);
    if (result != 0)
    {
        std::cerr << "[ERROR] Failed to generate random bytes, error: " << result << std::endl;
        return 0;
    }
    std::cerr << "[DEBUG] Generated random bytes: ";
    for (int i = 0; i < n; ++i)
    {
        std::cerr << std::hex << std::setw(2) << std::setfill('0') << ((unsigned char *)buf)[i] << " ";
    }
    std::cerr << std::endl;
    return n;
}

int RandomGenerator::bbs_init(int bits, const std::string &password_filename, int password_length)
{
    prng_seed_from_file(password_filename); // Инициализация с использованием данных из файла
    return 0;
}

int RandomGenerator::bbs_next_random_byte()
{
    unsigned char random_byte = 0;
    get_random_bytes(&random_byte, 1);
    return random_byte;
}

void RandomGenerator::fillBuffer(int bytes)
{
    if (buffer.size() < bytes)
    {
        buffer.resize(bytes);
    }

    bufferLeft = bytes;
    bufferRight = bytes;
    bufferTotal = bytes;
    get_random_bytes(buffer.data(), bytes);

    std::cerr << "[DEBUG] Buffer filled with random bytes: ";
    for (int i = 0; i < bytes; ++i)
    {
        std::cerr << std::hex << std::setw(2) << std::setfill('0') << (int)buffer[i] << " ";
    }
    std::cerr << std::endl;
}

std::vector<unsigned char> RandomGenerator::getRandomBytes(int size)
{
    std::vector<unsigned char> randomBytes(size);
    get_random_bytes(randomBytes.data(), size);
    return randomBytes;
}

std::string RandomGenerator::getRandomHexBytes(int size)
{
    std::vector<unsigned char> bytes = getRandomBytes(size);
    std::ostringstream oss;
    for (unsigned char byte : bytes)
    {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)byte;
    }
    return oss.str();
}