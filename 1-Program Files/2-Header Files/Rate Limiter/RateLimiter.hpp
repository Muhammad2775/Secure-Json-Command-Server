#pragma once

#include <chrono>
#include <mutex>
#include <string_view>
#include <unordered_map>

namespace sjcs {

    class RateLimiter {
    public:
        using clock = std::chrono::steady_clock;

        explicit RateLimiter(std::chrono::milliseconds cooldown = std::chrono::milliseconds{1000}) noexcept :
            cooldown_{cooldown} {}

        RateLimiter(const RateLimiter &) = delete;
        RateLimiter &operator=(const RateLimiter &) = delete;

        bool allow(std::string_view key) noexcept {
            const auto now = clock::now();
            std::scoped_lock lock(mutex_);

            auto it = last_request_.find(std::string(key));
            if (it == last_request_.end()) {
                last_request_[std::string(key)] = now;
                return true;
            }

            if (now - it->second >= cooldown_) {
                it->second = now;
                return true;
            }

            return false;
        }

        void set_cooldown(std::chrono::milliseconds cooldown) noexcept {
            std::scoped_lock lock(mutex_);
            cooldown_ = cooldown;
        }

        std::chrono::milliseconds cooldown() const noexcept {
            std::scoped_lock lock(mutex_);
            return cooldown_;
        }

    private:
        mutable std::mutex mutex_;
        std::unordered_map<std::string, clock::time_point> last_request_;
        std::chrono::milliseconds cooldown_;
    };

} // namespace sjcs
