#include "main.hpp"
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

std::optional<LevelData> parseLevelResponse(const std::string& response) {
    LevelData data;

    std::vector<std::string> parts;
    std::stringstream ss(response);
    std::string item;
    
    while (std::getline(ss, item, ':')) {
        parts.push_back(item);
    }
    
    // 1 = level ID, 2 = level name, 3 = description (base64), 5 = version, 6 = player ID
    // 8 = difficulty, 9 = downloads, 10 = official song ID, 12 = official song
    // 13 = game version, 14 = likes, 15 = length, 17 = demon, 18 = stars
    // 25 = auto, 35 = custom song ID, 42 = epic, 43 = demon difficulty
    
    try {
        for (size_t i = 0; i < parts.size() - 1; i += 2) {
            std::string key = parts[i];
            std::string value = parts[i + 1];
            
            if (key == "1") {
                data.levelID = std::stoi(value);
            }
            else if (key == "2") {
                data.levelName = value;
            }
            else if (key == "3") {
                // Description is base64 encoded
                auto decoded = ZipUtils::base64URLDecode(value);
                data.description = std::string(reinterpret_cast<const char*>(decoded.data()), decoded.size());
            }
            else if (key == "6") {
                data.creator = "User ID: " + value;
            }
            else if (key == "15") {
                data.length = std::stoi(value);
            }
            else if (key == "14") {
                data.likes = std::stoi(value);
            }
            else if (key == "10") {
                data.downloads = std::stoi(value);
            }
            else if (key == "12") {
                data.songName = "Official Song ID: " + value;
            }
            else if (key == "35") {
                data.songName = "Custom Song ID: " + value;
            }
            else if (key == "19") {
                data.featured = (std::stoi(value) > 0);
            }
            else if (key == "42") {
                data.epic = (std::stoi(value) > 0);
            }
        }
        
        return data;
    }
    catch (...) {
        return std::nullopt;
    }
}

void LevelFetchAPI::fetchLevel(int levelID, LevelDataCallback callback) {
    // Boomlings server endpoint for getting level info
    std::string url = "http://www.boomlings.com/database/getGJLevels21.php";
    
    // Build POST request body
    std::string postData = fmt::format(
        "gameVersion=22&binaryVersion=42&gdw=0&str={}&type=0&secret=Wmfd2893gb7",
        levelID
    );
    
    auto req = web::WebRequest();
    req.bodyString(postData);
    req.header("Content-Type", "application/x-www-form-urlencoded");
    req.userAgent("");
    
    // Async request
    req.post(url).listen([callback](web::WebResponse* response) {
        if (!response->ok()) {
            log::error("Failed to fetch level: HTTP {}", response->code());
            callback(std::nullopt);
            return;
        }
        
        auto responseStr = response->string().unwrapOr("");
        
        if (responseStr.empty() || responseStr == "-1") {
            log::error("Level not found or server error");
            callback(std::nullopt);
            return;
        }
        
        // i only care about the first part
        auto hashPos = responseStr.find('#');
        std::string levelData = (hashPos != std::string::npos) 
            ? responseStr.substr(0, hashPos) 
            : responseStr;
        
        auto parsed = parseLevelResponse(levelData);
        
        if (parsed.has_value()) {
            log::info("Successfully fetched level: {}", parsed->levelName);
        } else {
            log::error("Failed to parse level data");
        }
        
        callback(parsed);
    }, [callback](web::WebProgress* progress) {
        // Progress callback, listen here motherfucker we don't need this but it's required
    });
}
