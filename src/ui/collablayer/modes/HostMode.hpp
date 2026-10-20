#pragma once

#include <Geode/Geode.hpp>
#include <Geode/binding/TableViewCellDelegate.hpp>

using namespace geode::prelude;

// Forward declaration
class HostLocalLevelList;

class HostMode : public CCNode {
    friend class HostLocalLevelList;

    protected:
        bool init();
        void showListAfterDelay(float);

    public:
        ~HostMode();
        static HostMode* create();
        static HostMode* get();
        
        void onLevelSelected(GJGameLevel* level);
        void showLoadingOverlay(const char* message);
        void hideLoadingOverlay();

        GJListLayer* m_listLayer = nullptr;
        CCNode* m_loadingOverlay = nullptr;
        CCLabelBMFont* m_loadingLabel = nullptr;
};




class HostLocalLevelList : public CCNode, public TableViewCellDelegate {
protected:
    bool init();

public:
    static HostLocalLevelList* create();

    bool cellPerformedAction(
        TableViewCell* cell,
        int listType,
        CellAction action,
        cocos2d::CCNode* parent
    );

    int getSelectedCellIdx();
    bool shouldSnapToSelected();
    int getCellDelegateType();

    

};
