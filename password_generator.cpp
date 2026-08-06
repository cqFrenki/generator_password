// password_generator.cpp
// генератор надёжных паролей
// сборка:   g++ -std=c++17 -O2 password_generator.cpp -o passgen
// запуск:   ./passgen   (Windows: passgen.exe)

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

class PasswordGenerator {
public:
    struct Options {
        bool lowercase = true;
        bool uppercase = true;
        bool digits    = true;
        bool symbols   = true;
    };

    explicit PasswordGenerator(const Options& opts) {
        if (opts.lowercase) sets_.push_back("abcdefghijklmnopqrstuvwxyz");
        if (opts.uppercase) sets_.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        if (opts.digits)    sets_.push_back("0123456789");
        if (opts.symbols)   sets_.push_back("!@#$%^&*()-_=+[]{}:;,.?");
        for (const auto& s : sets_) pool_ += s;
    }

    std::size_t poolSize() const { return pool_.size(); }

    std::string generate(std::size_t length) {
        std::string password;
        password.reserve(length);

        for (const auto& s : sets_)
            password += pick(s);

        while (password.size() < length)
            password += pick(pool_);

        std::shuffle(password.begin(), password.end(), rng_);
        return password;
    }

private:
    char pick(const std::string& s) {
        std::uniform_int_distribution<std::size_t> dist(0, s.size() - 1);
        return s[dist(rng_)];
    }

    std::vector<std::string> sets_;
    std::string pool_;

    std::random_device rng_;
};

static bool askYesNo(const std::string& question, bool defaultYes = true) {
    std::cout << question << (defaultYes ? " [Да/нет]: " : " [да/Нет]: ");
    std::string line;
    std::getline(std::cin, line);
    if (line.empty()) return defaultYes;

    unsigned char c0 = static_cast<unsigned char>(line[0]);
    char c = static_cast<char>(std::tolower(c0));
    if (c == 'y' || c == '1' || c == '+') return true;
    if (c == 'n' || c == '0' || c == '-') return false;

    if (line.size() >= 2 && c0 == 0xD0) {
        unsigned char c1 = static_cast<unsigned char>(line[1]);
        if (c1 == 0xB4 || c1 == 0x94) return true;   
        if (c1 == 0xBD || c1 == 0x9D) return false;  
    }
    return defaultYes;
}

static std::size_t askNumber(const std::string& question, std::size_t defVal,
                             std::size_t minVal, std::size_t maxVal) {
    std::cout << question << " [" << defVal << "]: ";
    std::string line;
    std::getline(std::cin, line);
    if (line.empty()) return defVal;
    try {
        long long v = std::stoll(line);
        if (v < static_cast<long long>(minVal)) {
            std::cout << "  (минимум " << minVal << ")\n";
            return minVal;
        }
        if (v > static_cast<long long>(maxVal)) {
            std::cout << "  (максимум " << maxVal << ")\n";
            return maxVal;
        }
        return static_cast<std::size_t>(v);
    } catch (...) {
        return defVal;
    }
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);   // корректный вывод кириллицы в консоли Windows
    SetConsoleCP(CP_UTF8);
#endif

    std::cout << "=== Генератор паролей ===\n\n";

    std::size_t length = askNumber("Длина пароля", 16, 4, 128);
    std::size_t count  = askNumber("Сколько паролей сгенерировать", 5, 1, 100);

    PasswordGenerator::Options opts;
    opts.lowercase = askYesNo("Использовать строчные буквы (a-z)?");
    opts.uppercase = askYesNo("Использовать заглавные буквы (A-Z)?");
    opts.digits    = askYesNo("Использовать цифры (0-9)?");
    opts.symbols   = askYesNo("Использовать спецсимволы (!@#$...)?");

    if (!opts.lowercase && !opts.uppercase && !opts.digits && !opts.symbols) {
        std::cerr << "\nОшибка: не выбран ни один набор символов.\n";
        return 1;
    }

    PasswordGenerator gen(opts);

    // верхняя оценка: формула считает выборку равномерной по всему пулу
    // и не учитывает гарантию из generate().
    double entropy = static_cast<double>(length) *
                     std::log2(static_cast<double>(gen.poolSize()));

    std::cout << "\nНадёжность: до ~" << static_cast<int>(entropy)
              << " бит энтропии на пароль\n\nВаши пароли:\n";

    for (std::size_t i = 1; i <= count; ++i)
        std::cout << "  " << i << ") " << gen.generate(length) << '\n';

#ifdef _WIN32
    std::cout << "\nНажмите Enter для выхода...";
    std::cin.get();
#endif

    return 0;
}
