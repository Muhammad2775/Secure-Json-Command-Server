# Secure JSON Command Server

## Overview

This project is a *minimal header-only secure TCP command server* written in modern C++26. It demonstrates core concepts of networking, structured protocol design, controlled multi-threading, authentication, and rate limiting without relying on external binary dependencies or complex frameworks.

The system is designed for *learning and architectural discipline*. Clients communicate with the server using a JSON-based protocol over TCP. The server processes commands, manages user authentication in memory, issues session tokens, and enforces basic security mechanisms such as password hashing and rate limiting.

The project is fully CLI-based, self-contained, and uses only two `std::jthread`s to maintain strict concurrency boundaries. All logic is implemented inside header files, with `Application.cpp` acting only as the program bootstrapper.

---

## Features

- TCP server built using standalone Asio (header-only).
- JSON-based protocol using nlohmann/json (single header).
- In-memory user registration and authentication.
- Session token generation and validation.
- Rate limiting for brute-force mitigation.
- Strict two-thread concurrency model using `std::jthread`.
- CLI menu for server control and test execution.
- Header-only architecture for disciplined modular design.
- Feature-based test cases (one test per feature).
- Zero external binary dependencies.

---

## Project Structure

    Secure JSON Command Server/
    ├── Program Files/
    ├── Source Files/
    │ └── Application/
    │   └── Application.cpp
    ├── Header Files/
    │ ├── Server/
    │ │ └── Server.hpp
    │ ├── Session/
    │ │ └── Session.hpp
    │ ├── Dispatcher/
    │ │ └── Dispatcher.hpp
    │ ├── AuthManager/
    │ │ └── AuthManager.hpp
    │ ├── SessionManager/
    │ │ └── SessionManager.hpp
    │ ├── RateLimiter/
    │ │ └── RateLimiter.hpp
    │ ├── Hashing/
    │ │ └── Hashing.hpp
    │ └── SystemInterface/
    │   └── SystemInterface.hpp
    ├── External Dependancies/
    │ ├── Boost/
    │ │ └── boost/asio.hpp
    │ └── nlohmannjason/
    │   └── jason.hpp
    │ ├── Test Cases/
    │   └── AuthTest.hpp
    ├── Configuration/
    │ ├── CMakeLists.txt
    ├── Test Cases/
    │ └── AuthTest.hpp
    │ └── DispatcherTest.hpp
    │ └── RateLimiterTest.hpp
    │ └── ProtocolTest.hpp
    └── Documentation/
    └── ReadMe.md

---

## Concurrency Model

The system enforces a strict two-thread design:

- **Thread 1 – Network Worker**
  - Runs the Asio `io_context`
  - Accepts TCP connections
  - Handles asynchronous reads and writes

- **Thread 2 – CLI Controller**
  - Provides interactive server control
  - Displays runtime information
  - Executes feature test cases
  - Initiates graceful shutdown

All shared state (users, sessions, rate limits) is protected using `std::mutex`.

No additional threads are allowed.

---

## JSON Protocol

All communication occurs via structured JSON messages over TCP.

### Example: Register

Request:
```json
{
  "type": "register",
  "username": "user1",
  "password": "password123"
}
```

Response:
```json
{
  "status": "ok"
}
```

### Example: Login

Request:
```json
{
  "type": "login",
  "username": "user1",
  "password": "password123"
}
```

Response:
```json
{
  "status": "ok",
  "session_token": "generated_token"
}
```

### Example: Authenticated Command

```json
{
  "type": "some_command",
  "session_token": "generated_token"
}
```


All requests are validated before processing.
