#include "AutoParallel.hpp"
#include "ThreadsafeCout.hpp"
#include <array>
#include <chrono>
#include <string>
#include <thread>

using namespace std::chrono_literals;

std::string create(const char *str)
{
    ParCout{} << "Created string:" << str << '\n';
    std::this_thread::sleep_for(3s);

    return {str};
}

std::string concat(const std::string &first, const std::string &second)
{
    ParCout{} << "Concatinating two strings: " << first << " and " << second << '\n';
    std::this_thread::sleep_for(3s);

    return first + second;
}

std::string duplicate(const std::string &str)
{
    ParCout{} << "Duplicating string: " << str << '\n';
    std::this_thread::sleep_for(3s);

    return str + str;
}

int main()
{
    auto p_create{CreateAsynchronize(create)};
    auto p_concat{AsyncAdapter(concat)};
    auto p_duplicate{AsyncAdapter(duplicate)};

    auto result(p_duplicate(
        p_concat(p_duplicate(p_concat(p_create(" smoke "), p_create(" hookah "))),
                 p_create(" GraphLounge "))));

    std::cout << result().get() << '\n';

    return 0;
}
