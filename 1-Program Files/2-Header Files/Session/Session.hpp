#pragma once

#include <array>
#include <memory>
#include <string>
#include <cstddef>

#include <asio.hpp>

namespace sjcs {

    class Dispatcher;

    /**
     * Session
     *
     * Responsibility:
     *  - Own one TCP connection.
     *  - Read raw bytes from the socket.
     *  - Reconstruct complete JSON messages from the stream.
     *  - Forward parsed requests to Dispatcher.
     *  - Send JSON responses back to the client.
     *
     * Invariants:
     *  - Session owns its socket.
     *  - Session does not contain business logic.
     *  - Session lifetime is managed by std::shared_ptr.
     *  - Session must always be created by Server/acceptor logic.
     *
     * Threading:
     *  - Session operations run on the network worker thread.
     *  - stop() may be called from the CLI thread to request shutdown.
     * 
     * 
     * Note:
     *  - This header should stay as a contract only for now. The real implementation comes later when the 
     *    TCP server & acceptor logic is wired into Server & Session.
     */

    class Session : public std::enable_shared_from_this<Session> {
    public:
        using Socket = asio::ip::tcp::socket;

        Session(Socket socket, Dispatcher &dispatcher);

        Session(const Session &) = delete;
        Session &operator=(const Session &) = delete;

        void start();
        void stop() noexcept;

        std::string peer_id() const noexcept;

        ~Session() noexcept;

    private:
        void do_read();
        void handle_read(std::size_t bytes_transferred);
        void do_write(std::string response);

    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

} // namespace sjcs
