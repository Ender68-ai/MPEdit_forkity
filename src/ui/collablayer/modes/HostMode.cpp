#include "HostMode.hpp"
#include "hostmode/RoomCreateLayer.hpp"
#include "SessionManager.hpp"

#include <Geode/Geode.hpp>
#include <Geode/binding/LocalLevelManager.hpp>
#include <Geode/binding/CustomListView.hpp>
#include <Geode/binding/GJListLayer.hpp>

using namespace geode::prelude;


HostMode* HostMode::create() {
    auto ret = new HostMode();

    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

HostMode::~HostMode() {
    mpedit::SessionManager::get().removeListener(this);
}


void HostMode::showListAfterDelay(float) {
        m_listLayer->setVisible(true);
}

bool HostMode::init() {
    if (!CCNode::init())
        return false;

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    auto roomSetup = RoomCreateLayer::create([this]() {
        if (m_listLayer) {
            this->scheduleOnce(schedule_selector(HostMode::showListAfterDelay), 0.5f);
        }
    });

    if (!roomSetup) {
        return false;
    }

    this->addChild(roomSetup);

    // Register callback for when session ends
    mpedit::SessionManager::get().onSessionEnded(this, [this]() {
        if (m_listLayer) {
            m_listLayer->setVisible(false);
        }
    });

    auto delegate = HostLocalLevelList::create();

    auto listView = CustomListView::create(
        LocalLevelManager::sharedState()->m_localLevels,
        delegate,
        200.f,
        200.f,
        0,
        BoomListType::Level,
        0.f
    );

    if (!listView)
        return false;

    listView->setID("local-levels-list"_spr);

    auto levelListLayer = GJListLayer::create(
        listView,
        "Local",
        {255, 255, 255, 255},
        200.f,
        200.f,
        0
    );

    if (!levelListLayer)
        return false;

    auto top = levelListLayer->getChildByID("top-border");
    auto bottom = levelListLayer->getChildByID("bottom-border");

    if (top) {
        top->setScaleX(0.6f);
    }

    if (bottom) {
        bottom->setScaleX(0.6f);
    }

    levelListLayer->setPosition({
        winSize.width * 0.55f,
        winSize.height * 0.12f
    });

    levelListLayer->setVisible(false);
    levelListLayer->setScale(0.9f);

    m_listLayer = levelListLayer;
    this->addChild(m_listLayer);

    return true;
}

// HostLocalLevelList

HostLocalLevelList* HostLocalLevelList::create() {
    auto ret = new HostLocalLevelList();

    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }

    

    delete ret;
    return nullptr;
}

bool HostLocalLevelList::cellPerformedAction(
    TableViewCell* cell,
    int listType,
    CellAction action,
    cocos2d::CCNode* parent
) {
    if (action == CellAction::Click) {
        // TODO: handle level selection
        return true;
    }

    return false;
}

int HostLocalLevelList::getSelectedCellIdx() {
    return -1;
}

bool HostLocalLevelList::shouldSnapToSelected() {
    return false;
}

int HostLocalLevelList::getCellDelegateType() {
    return 0;
}

bool HostLocalLevelList::init() {
    if (!CCNode::init())
        return false; 

    return true;
}
