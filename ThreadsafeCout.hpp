#pragma once

#include <mutex>
#include <iostream>
#include <sstream>

class ParCout : public std::stringstream {
    static inline std::mutex m_;

public:
    ~ParCout() {
        std::lock_guard lck(m_);
        std::cout << rdbuf();
    }
};
