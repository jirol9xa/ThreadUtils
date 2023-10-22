#include <thread>
#include <array>
#include "ThreadsafeCout.hpp"

void threadFunc(int thread_id) {
    ParCout{} << "Thread number " << thread_id << " said hello!\n";
}

int main() {
    std::array<std::thread, 100> threads;
    for (int i = 0; i < threads.size(); ++i) 
        threads[i] = std::thread{threadFunc, i};
    for (auto &thread : threads)
        thread.join();

    return 0;
}
