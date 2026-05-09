#pragma once

#include <string>
#include <string_view>
#include <functional>
#include <unordered_map>

#include <Authentication Manager/AuthManager.hpp>
#include <Rate Limiter/RateLimiter.hpp>
#include <json.hpp>

namespace sjcs {

    class Dispatcher {
    public:
        using json = nlohmann::json;
        using Handler = std::function<json(const json &)>;

        Dispatcher(AuthManager &auth, RateLimiter &rate_limiter) noexcept : auth_{auth}, rate_limiter_{rate_limiter} {
            register_handlers();
        }

        Dispatcher(const Dispatcher &) = delete;
        Dispatcher &operator=(const Dispatcher &) = delete;

        json dispatch(const json &request) {
            if (!request.is_object() || !request.contains("type") || !request["type"].is_string()) {
                return make_error("invalid_request");
            }

            const std::string type = request["type"].get<std::string>();
            const auto it = handlers_.find(type);
            if (it == handlers_.end()) {
                return make_error("unknown_command");
            }

            return it->second(request);
        }

        void register_handler(std::string_view name, Handler handler) noexcept {
            handlers_[std::string(name)] = std::move(handler);
        }

    private:
        void register_handlers() {
            register_handler("ping", [this](const json &) { return json{{"status", "ok"}, {"result", "pong"}}; });

            register_handler("register", [this](const json &request) {
                if (!request.contains("username") || !request.contains("password")) {
                    return make_error("missing_fields");
                }

                const std::string username = request["username"].get<std::string>();
                const std::string password = request["password"].get<std::string>();

                if (!rate_limiter_.allow(username)) {
                    return make_error("rate_limited");
                }

                if (!auth_.register_user(username, password)) {
                    return make_error("registration_failed");
                }

                return json{{"status", "ok"}};
            });

            register_handler("login", [this](const json &request) {
                if (!request.contains("username") || !request.contains("password")) {
                    return make_error("missing_fields");
                }

                const std::string username = request["username"].get<std::string>();
                const std::string password = request["password"].get<std::string>();

                if (!rate_limiter_.allow(username)) {
                    return make_error("rate_limited");
                }

                auto token = auth_.login(username, password);
                if (!token) {
                    return make_error("invalid_credentials");
                }

                return json{{"status", "ok"}, {"session_token", *token}};
            });

            register_handler("logout", [this](const json &request) {
                if (!request.contains("session_token")) {
                    return make_error("missing_fields");
                }

                const std::string token = request["session_token"].get<std::string>();
                auth_.logout(token);

                return json{{"status", "ok"}};
            });

            register_handler("whoami", [this](const json &request) {
                if (!request.contains("session_token")) {
                    return make_error("missing_fields");
                }

                const std::string token = request["session_token"].get<std::string>();
                const auto username = auth_.username_for_token(token);

                if (!username) {
                    return make_error("unauthorized");
                }

                return json{{"status", "ok"}, {"username", *username}};
            });
        }

        static json make_error(std::string_view message) { return json{{"status", "error"}, {"message", message}}; }

    private:
        AuthManager &auth_;
        RateLimiter &rate_limiter_;
        std::unordered_map<std::string, Handler> handlers_;
    };

} // namespace sjcs
