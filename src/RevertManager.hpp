#pragma once

#include "ActionSerializer.hpp"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <optional>
#include <memory>

namespace mpedit {

    struct RevertPreview {
        int placedCount = 0;
        int deletedCount = 0;
        int modifiedCount = 0;
        int colorCount = 0;
        bool settingsChanged = false;

        bool isEmpty() const {
            return placedCount == 0 && deletedCount == 0 && modifiedCount == 0 && colorCount == 0 && !settingsChanged;
        }
    };

    class RevertManager {
    public:
        static RevertManager& get();

        void captureBaseline();

        void onObjectsPlaced(int playerId, std::vector<ActionSerializer::ObjectData> const& objects);
        void onObjectsDeleted(int playerId, std::vector<std::string> const& uuids);
        void onObjectsMoved(int playerId, std::vector<ActionSerializer::MoveData> const& moves);
        void onObjectsTransformed(int playerId, std::vector<ActionSerializer::TransformData> const& transforms);
        void onObjectsReconciled(int playerId, std::vector<ActionSerializer::ReconcileData> const& reconciles);
        void onObjectsUpdated(int playerId, std::vector<ActionSerializer::ObjectData> const& objects);
        void onColorChannelUpdated(int playerId, ActionSerializer::ColorChannelData const& data);
        void onSettingsUpdated(int playerId, ActionSerializer::LevelSettingsData const& settings);

        RevertPreview getRevertPreview(std::string const& playerName, std::optional<std::chrono::seconds> timeWindow = std::nullopt);
        RevertPreview getRollbackPreview(std::chrono::seconds timeWindow);
        RevertPreview getRollbackPreviewAtTime(std::chrono::steady_clock::time_point targetTime);
        RevertPreview getRollbackPreviewAtTimeInBranch(size_t branchIndex, std::chrono::steady_clock::time_point targetTime);

        bool revertPlayer(std::string const& playerName, std::optional<std::chrono::seconds> timeWindow = std::nullopt);
        bool rollbackLevel(std::chrono::seconds timeWindow);
        bool rollbackToTime(std::chrono::steady_clock::time_point targetTime);
        bool rollbackToTimeInBranch(size_t branchIndex, std::chrono::steady_clock::time_point targetTime);
        bool switchActiveBranch(size_t branchIndex, std::optional<std::chrono::steady_clock::time_point> targetTime = std::nullopt);

        struct TimelineBranchInfo {
            int id = 1;
            std::string name;
            std::chrono::steady_clock::time_point createdAt;
            std::chrono::steady_clock::time_point forkTime;
            int forkedFromBranchId = 0;
            size_t parentIndex = 0;
            size_t actionCount = 0;
            size_t cursor = 0;
            bool isForked = false;
        };

        size_t getBranchCount() const;
        TimelineBranchInfo getBranchInfo(size_t index) const;
        size_t getBranchIndexById(int id) const;
        size_t getActiveBranchIndex() const;
        bool deleteBranch(size_t index);

        std::pair<std::chrono::steady_clock::time_point, std::chrono::steady_clock::time_point> getTimelineTimeRange() const;
        std::pair<std::chrono::steady_clock::time_point, std::chrono::steady_clock::time_point> getBranchTimeRange(size_t branchIndex) const;
        std::chrono::steady_clock::time_point getCurrentTimelineTime() const;
        std::chrono::steady_clock::time_point getBranchCurrentTime(size_t branchIndex) const;
        std::pair<std::chrono::steady_clock::time_point, std::chrono::steady_clock::time_point> getPlayerTimeRange(std::string const& playerName) const;

        bool isRolledBack() const;
        void clear();

    private:
        RevertManager();
        ~RevertManager() = default;
        RevertManager(RevertManager const&) = delete;
        RevertManager& operator=(RevertManager const&) = delete;

        std::string resolvePlayerName(int playerId);

        struct DeletedEntry {
            ActionSerializer::ObjectData data;
            std::string deletedBy;
            std::chrono::steady_clock::time_point timestamp;
        };

        struct ActionRecord {
            enum class Type { Place, Delete, Modify, Color, Settings };
            Type type;
            std::string playerName;
            int playerId;
            std::chrono::steady_clock::time_point timestamp;
            std::string uuid;
            ActionSerializer::ObjectData preState;
            ActionSerializer::ObjectData postState;
            ActionSerializer::ColorChannelData preColor;
            ActionSerializer::ColorChannelData postColor;
            ActionSerializer::LevelSettingsData preSettings;
            ActionSerializer::LevelSettingsData postSettings;
            int colorChannelId = 0;
        };

        struct TimelineBranch {
            int id = 1;
            std::string name;
            std::chrono::steady_clock::time_point createdAt;
            std::chrono::steady_clock::time_point forkTime;
            int forkedFromBranchId = 0;
            std::vector<ActionRecord> actionLog;
            size_t cursor = 0;
            bool isForked = false;
        };

        void ensureBranch();
        void prepareNewAction();

        bool m_isApplyingRollback = false;

        std::unordered_map<std::string, std::string> m_lastTouchedBy;
        std::unordered_map<std::string, std::string> m_createdObjects;
        std::unordered_map<std::string, ActionSerializer::ObjectData> m_baselineStates;
        std::unordered_map<std::string, DeletedEntry> m_deletedObjects;
        std::unordered_map<std::string, std::chrono::steady_clock::time_point> m_touchTimestamps;

        std::unordered_map<int, ActionSerializer::ColorChannelData> m_baselineColors;
        std::unordered_map<int, ActionSerializer::ColorChannelData> m_currentColors;
        std::unordered_map<int, std::string> m_lastTouchedColor;
        std::unordered_map<int, std::chrono::steady_clock::time_point> m_colorTimestamps;

        std::optional<ActionSerializer::LevelSettingsData> m_baselineSettings;
        std::optional<ActionSerializer::LevelSettingsData> m_currentSettings;
        std::string m_lastTouchedSettings;
        std::chrono::steady_clock::time_point m_settingsTimestamp;

        std::vector<TimelineBranch> m_branches;
        size_t m_activeBranchIndex = 0;
    };

}
