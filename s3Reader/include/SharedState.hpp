#ifndef S3READER_SHAREDSTATE_HPP_
#define S3READER_SHAREDSTATE_HPP_

#include <memory>
#include <string>
#include <unordered_set>

// Forward declaration
class WebSocketSession;

// Accepts incoming connections and launches the session
class SharedState
{
public:
    SharedState();

    void join(WebSocketSession& session);
    void leave(WebSocketSession& session);
    void send(std::string message);

private:
    // This simple method of tracking sessions only works with an implicit strand (i.e. a single-threaded server)
    std::unordered_set<WebSocketSession*> sessions_;
};

#endif // S3READER_SHAREDSTATE_HPP_