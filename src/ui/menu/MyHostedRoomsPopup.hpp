#pragma once
#include <Geode/Geode.hpp>
#include "../core/BasePopup.hpp"
#include <Geode/utils/web.hpp>

namespace mpedit {

    class MyHostedRoomsPopup : public BasePopup {
    protected:
        geode::ScrollLayer* m_scrollLayer = nullptr;
        std::string m_url;
        std::string m_token;
        geode::async::TaskHolder<geode::utils::web::WebResponse> m_fetchTask;

        bool init();
        void fetchRooms();
        void setupList(matjson::Value const& rooms);
        void onRefresh(cocos2d::CCObject*);

    public:
        static MyHostedRoomsPopup* create();
    };

    class ManageRoomPopup : public BasePopup {
    protected:
        std::string m_code;
        std::string m_url;
        std::string m_token;
        geode::async::TaskHolder<geode::utils::web::WebResponse> m_manageTask;

        bool init(matjson::Value const& roomObj, std::string const& url, std::string const& token);
        void onCopyInvite(cocos2d::CCObject*);
        void onChangePassword(cocos2d::CCObject*);
        void onChangeMaxPlayers(cocos2d::CCObject*);
        void onDownloadBackups(cocos2d::CCObject*);
        void onShutDown(cocos2d::CCObject*);

    public:
        friend class PasswordPopup;
        friend class MaxPlayersPopup;
        static ManageRoomPopup* create(matjson::Value const& roomObj, std::string const& url, std::string const& token);
    };

}
