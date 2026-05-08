// Public API contract for tracking active sessions (separate from AuthManager).

#pragma once

#include <Session/Session.hpp>

#include <cstddef>
#include <string>
#include <string_view>
#include <optional>
#include <memory>

namespace sjcs {

    /**
     * SessionManager
     *
     * Responsibility:
     *  - Maintain mapping token -> username for active sessions.
     *  - Provide admin introspection (list active sessions, count, etc.).
     *
     * Rationale:
     *  - Separation from AuthManager simplifies responsibilities (AuthManager handles tokens and auth logic;
     *    SessionManager focuses on lifecycle tracking / admin listing if desired).
     *
     * Threading:
     *  - All public methods are thread-safe.
     */
    class SessionManager {
    public:
        SessionManager() noexcept;
        SessionManager(const SessionManager&) = delete;
        SessionManager& operator=(const SessionManager&) = delete;

        // Add an active session token mapped to username.
        void add(std::string token, std::string username) noexcept;

        // Remove session by token.
        void remove(std::string_view token) noexcept;

        // Check existence of a token.
        bool exists(std::string_view token) const noexcept;

        // Get username mapped to token; returns nullopt if not found.
        std::optional<std::string> username_for(std::string_view token) const noexcept;

        // Total active session count.
        std::size_t active_count() const noexcept;

        ~SessionManager() noexcept;

    private:
        struct Impl;
        std::unique_ptr<Impl> pimpl_;
    };

} // namespace sjcs
