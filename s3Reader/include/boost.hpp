#ifndef S3READER_BOOST_HPP
#define S3READER_BOOST_HPP

#include <boost/beast.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/dispatch.hpp>

namespace beast = boost::beast;
namespace http = boost::beast::http; // from <boost/beast/http.hpp>
namespace websocket = boost::beast::websocket; // from <boost/beast/websocket.hpp>
namespace asio = boost::asio;
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>
using error_code = boost::system::error_code; // from <boost/system/error_code.hpp>

#endif // S3READER_BOOST_HPP