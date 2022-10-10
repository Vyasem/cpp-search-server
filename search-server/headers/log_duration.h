#pragma once

#include <chrono>
#include <iostream>
#include <string_view>

#define PROFILE_CONCAT_INTERNAL(X, Y) X##Y
#define PROFILE_CONCAT(X, Y) PROFILE_CONCAT_INTERNAL(X, Y)
#define UNIQUE_VAR_NAME_PROFILE PROFILE_CONCAT(profileGuard, __LINE__)
#define LOG_DURATION(x) LogDuration UNIQUE_VAR_NAME_PROFILE(x)
#define LOG_DURATION_STREAM(x, y) LogDuration UNIQUE_VAR_NAME_PROFILE(x, y)

class LogDuration {
public:
    using Clock = std::chrono::steady_clock;
    LogDuration(std::string_view id): id_(id),os_(std::cerr){}
    LogDuration(std::string_view id, std::ostream& os): id_(id),os_(os){}
    ~LogDuration() {
        using namespace std::chrono;
        using namespace std::literals;
        const auto endTime = Clock::now();
        const auto dur = endTime - startTime_;
        os_ << id_ << ": "s << duration_cast<milliseconds>(dur).count() << " ms"s << std::endl;
    }
private:
    std::string_view id_;
    std::ostream& os_;
    const Clock::time_point startTime_ = Clock::now();
};
