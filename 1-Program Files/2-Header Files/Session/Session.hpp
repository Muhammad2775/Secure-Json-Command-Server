// Public API contract for a single client connection (session).

#pragma once

#include <memory>
#include <string>
//#include <array>
//#include <cstddef>
//#include <boost/include/asio.hpp>

namespace boost::asio {class ip;}

namespace sjcs {

    class Dispatcher; // forward

    /**
     * Session
     *
     * Responsibility:
     *  - Own a single TCP socket and manage async read/write cycles.
     *  - Parse raw bytes into JSON frames (length-prefixed or other agreed framing).
     *  - Forward parsed requests to Dispatcher and send back JSON responses.
     *
     * Invariants:
     *  - Session must not contain business logic (only forward to Dispatcher).
     *  - Session owns its socket and buffers.
     *  - Session lifetime controlled by shared_ptr and server/acceptor.
     *
     * Threading:
     *  - All Session operations run in the Network Worker thread.
     *  - Public stop() may be invoked from the CLI thread to request immediate close.
     */
    class Session : public std::enable_shared_from_this<Session> {
    public:
        // TCP socket type alias for clarity
        using Socket = boost::asio::ip;

        // Construct with a connected socket and reference to dispatcher (must outlive session).
        Session(Socket socket, Dispatcher& dispatcher) noexcept;

        // Non-copyable
        Session(const Session&) = delete;
        Session& operator=(const Session&) = delete;

        // Start the async read cycle. Called from network thread after construction.
        void start();

        // Request immediate stop and close socket safely. Can be called from any thread.
        void stop() noexcept;

        // Returns remote peer identifier (human-readable); non-throwing.
        std::string peer_id() const noexcept;

        ~Session() noexcept;

    private:
        struct Impl;
        std::unique_ptr<Impl> pimpl_;
    };
} // namespace sjcs