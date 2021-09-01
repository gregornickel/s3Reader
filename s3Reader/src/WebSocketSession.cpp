#include "WebSocketSession.hpp"
#include <iostream>

// Echoes back all received WebSocket messages
WebSocketSession::WebSocketSession(tcp::socket socket, std::shared_ptr<SharedState> const& state)
    : ws_(std::move(socket)), state_(state)
{
}

WebSocketSession::~WebSocketSession()
{
    // Remove this session from the list of active sessions
    state_->leave(*this);
}

void WebSocketSession::run()
{
    // Accept the websocket handshake
    ws_.async_accept(beast::bind_front_handler(&WebSocketSession::accept, shared_from_this()));
}

void WebSocketSession::send(std::shared_ptr<std::string const> const& ss)
{
    // Always add to queue
    queue_.push_back(ss);

    // Are we already writing?
    if (queue_.size() > 1)
        return;

    // We are not currently writing, so send this immediately
    ws_.async_write(asio::buffer(*queue_.front()),
                    [sp = shared_from_this()](error_code ec, std::size_t bytes) { sp->write(ec, bytes); });
}

// Report a failure
void WebSocketSession::fail(error_code ec, char const* what)
{
    // Don't report these
    if (ec == asio::error::operation_aborted || ec == websocket::error::closed)
        return;

    std::cerr << what << ": " << ec.message() << "\n";
}

void WebSocketSession::accept(beast::error_code ec)
{
    if (ec)
        return fail(ec, "accept");

    // Add this session to the list of active sessions
    state_->join(*this);

    // Keep io_service alive
    ws_.async_read(buffer_, [sp = shared_from_this()](error_code ec, std::size_t bytes) { sp->read(ec, bytes); });
}

void WebSocketSession::read(beast::error_code ec, std::size_t bytes_transferred)
{
    if (ec)
        return fail(ec, "read");

    // Send to all connections
    state_->send(beast::buffers_to_string(buffer_.data()));

    // Clear the buffer
    buffer_.consume(buffer_.size());

    // Read another message
    ws_.async_read(buffer_, [sp = shared_from_this()](error_code ec, std::size_t bytes) { sp->read(ec, bytes); });
}

void WebSocketSession::write(beast::error_code ec, std::size_t bytes_transferred)
{
    if (ec)
        return fail(ec, "write");

    // Remove the string from the queue
    queue_.erase(queue_.begin());

    // Send the next message if any
    if (!queue_.empty())
        ws_.async_write(asio::buffer(*queue_.front()),
                        [sp = shared_from_this()](error_code ec, std::size_t bytes) { sp->write(ec, bytes); });
}