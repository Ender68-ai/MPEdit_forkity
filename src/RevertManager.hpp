#pragma once

#include "ActionSerializer.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <optional>
#include <memory>

namespace mpedit {

    struct RevertPreview {
        int placedCount = 0;
        int deletedCount = 0;
        int modifiedCount = 0;
    };

    class RevertManager {
    public:
        static RevertManager& get();

        void onObjectsPlaced(int playerId, std::vector<ActionSerializer::ObjectData> const& objects);
        void onObjectsDeleted(int playerId, std::vector<std::string> const& uuids);
        void onObjectsMoved(int playerId, std::vector<ActionSerializer::MoveData> const& moves);
        void onObjectsTransformed(int playerId, std::vector<ActionSerializer::TransformData> const& transforms);
        void onObjectsReconciled(int playerId, std::vector<ActionSerializer::ReconcileData> const& reconciles);
        void onObjectsUpdated(int playerId, std::vector<ActionSerializer::ObjectData> const& objects);
        void onColorChannelUpdated(int playerId, ActionSerializer::ColorChannelData const& data);
        void onSettingsUpdated(int playerId, ActionSerializer::LevelSettingsData const& settings);

        RevertPreview getRevertPreview(int playerId, std::optional<std::chrono::seconds> timeWindow = std::nullopt);
        RevertPreview getRollbackPreview(std::chrono::seconds timeWindow);

        bool revertPlayer(int playerId, std::optional<std::chrono::seconds> timeWindow = std::nullopt);
        bool rollbackLevel(std::chrono::seconds timeWindow);

        void clear();

    private:
        RevertManager() = default;
        ~RevertManager() = default;
        RevertManager(RevertManager const&) = delete;
        RevertManager& operator=(RevertManager const&) = delete;

        struct DeletedEntry {
            ActionSerializer::ObjectData data;
            int deletedBy;
            std::chrono::steady_clock::time_point timestamp;
        };

        struct ActionRecord {
            enum class Type { Place, Delete, Modify, Color, Settings };
            Type type;
            int playerId;
            std::chrono::steady_clock::time_point timestamp;
            std::string uuid;
            ActionSerializer::ObjectData preState;
            ActionSerializer::ColorChannelData preColor;
            ActionSerializer::LevelSettingsData preSettings;
            int colorChannelId = 0;
        };

        std::unordered_map<std::string, int> m_lastTouchedBy;
        std::unordered_map<std::string, int> m_createdObjects;
        std::unordered_map<std::string, ActionSerializer::ObjectData> m_baselineStates;
        std::unordered_map<std::string, DeletedEntry> m_deletedObjects;
        std::unordered_map<std::string, std::chrono::steady_clock::time_point> m_touchTimestamps;

        std::unordered_map<int, ActionSerializer::ColorChannelData> m_baselineColors;
        std::unordered_map<int, int> m_lastTouchedColor;
        std::unordered_map<int, std::chrono::steady_clock::time_point> m_colorTimestamps;

        std::optional<ActionSerializer::LevelSettingsData> m_baselineSettings;
        int m_lastTouchedSettings = -1;
        std::chrono::steady_clock::time_point m_settingsTimestamp;

        std::vector<ActionRecord> m_actionLog;
    };

}
