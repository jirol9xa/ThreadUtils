#pragma once
#include <future>
#include <thread>

// TODO: Make possible controlling LatchedThreadRAII with shared_future

/// Class for implementing (not delayed) thread with action on end of lifetime (join or
/// detach) That simple realisation guarantees, that programm will not stop abruptly due
/// to a destructor call in the attachable thread
class InstantThreadRAII
{
  public:
    enum class Action
    {
        join,
        detach
    };

    InstantThreadRAII() = delete;
    InstantThreadRAII(std::thread &&thread, Action action)
        : action_(action), thread_(std::move(thread))
    {
    }

    InstantThreadRAII(const InstantThreadRAII &)            = delete;
    InstantThreadRAII &operator=(const InstantThreadRAII &) = delete;

    InstantThreadRAII(InstantThreadRAII &&)            = default;
    InstantThreadRAII &operator=(InstantThreadRAII &&) = default;

    std::thread &get() { return thread_; }

    ~InstantThreadRAII()
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

template <typename T> std::decay_t<T> decay_copy(T &&val) { return std::forward<T>(val); }

/// Class for implementing thread, that was delayed until std::promise will be setted
/// That simple realisation unlike the previous one guarantees, that the std::promise
/// will be setted automatically if an exception was thrown in parent thread.
/// That class can't gurantees safety for all cases, so use it carefully.
class LatchedThreadRAII
{
  public:
    enum Action
    {
        JOIN,
        DETACH
    };

  private:
    using promise_t = std::promise<bool>;
    using thread_t  = std::thread;

    enum execution_state
    {
        WAITING,
        EXECUTING,
        DISMISSED
    };

    execution_state state_ = WAITING;
    promise_t       should_execute_;
    thread_t        thread_;
    Action          action_ = Action::JOIN;

  public:
    using id                 = thread_t::id;
    using native_handle_type = thread_t::native_handle_type;

    LatchedThreadRAII()                          = delete;
    LatchedThreadRAII(const LatchedThreadRAII &) = delete;

    LatchedThreadRAII(LatchedThreadRAII &&rhs)
        : state_(std::move(rhs.state_)),
          should_execute_(std::move(rhs.should_execute_)),
          thread_(std::move(rhs.thread_)),
          action_(std::move(rhs.action_))
    {
        rhs.state_ = DISMISSED;
    }

    template <typename op_t>
    LatchedThreadRAII(op_t &&op)
        : should_execute_(),
          thread_(
              [op             = decay_copy(std::forward<op_t>(op)),
               should_execute = should_execute_.get_future()]() mutable
              {
                  if (should_execute.get())
                      op();
              })
    {
    }

    LatchedThreadRAII &operator=(const LatchedThreadRAII &) = delete;
    LatchedThreadRAII &operator=(LatchedThreadRAII &&rhs)
    {
        state_          = std::move(rhs.state_);
        should_execute_ = std::move(rhs.should_execute_);
        thread_         = std::move(rhs.thread_);
        action_         = std::move(rhs.action_);

        rhs.state_ = DISMISSED;
        return *this;
    }

    bool has_started() const { return state_ == EXECUTING; }

    void execute()
    {
        if (state_ == WAITING)
        {
            state_ = EXECUTING;
            should_execute_.set_value(true);
        }
    }

    void join()
    {
        if (!has_started())
            throw std::system_error(
                std::make_error_code(std::errc::resource_deadlock_would_occur));

        // Unsafety of double join is under your responsobility
        thread_.join();
    }

    void detach()
    {
        if (!has_started())
            throw std::system_error(std::make_error_code(std::errc::invalid_argument));

        // Unsafety of double detach is under your responsobility
        thread_.detach();
    }

    void swap(LatchedThreadRAII &other)
    {
        using std::swap;

        swap(state_, other.state_);
        swap(should_execute_, other.should_execute_);
        swap(thread_, other.thread_);
        swap(action_, other.action_);
    }

    bool joinable() const { return has_started() && thread_.joinable(); }

    id get_id() const { return thread_.get_id(); }

    native_handle_type native_handle() { return thread_.native_handle(); }

    ~LatchedThreadRAII()
    {
        if (state_ == WAITING)
        {
            // If thread is waiting, but the caller is ended
            should_execute_.set_value(false);
            if (thread_.joinable())
                thread_.detach();

            return;
        }

        if (thread_.joinable())
        {

            if (action_ == Action::JOIN)
                thread_.join();
            else
                thread_.detach();
        }
    }
};

// Std injection
namespace std
{
template <> void swap(LatchedThreadRAII &first, LatchedThreadRAII &second)
{
    first.swap(second);
}
} // namespace std
