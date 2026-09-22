#include "EditorHookState.hpp"

namespace mpedit {
    void updateStartPosCache(GameObject* obj) {
        if (obj && obj->m_objectID == 31 && hookstate::s_startPosObjects.count(obj)) {
            if (auto* editor = LevelEditorLayer::get()) {
                hookstate::s_startPosSaveStrings[obj] = obj->getSaveString(editor);
            }
        }
    }
}