#include <Geode/Geode.hpp>
#include <Geode/modify/UILayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

bool g_isModActive = true;

class $modify(MyUILayer, UILayer) {
    void keyDown(cocos2d::enumKeyCodes key) {
        UILayer::keyDown(key);
        // Бесшумное переключение мода по кнопке [
        if (key == cocos2d::enumKeyCodes::KEY_LeftBracket) {
            g_isModActive = !g_isModActive;
        }
    }
};

class $modify(MyPlayer, PlayerObject) {
    // Пустой перехват прыжка для проверки сборки
    void pushButton(PlayerButton btn) {
        PlayerObject::pushButton(btn);
    }
};
