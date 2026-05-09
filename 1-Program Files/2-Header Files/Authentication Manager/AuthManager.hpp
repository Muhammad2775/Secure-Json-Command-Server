#pragma once

#include <cstddef>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include <Hashing/Hashing.hpp>
#include <Session Manager/SessionManager.hpp>

namespace sjcs {

    struct User {
        std::string username{};
        std::string password_hash{};
    };

    class AuthManager {
    public:
        explicit AuthManager(SessionManager &sessions) noexcept : sessions_{sessions} {}

        AuthManager(const AuthManager &) = delete;
        AuthManager &operator=(const AuthManager &) = delete;

        bool register_user(std::string_view username, std::string_view password) {
            if (username.empty() || password.empty()) {
                return false;
            }

            std::scoped_lock lock(mutex_);
            const std::string uname{username};

            if (users_.find(uname) != users_.end()) {
                return false;
            }

            const std::string salt = Hashing::generate_salt();
            const std::string hashed = Hashing::hash(password, salt);

            users_.emplace(uname, User{uname, salt + ":" + hashed});
            return true;
        }

        std::optional<std::string> login(std::string_view username, std::string_view password) {
            if (username.empty() || password.empty()) {
                return std::nullopt;
            }

            std::scoped_lock lock(mutex_);
            const std::string uname{username};

            const auto it = users_.find(uname);
            if (it == users_.end()) {
                return std::nullopt;
            }

            const auto sep = it->second.password_hash.find(':');
            if (sep == std::string::npos) {
                return std::nullopt;
            }

            const std::string salt = it->second.password_hash.substr(0, sep);
            const std::string expected = it->second.password_hash.substr(sep + 1);
            const std::string actual = Hashing::hash(password, salt);

            if (actual != expected) {
                return std::nullopt;
            }

            const std::string token = generate_token_locked(uname);
            sessions_.add(token, uname);
            return token;
        }

        bool validate_token(std::string_view token) const noexcept { return sessions_.exists(token); }

        void logout(std::string_view token) noexcept { sessions_.remove(token); }

        std::optional<std::string> username_for_token(std::string_view token) const noexcept {
            return sessions_.username_for(token);
        }

        std::size_t user_count() const noexcept {
            std::scoped_lock lock(mutex_);
            return users_.size();
        }

        std::size_t active_session_count() const noexcept { return sessions_.active_count(); }

    private:
        std::string generate_token_locked(std::string_view username) const {
            const std::string salt = Hashing::generate_salt();
            return Hashing::hash(username, salt) + "-" + salt;
        }

    private:
        mutable std::mutex mutex_;
        std::unordered_map<std::string, User> users_;
        SessionManager &sessions_;
    };

} // namespace sjcs
