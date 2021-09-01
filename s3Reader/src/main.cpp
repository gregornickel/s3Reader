#include <thread>
#include "WebSocketServer.hpp"
#include "GameHandler.hpp"
#include "S3Stats.hpp"

int main(int argc, char* argv[])
{
    WebSocketServer* wss = new WebSocketServer();
    std::thread wssThread(&WebSocketServer::run, wss);

    std::string gameName = "s3_alobby.exe";
    std::string windowName = "S3";
    GameHandler s3 = GameHandler(gameName, windowName);
    s3.find();

    S3Stats stats = S3Stats(s3);

    bool gameEnded = false;
    while (!gameEnded) {
        gameEnded = stats.record();
        wss->send(stats.overlayData());
        stats.save();

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    wssThread.join();
    delete wss;

    return 0;
}
