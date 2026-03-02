// Public API contract for deterministic hashing utilities used by AuthManager.

#pragma once

#include <string>
#include <string_view>

namespace sjcs {

    /**
     * Hashing
     *
     * Responsibility:
     *  - Provide deterministic hashing helper(s) (e.g., sha-like wrapper).
     *  - Provide salt generation helper.
     *
     * Threading:
     *  - Stateless functions are thread-safe.
     */
    struct Hashing {
        // Return a hex-encoded hash of input + optional salt.
        static std::string hash(std::string_view input, std::string_view salt = {}) noexcept;

        // Generate a cryptographically unique salt string. May use randomness.
        static std::string generate_saltStr() noexcept;
    };

} // namespace sjcs