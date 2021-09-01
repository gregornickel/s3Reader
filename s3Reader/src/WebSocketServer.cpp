#include "WebSocketServer.hpp"
#include "Listener.hpp"
#include "SharedState.hpp"
#include <boost/asio/signal_set.hpp>

WebSocketServer::WebSocketServer() { connections_ = std::make_shared<SharedState>(); }

void WebSocketServer::run()
{
    auto const address = boost::asio::ip::make_address("127.0.0.1");
    auto const port = static_cast<unsigned short>(std::atoi("8082"));

    // The io_context is required for all I/O
    boost::asio::io_context ioc;

    // Create and launch a listening port
    std::make_shared<Listener>(ioc, tcp::endpoint{ address, port }, connections_)->run();

    // Capture SIGINT and SIGTERM to perform a clean shutdown
    asio::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait([&ioc](boost::system::error_code const&, int) {
        // Stop the io_context. This will cause run() to return immediately, eventually destroying the io_context
        // and any remaining handlers in it.
        std::cout << "SIGINT, SIGTERM\n";
        ioc.stop();
    });

    // Run the I/O service on the main thread
    ioc.run();
}

void WebSocketServer::send(std::string message) { connections_->send(message); }