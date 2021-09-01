#ifndef S3READER_WEBSOCKETSERVER_HPP_
#define S3READER_WEBSOCKETSERVER_HPP_

#include <iostream>
#include <memory>
#include <boost/asio/ip/tcp.hpp>

// Forward declaration
class SharedState;

using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

class WebSocketServer
{
public:
    WebSocketServer();
    void run();
    void send(std::string);

private:
    std::shared_ptr<SharedState> connections_;
};

#endif // S3READER_WEBSOCKETSERVER_HPP_