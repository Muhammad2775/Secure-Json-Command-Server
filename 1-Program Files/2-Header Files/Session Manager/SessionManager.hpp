#pragma once

#include <cstddef>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace sjcs {

    class SessionManager {
    public:
        SessionManager() noexcept = default;
        SessionManager(const SessionManager &) = delete;
        SessionManager &operator=(const SessionManager &) = delete;

        void add(std::string token, std::string username) noexcept {
            std::scoped_lock lock(mutex_);
            sessions_[std::move(token)] = std::move(username);
        }

        void remove(std::string_view token) noexcept {
            std::scoped_lock lock(mutex_);
            sessions_.erase(std::string(token));
        }

        bool exists(std::string_view token) const noexcept {
            std::scoped_lock lock(mutex_);
            return sessions_.find(std::string(token)) != sessions_.end();
        }

        std::optional<std::string> username_for(std::string_view token) const noexcept {
            std::scoped_lock lock(mutex_);
            const auto it = sessions_.find(std::string(token));
            if (it == sessions_.end()) {
                return std::nullopt;
            }
            return it->second;
        }

        std::size_t active_count() const noexcept {
            std::scoped_lock lock(mutex_);
            return sessions_.size();
        }

    private:
        mutable std::mutex mutex_;
        std::unordered_map<std::string, std::string> sessions_;
    };

} // namespace sjcs
