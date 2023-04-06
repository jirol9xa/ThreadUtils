#pragma once
#include <thread>

/// Class for implementing thread with action on end of lifetime (join or detach)
/// That simple realisation guarantees, that programm will not stop abruptly due to
/// a destructor call in the attachable thread
class ThreadRAII
{
  public:
    enum Action
    {
        join,
        detach
    };

    ThreadRAII(std::thread &&thread, Action action)
        : action_(action), thread_(std::move(thread))
    {
    }

    ThreadRAII(ThreadRAII &&rhs)            = default;
    ThreadRAII &operator=(ThreadRAII &&rhs) = default;

    std::thread &get() { return thread_; }

    ~ThreadRAII()
    {
        if (thread_.joinable())
        {
            if (action_ == Action::join)
                thread_.join();
            else
                thread_.detach();
        }
    }

  private:
    Action      action_;
    std::thread thread_;
};
