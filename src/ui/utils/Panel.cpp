#include "Panel.hpp"

Panel* Panel::create(const char* title, CCSize size) {
    auto ret = new Panel();

    if (ret && ret->init(title, size)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool Panel::init(const char* title, CCSize size) {
    if (!CCNode::init())
        return false;

    auto bg = extension::CCScale9Sprite::create("square02b_001.png");
    bg->setContentSize(size);
    bg->setColor({139, 69, 19});
    addChild(bg);

    auto label = CCLabelBMFont::create(title, "bigFont.fnt");
    label->setPosition({size.width / 4.f, size.height - 50.f});
    addChild(label);

    return true;
}
