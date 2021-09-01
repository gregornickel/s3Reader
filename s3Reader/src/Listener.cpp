#include "Listener.hpp"
#include "WebSocketSession.hpp"
#include <iostream>

Listener::Listener(asio::io_context& ioc, tcp::endpoint endpoint, std::shared_ptr<SharedState>& state)
    : acceptor_(ioc), socket_(ioc), state_(state)
{
    error_code ec;

    // Open the acceptor
    acceptor_.open(endpoint.protocol(), ec);
    if (ec) {
        fail(ec, "open");
        return;
    }

    // Allow address reuse
    acceptor_.set_option(asio::socket_base::reuse_address(true), ec);
    if (ec) {
        fail(ec, "set_option");
        return;
    }

    // Bind to the server address
    acceptor_.bind(endpoint, ec);
    if (ec) {
        fail(ec, "bind");
        return;
    }

    // Start listening for connections
    acceptor_.listen(asio::socket_base::max_listen_connections, ec);
    if (ec) {
        fail(ec, "listen");
        return;
    }
}

void Listener::run()
{
    // Start accepting incoming connections
    acceptor_.async_accept(socket_, [self = shared_from_this()](error_code ec) { self->accept(ec); });
}

void Listener::accept(error_code ec)
{
    if (ec)
        fail(ec, "accept");
    else
        // Launch a new session for this connection and run it
        std::make_shared<WebSocketSession>(std::move(socket_), state_)->run();

    // Accept another connection
    acceptor_.async_accept(socket_, [self = shared_from_this()](error_code ec) { self->accept(ec); });
}

// Report a failure
void Listener::fail(beast::error_code ec, char const* what) { std::cerr << what << ": " << ec.message() << "\n"; }