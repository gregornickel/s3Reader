#ifndef S3READER_WEBSOCKETSESSION_HPP_
#define S3READER_WEBSOCKETSESSION_HPP_

#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#include "boost.hpp"
#include "SharedState.hpp"

// Forward declaration
class SharedState;

class WebSocketSession : public std::enable_shared_from_this<WebSocketSession>
{
public:
    WebSocketSession(tcp::socket socket, std::shared_ptr<SharedState> const& state);
    ~WebSocketSession();

    void run();
    void send(std::shared_ptr<std::string const> const& ss);

private:
    void fail(beast::error_code ec, char const* what);
    void accept(beast::error_code ec);
    void read(beast::error_code ec, std::size_t bytes_transferred);
    void write(beast::error_code ec, std::size_t bytes_transferred);

    beast::flat_buffer buffer_;
    websocket::stream<beast::tcp_stream> ws_;
    std::shared_ptr<SharedState> state_;
    std::vector<std::shared_ptr<std::string const>> queue_;
};

#endif // S3READER_WEBSOCKETSESSION_HPP_