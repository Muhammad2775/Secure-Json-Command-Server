#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <Authentication Manager/AuthManager.hpp>
#include <Dispatcher/Dispatcher.hpp>
#include <Rate Limiter/RateLimiter.hpp>
#include <Server/Server.hpp>
#include <Session Manager/SessionManager.hpp>

namespace sjcs {

    class Application final {
    public:
        static std::unique_ptr<Application> create(std::uint16_t port = 8000) {
            return std::unique_ptr<Application>(new Application(port));
        }

        Application(const Application &) = delete;
        Application &operator=(const Application &) = delete;

        int run() {
            if (!initialize()) {
                return 1;
            }

            running_.store(true, std::memory_order_release);
            server_.start();

            network_thread_ = std::jthread([this](std::stop_token st) { network_loop(st); });

            cli_thread_ = std::jthread([this](std::stop_token st) { show_menu(st); });

            main_loop();
            stop();

            return 0;
        }

        void stop() noexcept {
            const bool was_running = running_.exchange(false, std::memory_order_acq_rel);
            if (!was_running) {
                return;
            }

            server_.stop();

            if (network_thread_.joinable()) {
                network_thread_.request_stop();
            }

            if (cli_thread_.joinable()) {
                cli_thread_.request_stop();
            }

            if (cli_thread_.joinable()) {
                cli_thread_.join();
            }

            if (network_thread_.joinable()) {
                network_thread_.join();
            }
        }

        bool is_running() const noexcept { return running_.load(std::memory_order_acquire); }

        std::uint16_t port() const noexcept { return server_.port(); }

    private:
        explicit Application(std::uint16_t port) :
            server_{port}, sessions_{}, auth_{sessions_}, rate_limiter_{std::chrono::milliseconds{1000}},
            dispatcher_{auth_, rate_limiter_} {}

        bool initialize() {
            if (server_.port() == 0) {
                std::cerr << "Invalid port.\n";
                return false;
            }

            std::cout << "Initializing Secure JSON Command Server...\n";
            std::cout << "Port: " << server_.port() << '\n';
            std::cout << "Users: " << auth_.user_count() << '\n';
            std::cout << "Active sessions: " << auth_.active_session_count() << '\n';
            return true;
        }

        void main_loop() {
            using clock = std::chrono::steady_clock;
            using namespace std::chrono_literals;

            auto last_tick = clock::now();

            while (running_.load(std::memory_order_acquire)) {
                const auto now = clock::now();
                frame_time_ = now - last_tick;
                last_tick = now;

                ++frame_count_;
                std::this_thread::sleep_for(16ms);
            }
        }

        void network_loop(std::stop_token st) {
            std::cout << "Network Thread Started.\n";

            while (!st.stop_requested() && running_.load(std::memory_order_acquire)) {
                std::this_thread::sleep_for(std::chrono::milliseconds{50});
            }

            std::cout << "Network Thread Stopped.\n";
        }

        void show_menu(std::stop_token st) {
            std::cout << "CLI Thread Started.\n";

            while (!st.stop_requested() && running_.load(std::memory_order_acquire)) {
                {
                    std::scoped_lock lock(io_mutex_);
                    std::cout << "\nSecure JSON Command Server\n"
                              << "1. Runtime Status\n"
                              << "2. Register User\n"
                              << "3. Login User\n"
                              << "4. Validate Token\n"
                              << "5. Logout Token\n"
                              << "6. Ping\n"
                              << "7. Exit\n"
                              << "Select option: ";
                }

                int choice = 0;
                if (!(std::cin >> choice)) {
                    std::cin.clear();
                    std::cin.ignore(10000, '\n');
                    continue;
                }

                std::cin.ignore(10000, '\n');

                switch (choice) 
                {
                    case 1: {
                        std::scoped_lock lock(io_mutex_);
                        std::cout << "\n[STATUS]\n"
                                  << "Server running: " << (server_.is_running() ? "yes" : "no") << '\n'
                                  << "Users: " << auth_.user_count() << '\n'
                                  << "Active sessions: " << auth_.active_session_count() << '\n'
                                  << "Frames: " << frame_count_ << '\n';
                        break;
                    }

                    case 2: 
                    {
                        std::string username;
                        std::string password;

                        {
                            std::scoped_lock lock(io_mutex_);
                            std::cout << "Username: ";
                        }
                        std::getline(std::cin >> std::ws, username);

                        {
                            std::scoped_lock lock(io_mutex_);
                            std::cout << "Password: ";
                        }
                        std::getline(std::cin, password);

                        const bool is_valid = auth_.register_user(username, password);

                        std::scoped_lock lock(io_mutex_);
                        std::cout << (is_valid ? "Registration Successful.\n" : "Registration Failed.\n");
                        break;
                    }

                    case 3: 
                    {
                        std::string username;
                        std::string password;

                        {
                            std::scoped_lock lock(io_mutex_);
                            std::cout << "Username: ";
                        }
                        std::getline(std::cin >> std::ws, username);

                        {
                            std::scoped_lock lock(io_mutex_);
                            std::cout << "Password: ";
                        }
                        std::getline(std::cin, password);

                        auto token = auth_.login(username, password);

                        std::scoped_lock lock(io_mutex_);
                        if (token) {
                            last_token_ = *token;
                            std::cout << "Login successful.\nToken: " << *token << '\n';
                        } else {
                            std::cout << "Login Failed.\n";
                        }
                        break;
                    }

                    case 4: 
                    {
                        std::string token;

                        {
                            std::scoped_lock lock(io_mutex_);
                            std::cout << "Token (blank uses last token): ";
                        }
                        std::getline(std::cin >> std::ws, token);

                        if (token.empty()) {
                            token = last_token_;
                        }

                        const bool is_valid = auth_.validate_token(token);

                        std::scoped_lock lock(io_mutex_);
                        std::cout << (is_valid ? "Token is valid.\n" : "Token is invalid.\n");
                        break;
                    }

                    case 5: 
                    {
                        std::string token;

                        {
                            std::scoped_lock lock(io_mutex_);
                            std::cout << "Token (blank uses last token): ";
                        }
                        
                        std::getline(std::cin >> std::ws, token);

                        if (token.empty()) {
                            token = last_token_;
                        }

                        auth_.logout(token);

                        std::scoped_lock lock(io_mutex_);
                        std::cout << "Logout requested.\n";
                        break;
                    }

                    case 6: 
                    {
                        const auto response = dispatcher_.dispatch({{"type", "ping"}});
                        std::scoped_lock lock(io_mutex_);
                        std::cout << response.dump(2) << '\n';
                        break;
                    }

                    case 7:
                        std::cout << "Shutdown Requested.\n";
                        stop();
                        return;

                    default: 
                        std::scoped_lock lock(io_mutex_);
                        std::cout << "Invalid Option.\n";
                        break;
                    
                }
            }

            std::cout << "CLI Thread Stopped.\n";
        }

    private:
        Server server_;
        SessionManager sessions_;
        AuthManager auth_;
        RateLimiter rate_limiter_;
        Dispatcher dispatcher_;

        std::atomic_bool running_{false};
        std::jthread network_thread_;
        std::jthread cli_thread_;

        std::mutex io_mutex_;
        std::string last_token_;
        std::size_t frame_count_{0};
        std::chrono::steady_clock::duration frame_time_{};
    };

} // namespace sjcs
