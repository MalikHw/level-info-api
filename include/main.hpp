#pragma once
#include <Geode/Geode.hpp>

namespace LevelFetchAPI {
    struct LevelData {
        int levelID;
        std::string levelName;
        std::string creator;
        std::string description;
        int length; // 0=tiny, 1=short, 2=medium, 3=long, 4=xl, 5=plat
        int likes; // negative = dislikes
        int downloads;
        std::string songName;
        bool featured;
        bool epic;
    };

    using LevelDataCallback = std::function<void(std::optional<LevelData>)>;

    void fetchLevel(int levelID, LevelDataCallback callback);
}
