// Public API contract for the networking acceptor and lifecycle controller.

#pragma once

#include <cstdint>
#include <atomic>
#include <memory>

namespace sjcs {

    namespace net { class IoContextWrapper; } // forward (implementation detail)

    /**
     * Server
     *
     * Responsibility:
     *  - Own and manage the acceptor and io_context for all incoming connections.
     *  - Create Session instances (declared elsewhere) for each accepted connection.
     *
     * Invariants:
     *  - start() is idempotent.
     *  - stop() may be called concurrently from the CLI thread.
     *  - Server does not perform authentication or command processing itself.
     *
     * Threading:
     *  - start()/stop() are safe to call from the CLI thread.
     *  - Accept loop runs in the Network Worker thread only.
     */
    class Server {
    public:
        // Construct with pre-created io context wrapper and TCP port.
        // The io context must outlive this Server instance.
        Server(net::IoContextWrapper& ios, std::uint16_t port) noexcept;

        // Non-copyable, non-movable
        Server(const Server&) = delete;
        Server& operator=(const Server&) = delete;

        // Start accepting connections. Idempotent.
        void start();

        // Initiate graceful shutdown. Safe to call from any thread.
        void stop();

        // Query running state without locking.
        bool is_running() const noexcept;

        // Destructor performs final cleanup/shutdown (no-throw).
        ~Server() noexcept;

    private:
        struct Impl;
        std::unique_ptr<Impl> pimpl_;
        std::atomic_bool running_{false};
    };

} // namespace sjcs