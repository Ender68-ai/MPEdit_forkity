#include "cocos2d.h"

using namespace cocos2d;
using namespace geode::prelude;

class Panel : public CCNode {
public:
    static Panel* create(
        const char* title,
        CCSize size
    );

    bool init(
        const char* title,
        CCSize size
    );

    CCNode* getContent() {
        return m_content;
    }

private:
    CCScale9Sprite* m_background;
    CCLabelBMFont* m_title;
    CCNode* m_content;
};