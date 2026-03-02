// Public API contract for a simple rate limiter used to mitigate brute-force attempts.

#pragma once

#include <string_view>
#include <chrono>

namespace sjcs {

    /**
     * RateLimiter
     *
     * Responsibility:
     *  - Track last request time per key (e.g., username or IP) and allow/deny requests based on cooldown.
     *
     * Properties:
     *  - Non-blocking: allow() returns immediately (no sleeps).
     *  - Lightweight and in-memory; intended for learning/demo purposes.
     *
     * Threading:
     *  - allow() is thread-safe.
     */
    class RateLimiter {
    public:
        using clock = std::chrono::steady_clock;

        // Construct with a cooldown duration.
        explicit RateLimiter(std::chrono::milliseconds cooldown = std::chrono::milliseconds(1000)) noexcept;

        RateLimiter(const RateLimiter&) = delete;
        RateLimiter& operator=(const RateLimiter&) = delete;

        // Return true if a request associated with 'key' is allowed right now.
        // 'key' can be username, IP, or token depending on usage.
        bool allow(std::string_view key) noexcept;

        // Adjust cooldown at runtime (thread-safe).
        void set_cooldown(std::chrono::milliseconds cooldown) noexcept;

        // Query current cooldown.
        std::chrono::milliseconds cooldown() const noexcept;

        ~RateLimiter() noexcept;

    private:
        struct Impl;
        std::unique_ptr<Impl> pimpl_;
    };

} // namespace sjcs