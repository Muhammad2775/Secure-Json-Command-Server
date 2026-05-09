#pragma once

#include <cstdint>
#include <functional>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>
#include <string_view>

namespace sjcs {

    struct Hashing {
        static std::string hash(std::string_view input, std::string_view salt = {}) noexcept {
            try {
                const std::hash<std::string_view> hasher{};
                const std::uint64_t a = static_cast<std::uint64_t>(hasher(input));
                const std::uint64_t b = static_cast<std::uint64_t>(hasher(salt));

                std::uint64_t value = a ^ (b + 0x9e3779b97f4a7c15ULL + (a << 6U) + (a >> 2U));

                std::ostringstream oss;
                oss << std::hex << std::setw(16) << std::setfill('0') << value;
                return oss.str();
            } catch (...) {
                return {};
            }
        }

        static std::string generate_salt() noexcept {
            try {
                static thread_local std::mt19937_64 rng{std::random_device{}()};
                std::uniform_int_distribution<std::uint64_t> dist;

                std::uint64_t value = dist(rng);

                std::ostringstream oss;
                oss << std::hex << value;
                return oss.str();
            } catch (...) {
                return "salt";
            }
        }
    };

} // namespace sjcs
