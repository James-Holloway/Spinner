#ifndef SPINNER_SCOPEDTIMER_HPP
#define SPINNER_SCOPEDTIMER_HPP
#include <chrono>
#include <utility>

class ScopedTimer
{
public:
    using Clock = std::chrono::high_resolution_clock;

    inline static bool Print = false; // Set on App creation and before main App::Run loop

    explicit ScopedTimer(std::string timerName) : Name(std::move(timerName))
    {
        Start = Clock::now();
        Depth++;
    }

    ~ScopedTimer()
    {
        Depth--;

        if (Print)
        {
            const auto end = Clock::now();
            const auto seconds = std::chrono::duration<double, std::chrono::seconds::period>(end - Start).count();
            for (int i = 0; i < Depth; i++)
            {
                std::clog << "  ";
            }
            std::clog << "> " << Name << ": " << seconds << " seconds\n";
        }
    }

    const ScopedTimer &operator =(const ScopedTimer &) = delete;
    const ScopedTimer &operator =(ScopedTimer &&) = delete;

private:
    std::string Name;
    Clock::time_point Start;

    inline static thread_local int Depth = 0;
};

#endif
