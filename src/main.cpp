#include <Geode/Geode.hpp>
#include <Geode/modify/UILayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

bool g_isModActive = true;

class $modify(MyUILayer, UILayer) {
    void keyDown(cocos2d::enumKeyCodes key) {
        UILayer::keyDown(key);
        // Проверка нажатия клавиши [
        if (key == cocos2d::enumKeyCodes::KEY_LeftBracket) {
            g_isModActive = !g_isModActive;
            if (g_isModActive) {
                FLAlertLayer::create("Smooth Spam", "Mod is ON", "OK")->show();
            } else {
                FLAlertLayer::create("Smooth Spam", "Mod is OFF", "OK")->show();
            }
        }
    }
};

class $modify(MyPlayer, PlayerObject) {
    // Просто чистый перехват прыжка, без сложной физики
    void pushButton(PlayerButton btn) {
        PlayerObject::pushButton(btn);
    }
};
