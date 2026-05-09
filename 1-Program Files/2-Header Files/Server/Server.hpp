#pragma once

#include <atomic>
#include <cstdint>

namespace sjcs {

    /**
     * Server
     *
     * Phase 1 responsibility:
     *  - Own the server lifecycle state.
     *  - Track the configured port.
     *  - Expose start/stop/is_running for the runtime controller.
     *
     * This is intentionally lightweight for now.
     * Networking internals can be added in later phases without changing the public shape.
     */
    class Server {
    public:
        explicit Server(std::uint16_t port = 8000) noexcept : port_{port} {}

        Server(const Server &) = delete;
        Server &operator=(const Server &) = delete;

        void start() noexcept { running_.store(true, std::memory_order_release); }

        void stop() noexcept { running_.store(false, std::memory_order_release); }

        bool is_running() const noexcept { return running_.load(std::memory_order_acquire); }

        std::uint16_t port() const noexcept { return port_; }

    private:
        std::uint16_t port_{8000};
        std::atomic_bool running_{false};
    };

} // namespace sjcs
