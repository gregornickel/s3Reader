#ifndef S3READER_S3STATS_H_
#define S3READER_S3STATS_H_

#include <iostream>
#include <iomanip> // std::setw
#include <fstream> // std::ofstream
#include <chrono>
#include "lib/nlohmann/json.hpp"
#include "GameHandler.hpp"

// for convenience
using json = nlohmann::json;

class S3Stats
{
public:
    S3Stats(GameHandler& s3);
    bool record();
    void save();

private:
    void getTakenSpots();
    int findRandomRace(int i);
    void readRace(int i);
    void readStats(int i);
    void readSpellCost(int i);

    GameHandler s3;
    std::string filename;
    std::chrono::high_resolution_clock::time_point t1;
    json j;
    json jCast;
    std::vector<int> takenSpots;
};

#endif // S3READER_S3STATS_H_