#include "JoinMode.hpp"
#include "../CollabLayer.hpp"
#include "../../core/BasePopup.hpp"
#include "../../utils/NineSlice.hpp"
#include <Geode/ui/TextInput.hpp>

using namespace geode::prelude;
using namespace cocos2d;

namespace {
	class PasswordPopup : public mpedit::BasePopup {
	protected:
		geode::TextInput* m_input = nullptr;
		P2PManager::RoomInfo m_room;
		RoomList::JoinCallback m_onJoin;

		bool init(P2PManager::RoomInfo const& room, RoomList::JoinCallback onJoin) {
			if (!mpedit::BasePopup::init(260.f, 160.f)) return false;
			m_room = room;
			m_onJoin = std::move(onJoin);
			setTitle("Enter Password");

			m_input = geode::TextInput::create(200.f, "Password", "chatFont.fnt");
			m_input->setCommonFilter(geode::CommonFilter::Any);
			m_mainLayer->addChildAtPosition(m_input, Anchor::Center);

			auto sprite = ButtonSprite::create("Join", "goldFont.fnt", "GJ_button_01.png", 0.6f);
			auto button = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(PasswordPopup::onJoin));
			button->setPosition(this->fromBottom(25.f));
			m_uiMenu->addChild(button);
			return true;
		}

		void onJoin(CCObject*) {
			if (m_onJoin) m_onJoin(m_room, m_input->getString());
			onClose(nullptr);
		}

	public:
		static PasswordPopup* create(P2PManager::RoomInfo const& room, RoomList::JoinCallback onJoin) {
			auto ret = new PasswordPopup();
			if (ret->init(room, std::move(onJoin))) {
				ret->autorelease();
				return ret;
			}
			delete ret;
			return nullptr;
		}
	};

	class RoomCell : public CCNode {
		P2PManager::RoomInfo m_room;
		RoomList::JoinCallback m_onJoin;

		bool init(P2PManager::RoomInfo const& room, RoomList::JoinCallback onJoin, float width) {
			if (!CCNode::init()) return false;
			m_room = room;
			m_onJoin = std::move(onJoin);
			setContentSize({width, 45.f});

			auto background = NineSliceBox::create(width, getContentSize().height);
			background->setPosition({0.f, 0.f});
			background->setOpacity(255);
			addChild(background);

			auto title = CCLabelBMFont::create(fmt::format("{} ({}/{})", room.roomName, room.playerCount, room.playerLimit).c_str(), "bigFont.fnt");
			title->setAnchorPoint({0, 0.5f});
			title->setPosition({12.f, 30.f});
			title->setScale(0.45f);
			addChild(title);

			if (room.hasPassword) {
				auto lock = CCSprite::createWithSpriteFrameName("GJ_lock_001.png");
				lock->setScale(0.5f);
				lock->setPosition({width - 72.f, 22.5f});
				addChild(lock);
			}

			auto host = CCLabelBMFont::create(fmt::format("Host: {} ({})", room.hostName, room.version).c_str(), "goldFont.fnt");
			host->setAnchorPoint({0, 0.5f});
			host->setPosition({12.f, 13.f});
			host->setScale(0.45f);
			addChild(host);

			auto joinSprite = ButtonSprite::create("Join", "goldFont.fnt", "GJ_button_01.png", 0.8f);
			joinSprite->setScale(0.55f);
			auto join = CCMenuItemSpriteExtra::create(joinSprite, this, menu_selector(RoomCell::onJoin));
			auto menu = CCMenu::create();
			menu->setPosition({width - 45.f, 22.5f});
			menu->addChild(join);
			addChild(menu);
			return true;
		}

		void onJoin(CCObject*) {
			if (m_room.hasPassword) {
				if (auto popup = PasswordPopup::create(m_room, m_onJoin)) popup->show();
			} else if (m_onJoin) {
				m_onJoin(m_room, "");
			}
		}

	public:
		static RoomCell* create(P2PManager::RoomInfo const& room, RoomList::JoinCallback onJoin, float width) {
			auto ret = new RoomCell();
			if (ret->init(room, std::move(onJoin), width)) {
				ret->autorelease();
				return ret;
			}
			delete ret;
			return nullptr;
		}
	};
}

bool RoomList::init(JoinCallback onJoin) {
	if (!CCNode::init()) return false;
	auto winSize = CCDirector::sharedDirector()->getWinSize();
	setContentSize({winSize.width * 0.8f, winSize.height * 0.55f});
	setID("public-room-list"_spr);
	m_onJoin = std::move(onJoin);

	auto status = CCLabelBMFont::create("Fetching rooms...", "chatFont.fnt");
	status->setID("room-status"_spr);
	status->setPosition(getContentSize() / 2.f);
	status->setScale(0.6f);
	addChild(status);

	auto bg = NineSliceBox::create(getContentSize().width, getContentSize().height);
	bg->setPosition({0.f, 0.f});
	addChild(bg, -1);

	m_scroll = ScrollLayer::create(getContentSize());
	m_scroll->m_contentLayer->setLayout(ColumnLayout::create()->setGap(0.f)->setAxisReverse(true)->setAxisAlignment(AxisAlignment::End));
	addChild(m_scroll);

	auto borders = ListBorders::create();
	borders->setContentSize(getContentSize());
	borders->setPosition(getContentSize() / 2.f);
	addChild(borders);
	fetchRooms();
	return true;
}

RoomList* RoomList::create(JoinCallback onJoin) {
	auto ret = new RoomList();
	if (ret->init(std::move(onJoin))) {
		ret->autorelease();
		return ret;
	}
	delete ret;
	return nullptr;
}

void RoomList::fetchRooms() {
	Ref<RoomList> self = this;
	P2PManager::get().fetchRooms([self](std::vector<P2PManager::RoomInfo> const& rooms) {
		if (self->getParent()) self->populateRooms(rooms);
	});
}

void RoomList::populateRooms(std::vector<P2PManager::RoomInfo> const& rooms) {
	auto status = typeinfo_cast<CCLabelBMFont*>(getChildByID("room-status"_spr));
	if (status) {
		status->setVisible(rooms.empty());
		if (rooms.empty()) status->setString("No rooms found");
	}
	m_scroll->m_contentLayer->removeAllChildren();
	m_scroll->m_contentLayer->setContentHeight(std::max(getContentSize().height, rooms.size() * 45.f));
	for (auto const& room : rooms) {
		m_scroll->m_contentLayer->addChild(RoomCell::create(room, m_onJoin, getContentSize().width));
	}
	m_scroll->m_contentLayer->updateLayout();
	m_scroll->scrollToTop();
}

void RoomList::onRefresh(CCObject*) {
	auto status = typeinfo_cast<CCLabelBMFont*>(getChildByID("room-status"_spr));
	if (status) {
		status->setVisible(true);
		status->setString("Fetching rooms...");
	}
	m_scroll->m_contentLayer->removeAllChildren();
	fetchRooms();
}

void RoomList::refresh() {
	onRefresh(nullptr);
}
