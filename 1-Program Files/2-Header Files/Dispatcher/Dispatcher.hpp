// Public API contract for routing parsed JSON requests to handlers.

#pragma once

#include <functional>
#include <string>
#include <vector>
#include <memory>
#include <utility> 
//#include <unordered_map>

#include <nlohmannjason/json.hpp>

namespace sjcs {

    //These classes have been forward declared
    class AuthManager;
    class RateLimiter;

    /**
     * Dispatcher
     *
     * Responsibility:
     *  - Accept a parsed JSON request and return a JSON response.
     *  - Validate request envelope fields (e.g., "type" exists).
     *  - Map "type" -> handler functions.
     *
     * Invariants:
     *  - Dispatcher contains no networking code.
     *  - Handlers are synchronous: they run on the Network Worker thread and must be fast.
     *  - Unknown/invalid requests return structured JSON error objects.
     *
     * Threading:
     *  - Called from Network Worker thread only.
     *  - Does not internally lock any global state; handlers must use appropriate managers.
     */
    class Dispatcher {
    public:
        using json = nlohmann::json;
        using Handler = std::function<json(const json&)>;

        // Construct with references to business managers (Auth/RateLimiter).
        // Managers must outlive the Dispatcher in terms of lifetimes.
        explicit Dispatcher(AuthManager& auth, RateLimiter& rate_limiter) noexcept;

        Dispatcher(const Dispatcher&) = delete;
        Dispatcher& operator=(const Dispatcher&) = delete;

        // Dispatch a validated JSON request and return a JSON response.
        // This method performs envelope validation (presence of "type") before invoking a handler.
        json dispatch(const json& request);

        // Register a named handler. Used during initialization.
        // Overwrites an existing handler with the same name.
        void register_handler(std::string_view name, Handler handler) noexcept;

        // Query registered handlers (for admin introspection). Thread-safe for reading in network thread.
        std::vector<std::string> registered_handlers() const noexcept;

        ~Dispatcher() noexcept;

    private:
        struct Impl;
        std::unique_ptr<Impl> pimpl_;
    };

} // namespace sjcs
