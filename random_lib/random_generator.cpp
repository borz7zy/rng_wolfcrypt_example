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
        std::cerr << "Failed to initialize RNG" << std::endl;
        exit(1);
    }

    bbs_init(keySize, passwordFile, passwordFile.length());
    for (int i = 0; i <= 64; i++)
    {
        bbs_next_random_byte();
    }
}

RandomGenerator::~RandomGenerator()
{
    wc_FreeRng(&rng);
}

void RandomGenerator::prng_seed(const std::string &password_filename, int password_length)
{
    std::vector<unsigned char> a(64 + password_length);
    long long r = []()
    {
        unsigned long long tsc;
#ifdef _WIN32
        tsc = GetTickCount64();
#elif defined(__x86_64__) || defined(__i386__)
        asm volatile("rdtsc" : "=A"(tsc));
#endif
        return tsc;
    }();

    struct timespec T;
#ifdef _WIN32
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER ul;
    ul.LowPart = ft.dwLowDateTime;
    ul.HighPart = ft.dwHighDateTime;
    T.tv_sec = (ul.QuadPart - 116444736000000000LL) / 10000000;
    T.tv_nsec = 0;
#else
    assert(clock_gettime(CLOCK_REALTIME, &T) >= 0);
#endif

    memcpy(a.data(), &T.tv_sec, 4);
    memcpy(a.data() + 4, &T.tv_nsec, 4);
    memcpy(a.data() + 8, &r, 8);
    unsigned short p = getpid();
    memcpy(a.data() + 16, &p, 2);
    int s = get_random_bytes(a.data() + 18, 32) + 18;

#ifdef _WIN32
    HANDLE hFile = CreateFile(password_filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        std::cerr << "Warning: fail to open password file - \"" << password_filename << "\", " << GetLastError() << "." << std::endl;
    }
    else
    {
        DWORD bytesRead;
        ReadFile(hFile, a.data() + s, password_length, &bytesRead, NULL);
        if (bytesRead == 0)
        {
            std::cerr << "Warning: fail to read password file - \"" << password_filename << "\", " << GetLastError() << "." << std::endl;
        }
        else
        {
            std::cerr << "read " << bytesRead << " bytes from password file." << std::endl;
            s += bytesRead;
        }
        CloseHandle(hFile);
    }
#else
    int fd = open(password_filename.c_str(), O_RDONLY);
    if (fd < 0)
    {
        std::cerr << "Warning: fail to open password file - \"" << password_filename << "\", " << strerror(errno) << "." << std::endl;
    }
    else
    {
        int l = read(fd, a.data() + s, password_length);
        if (l < 0)
        {
            std::cerr << "Warning: fail to read password file - \"" << password_filename << "\", " << strerror(errno) << "." << std::endl;
        }
        else
        {
            std::cerr << "read " << l << " bytes from password file." << std::endl;
            s += l;
        }
        close(fd);
    }
#endif

    if (wc_RNG_GenerateBlock(&rng, a.data(), a.size()) != 0)
    {
        std::cerr << "Failed to seed RNG" << std::endl;
        exit(1);
    }
}

int RandomGenerator::get_random_bytes(void *buf, int n)
{
    int r = 0;
    if (wc_RNG_GenerateBlock(&rng, (unsigned char *)buf, n) != 0)
    {
        std::cerr << "Failed to generate random bytes" << std::endl;
        return 0;
    }
    return n;
}

int RandomGenerator::bbs_init(int bits, const std::string &password_filename, int password_length)
{
    prng_seed(password_filename, password_length);
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

void RandomGenerator::saveKey(const std::string &passwordFile)
{
    std::ofstream out(passwordFile, std::ios::binary);
    if (out.is_open())
    {
        out.write(reinterpret_cast<char *>(buffer.data()), buffer.size());
        out.close();
    }
    else
    {
        std::cerr << "Failed to save key to file: " << passwordFile << std::endl;
    }
}
