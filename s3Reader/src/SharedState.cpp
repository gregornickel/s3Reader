#include "SharedState.hpp"
#include "WebSocketSession.hpp"

SharedState::SharedState() {}

void SharedState::join(WebSocketSession& session) { sessions_.insert(&session); }

void SharedState::leave(WebSocketSession& session) { sessions_.erase(&session); }

void SharedState::send(std::string message)
{
    auto const ss = std::make_shared<std::string const>(std::move(message));

    for (auto session : sessions_)
        session->send(ss);
}