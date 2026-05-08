// Public API for bootstrapping and high-level control of the application.
// This header is the single include file used in Application.cpp (empty main will include it).

#pragma once

#include <cstdint>
//#include <iostream>
//#include <format>
#include <memory>

namespace sjcs {

    /**
     * Application
     *
     * High-level, minimal interface to wire system components together.
     * This header purposely exposes only a minimal top-level contract suitable for inclusion in main.
     *
     * Responsibilities:
     *  - Provide a factory method to create the application runtime object (opaque).
     *  - Provide start/stop hooks (not used in this minimal template).
     *
     * Notes:
     *  - Implementations remain header-only and internal; this contract is intentionally small.
     */

    class Application {
    public:
        /*@Brief:
        Create the application with the configured port; ownership returned via unique_ptr.
        Before binding your code to a port, ensure it's free. On Windows, you can use 
        netstat -ano | findstr :<port_number> in the command prompt to see if a specific port is in use.
        It is good practice in C++ to make the port a variable or command-line argument so you can quickly switch if your chosen port is blocked.
        If you are testing a client-server setup, let the operating system automatically assign an ephemeral port 
        (typically in the 49152–65535 range) to the client side.
        */
        static std::unique_ptr<Application> create(std::uint16_t port = 8000);

        // Start the application (non-blocking if desired).
        virtual void start() = 0;

        // Stop the application and join threads; idempotent.
        virtual void stop() noexcept = 0;

        // Run the CLI loop on the current thread (blocking). Optional.
        virtual void run_menu() = 0;

        virtual ~Application() noexcept = default;
    };

} // namespace sjcs
