#ifndef S3READER_LISTENER_HPP_
#define S3READER_LISTENER_HPP_

#include <memory>
#include <string>
#include "boost.hpp"

// Forward declaration
class SharedState;

// Accepts incoming connections and launches the session
class Listener : public std::enable_shared_from_this<Listener>
{
public:
    Listener(asio::io_context& ioc, tcp::endpoint endpoint, std::shared_ptr<SharedState>& state);

    // Accept incoming connections
    void run();

private:
    void accept(error_code ec);
    void fail(error_code ec, char const* what);

    tcp::acceptor acceptor_;
    tcp::socket socket_;
    std::shared_ptr<SharedState> state_;
};

#endif // S3READER_LISTENER_HPP_