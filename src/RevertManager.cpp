#include "RevertManager.hpp"
#include "RemoteActionHandler.hpp"
#include "P2PManager.hpp"
#include "BinaryProtocol.hpp"
#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/binding/EditorUI.hpp>
#include <Geode/binding/LevelSettingsObject.hpp>
#include <Geode/binding/ColorAction.hpp>
#include <algorithm>

namespace mpedit {

    RevertManager& RevertManager::get() {
        static RevertManager instance;
        return instance;
    }

    void RevertManager::clear() {
        m_lastTouchedBy.clear();
        m_createdObjects.clear();
        m_baselineStates.clear();
        m_deletedObjects.clear();
        m_touchTimestamps.clear();
        m_baselineColors.clear();
        m_lastTouchedColor.clear();
        m_colorTimestamps.clear();
        m_baselineSettings.reset();
        m_lastTouchedSettings = -1;
        m_actionLog.clear();
    }

    void RevertManager::onObjectsPlaced(int playerId, std::vector<ActionSerializer::ObjectData> const& objects) {
        auto now = std::chrono::steady_clock::now();
        for (auto const& obj : objects) {
            if (obj.uuid.empty()) continue;
            m_lastTouchedBy[obj.uuid] = playerId;
            m_createdObjects[obj.uuid] = playerId;
            m_touchTimestamps[obj.uuid] = now;
            m_deletedObjects.erase(obj.uuid);

            ActionRecord rec;
            rec.type = ActionRecord::Type::Place;
            rec.playerId = playerId;
            rec.timestamp = now;
            rec.uuid = obj.uuid;
            m_actionLog.push_back(rec);
        }
    }

    void RevertManager::onObjectsDeleted(int playerId, std::vector<std::string> const& uuids) {
        auto now = std::chrono::steady_clock::now();
        auto& handler = RemoteActionHandler::get();

        for (auto const& uuid : uuids) {
            if (uuid.empty()) continue;
            auto* obj = handler.getObjectByUUID(uuid);
            if (!obj) continue;

            ActionSerializer::ObjectData data = ActionSerializer::extractObjectData(obj, uuid);

            DeletedEntry entry;
            entry.data = data;
            entry.deletedBy = playerId;
            entry.timestamp = now;
            m_deletedObjects[uuid] = entry;

            m_lastTouchedBy[uuid] = playerId;
            m_touchTimestamps[uuid] = now;

            ActionRecord rec;
            rec.type = ActionRecord::Type::Delete;
            rec.playerId = playerId;
            rec.timestamp = now;
            rec.uuid = uuid;
            rec.preState = data;
            m_actionLog.push_back(rec);
        }
    }

    void RevertManager::onObjectsMoved(int playerId, std::vector<ActionSerializer::MoveData> const& moves) {
        auto now = std::chrono::steady_clock::now();
        auto& handler = RemoteActionHandler::get();

        for (auto const& move : moves) {
            if (move.uuid.empty()) continue;
            auto* obj = handler.getObjectByUUID(move.uuid);
            if (!obj) continue;

            auto it = m_lastTouchedBy.find(move.uuid);
            if (it == m_lastTouchedBy.end() || it->second != playerId) {
                if (m_baselineStates.find(move.uuid) == m_baselineStates.end()) {
                    m_baselineStates[move.uuid] = ActionSerializer::extractObjectData(obj, move.uuid);
                }
            }

            m_lastTouchedBy[move.uuid] = playerId;
            m_touchTimestamps[move.uuid] = now;

            ActionRecord rec;
            rec.type = ActionRecord::Type::Modify;
            rec.playerId = playerId;
            rec.timestamp = now;
            rec.uuid = move.uuid;
            rec.preState = ActionSerializer::extractObjectData(obj, move.uuid);
            m_actionLog.push_back(rec);
        }
    }

    void RevertManager::onObjectsTransformed(int playerId, std::vector<ActionSerializer::TransformData> const& transforms) {
        auto now = std::chrono::steady_clock::now();
        auto& handler = RemoteActionHandler::get();

        for (auto const& t : transforms) {
            if (t.uuid.empty()) continue;
            auto* obj = handler.getObjectByUUID(t.uuid);
            if (!obj) continue;

            auto it = m_lastTouchedBy.find(t.uuid);
            if (it == m_lastTouchedBy.end() || it->second != playerId) {
                if (m_baselineStates.find(t.uuid) == m_baselineStates.end()) {
                    m_baselineStates[t.uuid] = ActionSerializer::extractObjectData(obj, t.uuid);
                }
            }

            m_lastTouchedBy[t.uuid] = playerId;
            m_touchTimestamps[t.uuid] = now;

            ActionRecord rec;
            rec.type = ActionRecord::Type::Modify;
            rec.playerId = playerId;
            rec.timestamp = now;
            rec.uuid = t.uuid;
            rec.preState = ActionSerializer::extractObjectData(obj, t.uuid);
            m_actionLog.push_back(rec);
        }
    }

    void RevertManager::onObjectsReconciled(int playerId, std::vector<ActionSerializer::ReconcileData> const& reconciles) {
        auto now = std::chrono::steady_clock::now();
        auto& handler = RemoteActionHandler::get();

        for (auto const& r : reconciles) {
            if (r.uuid.empty()) continue;
            auto* obj = handler.getObjectByUUID(r.uuid);
            if (!obj) continue;

            auto it = m_lastTouchedBy.find(r.uuid);
            if (it == m_lastTouchedBy.end() || it->second != playerId) {
                if (m_baselineStates.find(r.uuid) == m_baselineStates.end()) {
                    m_baselineStates[r.uuid] = ActionSerializer::extractObjectData(obj, r.uuid);
                }
            }

            m_lastTouchedBy[r.uuid] = playerId;
            m_touchTimestamps[r.uuid] = now;

            ActionRecord rec;
            rec.type = ActionRecord::Type::Modify;
            rec.playerId = playerId;
            rec.timestamp = now;
            rec.uuid = r.uuid;
            rec.preState = ActionSerializer::extractObjectData(obj, r.uuid);
            m_actionLog.push_back(rec);
        }
    }

    void RevertManager::onObjectsUpdated(int playerId, std::vector<ActionSerializer::ObjectData> const& objects) {
        auto now = std::chrono::steady_clock::now();
        auto& handler = RemoteActionHandler::get();

        for (auto const& data : objects) {
            if (data.uuid.empty()) continue;
            auto* obj = handler.getObjectByUUID(data.uuid);
            if (!obj) continue;

            auto it = m_lastTouchedBy.find(data.uuid);
            if (it == m_lastTouchedBy.end() || it->second != playerId) {
                if (m_baselineStates.find(data.uuid) == m_baselineStates.end()) {
                    m_baselineStates[data.uuid] = ActionSerializer::extractObjectData(obj, data.uuid);
                }
            }

            m_lastTouchedBy[data.uuid] = playerId;
            m_touchTimestamps[data.uuid] = now;

            ActionRecord rec;
            rec.type = ActionRecord::Type::Modify;
            rec.playerId = playerId;
            rec.timestamp = now;
            rec.uuid = data.uuid;
            rec.preState = ActionSerializer::extractObjectData(obj, data.uuid);
            m_actionLog.push_back(rec);
        }
    }

    void RevertManager::onColorChannelUpdated(int playerId, ActionSerializer::ColorChannelData const& data) {
        auto now = std::chrono::steady_clock::now();
        if (m_baselineColors.find(data.channelID) == m_baselineColors.end()) {
            m_baselineColors[data.channelID] = data;
        }
        m_lastTouchedColor[data.channelID] = playerId;
        m_colorTimestamps[data.channelID] = now;

        ActionRecord rec;
        rec.type = ActionRecord::Type::Color;
        rec.playerId = playerId;
        rec.timestamp = now;
        rec.preColor = data;
        rec.colorChannelId = data.channelID;
        m_actionLog.push_back(rec);
    }

    void RevertManager::onSettingsUpdated(int playerId, ActionSerializer::LevelSettingsData const& settings) {
        auto now = std::chrono::steady_clock::now();
        if (!m_baselineSettings.has_value()) {
            m_baselineSettings = settings;
        }
        m_lastTouchedSettings = playerId;
        m_settingsTimestamp = now;

        ActionRecord rec;
        rec.type = ActionRecord::Type::Settings;
        rec.playerId = playerId;
        rec.timestamp = now;
        rec.preSettings = settings;
        m_actionLog.push_back(rec);
    }

    RevertPreview RevertManager::getRevertPreview(int playerId, std::optional<std::chrono::seconds> timeWindow) {
        RevertPreview preview;
        auto now = std::chrono::steady_clock::now();

        auto inWindow = [&](std::chrono::steady_clock::time_point tp) {
            if (!timeWindow.has_value()) return true;
            return (now - tp) <= timeWindow.value();
        };

        for (auto const& [uuid, owner] : m_lastTouchedBy) {
            if (owner != playerId) continue;
            auto tIt = m_touchTimestamps.find(uuid);
            if (tIt != m_touchTimestamps.end() && !inWindow(tIt->second)) continue;

            if (m_deletedObjects.find(uuid) != m_deletedObjects.end()) {
                preview.deletedCount++;
            } else if (m_createdObjects.find(uuid) != m_createdObjects.end() && m_createdObjects.at(uuid) == playerId) {
                preview.placedCount++;
            } else if (m_baselineStates.find(uuid) != m_baselineStates.end()) {
                preview.modifiedCount++;
            }
        }

        return preview;
    }

    RevertPreview RevertManager::getRollbackPreview(std::chrono::seconds timeWindow) {
        RevertPreview preview;
        auto cutoff = std::chrono::steady_clock::now() - timeWindow;

        std::unordered_set<std::string> placed;
        std::unordered_set<std::string> deleted;
        std::unordered_set<std::string> modified;

        for (auto it = m_actionLog.rbegin(); it != m_actionLog.rend(); ++it) {
            if (it->timestamp < cutoff) break;
            if (it->uuid.empty()) continue;

            switch (it->type) {
                case ActionRecord::Type::Place:
                    placed.insert(it->uuid);
                    break;
                case ActionRecord::Type::Delete:
                    deleted.insert(it->uuid);
                    break;
                case ActionRecord::Type::Modify:
                    if (!placed.count(it->uuid) && !deleted.count(it->uuid)) {
                        modified.insert(it->uuid);
                    }
                    break;
                default:
                    break;
            }
        }

        preview.placedCount = static_cast<int>(placed.size());
        preview.deletedCount = static_cast<int>(deleted.size());
        preview.modifiedCount = static_cast<int>(modified.size());

        return preview;
    }

    bool RevertManager::revertPlayer(int playerId, std::optional<std::chrono::seconds> timeWindow) {
        auto* editor = LevelEditorLayer::get();
        if (!editor) return false;

        auto now = std::chrono::steady_clock::now();
        auto inWindow = [&](std::chrono::steady_clock::time_point tp) {
            if (!timeWindow.has_value()) return true;
            return (now - tp) <= timeWindow.value();
        };

        std::vector<std::string> toDelete;
        std::vector<ActionSerializer::ObjectData> toRestore;
        std::vector<ActionSerializer::ObjectData> toUpdate;

        std::vector<std::string> targetUuids;
        for (auto const& [uuid, owner] : m_lastTouchedBy) {
            if (owner == playerId) {
                auto tIt = m_touchTimestamps.find(uuid);
                if (tIt == m_touchTimestamps.end() || inWindow(tIt->second)) {
                    targetUuids.push_back(uuid);
                }
            }
        }

        for (auto const& uuid : targetUuids) {
            auto delIt = m_deletedObjects.find(uuid);
            if (delIt != m_deletedObjects.end()) {
                toRestore.push_back(delIt->second.data);
                m_deletedObjects.erase(delIt);
                m_lastTouchedBy.erase(uuid);
                m_touchTimestamps.erase(uuid);
                continue;
            }

            auto createIt = m_createdObjects.find(uuid);
            if (createIt != m_createdObjects.end() && createIt->second == playerId) {
                toDelete.push_back(uuid);
                m_createdObjects.erase(createIt);
                m_lastTouchedBy.erase(uuid);
                m_touchTimestamps.erase(uuid);
                continue;
            }

            auto baseIt = m_baselineStates.find(uuid);
            if (baseIt != m_baselineStates.end()) {
                toUpdate.push_back(baseIt->second);
                m_baselineStates.erase(baseIt);
                m_lastTouchedBy.erase(uuid);
                m_touchTimestamps.erase(uuid);
            }
        }

        if (!toDelete.empty()) {
            RemoteActionHandler::get().handleRemoteDeleteObjects(playerId, toDelete);
            constexpr size_t CHUNK = 300;
            for (size_t i = 0; i < toDelete.size(); i += CHUNK) {
                size_t count = std::min(CHUNK, toDelete.size() - i);
                std::vector<std::string> chunk(toDelete.begin() + i, toDelete.begin() + i + count);
                P2PManager::get().broadcast(proto::serializeDeleteObjects(chunk), ChannelType::Reliable);
            }
        }

        if (!toRestore.empty()) {
            RemoteActionHandler::get().handleRemotePlaceObjects(playerId, toRestore);
            constexpr size_t CHUNK = 100;
            for (size_t i = 0; i < toRestore.size(); i += CHUNK) {
                size_t count = std::min(CHUNK, toRestore.size() - i);
                std::vector<ActionSerializer::ObjectData> chunk(toRestore.begin() + i, toRestore.begin() + i + count);
                P2PManager::get().broadcast(proto::serializePlaceObjects(chunk), ChannelType::Reliable);
            }
        }

        if (!toUpdate.empty()) {
            RemoteActionHandler::get().handleRemoteUpdateObjects(playerId, toUpdate);
            constexpr size_t CHUNK = 100;
            for (size_t i = 0; i < toUpdate.size(); i += CHUNK) {
                size_t count = std::min(CHUNK, toUpdate.size() - i);
                std::vector<ActionSerializer::ObjectData> chunk(toUpdate.begin() + i, toUpdate.begin() + i + count);
                P2PManager::get().broadcast(proto::serializeUpdateObjects(chunk), ChannelType::Reliable);
            }
        }

        for (auto& [ch, owner] : m_lastTouchedColor) {
            if (owner == playerId) {
                auto tIt = m_colorTimestamps.find(ch);
                if (tIt == m_colorTimestamps.end() || inWindow(tIt->second)) {
                    if (m_baselineColors.find(ch) != m_baselineColors.end()) {
                        RemoteActionHandler::get().handleRemoteUpdateColorChannel(playerId, m_baselineColors[ch]);
                        P2PManager::get().broadcast(proto::serializeUpdateColorChannel(m_baselineColors[ch]), ChannelType::Reliable);
                    }
                }
            }
        }

        if (m_lastTouchedSettings == playerId) {
            if (inWindow(m_settingsTimestamp) && m_baselineSettings.has_value()) {
                RemoteActionHandler::get().handleRemoteUpdateSettings(playerId, m_baselineSettings.value());
                P2PManager::get().broadcast(proto::serializeUpdateSettings(m_baselineSettings.value()), ChannelType::Reliable);
            }
        }

        return true;
    }

    bool RevertManager::rollbackLevel(std::chrono::seconds timeWindow) {
        auto* editor = LevelEditorLayer::get();
        if (!editor) return false;

        auto cutoff = std::chrono::steady_clock::now() - timeWindow;

        std::vector<std::string> toDelete;
        std::vector<ActionSerializer::ObjectData> toRestore;
        std::vector<ActionSerializer::ObjectData> toUpdate;

        std::unordered_set<std::string> processedUuids;

        for (auto it = m_actionLog.rbegin(); it != m_actionLog.rend(); ++it) {
            if (it->timestamp < cutoff) break;
            if (it->uuid.empty() || processedUuids.count(it->uuid)) continue;
            processedUuids.insert(it->uuid);

            switch (it->type) {
                case ActionRecord::Type::Place:
                    toDelete.push_back(it->uuid);
                    break;
                case ActionRecord::Type::Delete:
                    toRestore.push_back(it->preState);
                    break;
                case ActionRecord::Type::Modify:
                    toUpdate.push_back(it->preState);
                    break;
                case ActionRecord::Type::Color:
                    RemoteActionHandler::get().handleRemoteUpdateColorChannel(it->playerId, it->preColor);
                    P2PManager::get().broadcast(proto::serializeUpdateColorChannel(it->preColor), ChannelType::Reliable);
                    break;
                case ActionRecord::Type::Settings:
                    RemoteActionHandler::get().handleRemoteUpdateSettings(it->playerId, it->preSettings);
                    P2PManager::get().broadcast(proto::serializeUpdateSettings(it->preSettings), ChannelType::Reliable);
                    break;
            }
        }

        int localId = P2PManager::get().getLocalPlayerId();

        if (!toDelete.empty()) {
            RemoteActionHandler::get().handleRemoteDeleteObjects(localId, toDelete);
            constexpr size_t CHUNK = 300;
            for (size_t i = 0; i < toDelete.size(); i += CHUNK) {
                size_t count = std::min(CHUNK, toDelete.size() - i);
                std::vector<std::string> chunk(toDelete.begin() + i, toDelete.begin() + i + count);
                P2PManager::get().broadcast(proto::serializeDeleteObjects(chunk), ChannelType::Reliable);
            }
        }

        if (!toRestore.empty()) {
            RemoteActionHandler::get().handleRemotePlaceObjects(localId, toRestore);
            constexpr size_t CHUNK = 100;
            for (size_t i = 0; i < toRestore.size(); i += CHUNK) {
                size_t count = std::min(CHUNK, toRestore.size() - i);
                std::vector<ActionSerializer::ObjectData> chunk(toRestore.begin() + i, toRestore.begin() + i + count);
                P2PManager::get().broadcast(proto::serializePlaceObjects(chunk), ChannelType::Reliable);
            }
        }

        if (!toUpdate.empty()) {
            RemoteActionHandler::get().handleRemoteUpdateObjects(localId, toUpdate);
            constexpr size_t CHUNK = 100;
            for (size_t i = 0; i < toUpdate.size(); i += CHUNK) {
                size_t count = std::min(CHUNK, toUpdate.size() - i);
                std::vector<ActionSerializer::ObjectData> chunk(toUpdate.begin() + i, toUpdate.begin() + i + count);
                P2PManager::get().broadcast(proto::serializeUpdateObjects(chunk), ChannelType::Reliable);
            }
        }

        while (!m_actionLog.empty() && m_actionLog.back().timestamp >= cutoff) {
            m_actionLog.pop_back();
        }

        return true;
    }

}
