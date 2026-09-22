#pragma once

#include <Geode/Geode.hpp>
#include <Geode/binding/TableViewCellDelegate.hpp>

using namespace geode::prelude;

class HostMode : public CCNode {
    protected:
        bool init();
        void showListAfterDelay(float);

        GJListLayer* m_listLayer = nullptr;

    public:
        ~HostMode();
        static HostMode* create();
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

