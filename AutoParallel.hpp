#pragma once

#include <future>
#include <thread>

static auto CreateAsynchronize(auto func)
{
    return [func](auto... args) {
        return [=]() { return std::async(std::launch::async, func, args...); };
    };
}

static auto UnwrapFuture(auto func)
{
    return [func](auto... args) { return func(args.get()...); };
}

static auto AsyncAdapter(auto func)
{
    return [func](auto... args) {
        return [=]() {
            return std::async(std::launch::async, UnwrapFuture(func), args()...);
        };
    };
}
