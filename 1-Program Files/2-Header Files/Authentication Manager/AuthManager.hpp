// Public API contract for user registration, login, and session token lifecycle.

#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <optional>
#include <memory>

namespace sjcs {

    /**
     * User record (immutable once created via register_user).
     * Note: password_hash must contain a hashed+salted password; no plaintext kept.
     */

    struct User {std::string username{}, password_hash{};};

    /**
     * AuthManager
     *
     * Responsibility:
     *  - Register users (in-memory).
     *  - Authenticate users (username+password) and issue session tokens.
     *  - Validate and revoke session tokens.
     *
     * Invariants:
     *  - No plaintext password is persisted after register_user returns.
     *  - Tokens are opaque strings that must be unique.
     *  - Token validation does not mutate state.
     *
     * Threading:
     *  - All public methods are thread-safe (guarded internally).
     *  - Used concurrently by Network Worker (login requests) and CLI thread (admin commands).
     */

    class AuthManager {
    public:
        AuthManager() noexcept;
        AuthManager(const AuthManager&) = delete;
        AuthManager& operator=(const AuthManager&) = delete;

        // Register a new user. Returns false if the username is already taken or invalid.
        bool register_user(std::string_view username, std::string_view password);

        // Authenticate and return session token on success.
        // Returns std::nullopt on failure.
        std::optional<std::string> login(std::string_view username, std::string_view password);

        // Validate an existing session token without mutating state.
        [[nodiscard]] bool validate_token(std::string_view token) const noexcept;

        // Revoke an active session token.
        void logout(std::string_view token) noexcept;

        // Number of registered users (approximate, thread-safe).
        [[nodiscard]]  std::size_t user_count() const noexcept;

        // Number of active sessions (thread-safe).
        [[nodiscard]] std::size_t active_session_count() const noexcept;

        ~AuthManager() noexcept;

    private:
        // Internal helpers (not visible to users of the API).
        [[nodiscard]] std::string hash_password(std::string_view password) const;
        [[nodiscard]] std::string generate_token() const;

        struct Impl;
        std::unique_ptr<Impl> pimpl_{nullptr};
    };

} // namespace sjcs