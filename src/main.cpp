#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/LevelCompleteLayer.hpp>
#include <Geode/modify/UILayer.hpp>

using namespace geode::prelude;

// Глобальная переменная для переключения мода
bool g_isModActive = true;

// Перехват клавиши [ для включения/выключения
class $modify(MyUILayer, UILayer) {
    void keyDown(enumKeyCodes key) {
        UILayer::keyDown(key);
        if (key == enumKeyCodes::KEY_LeftBracket) {
            g_isModActive = !g_isModActive;
            Notification::create(
                g_isModActive ? "Smooth Spam: ON" : "Smooth Spam: OFF",
                g_isModActive ? NotificationIcon::Success : NotificationIcon::Error
            )->show();
        }
    }
};

// Надпись в конце уровня
class $modify(SpamWarningLayer, LevelCompleteLayer) {
    bool init() {
        if (!LevelCompleteLayer::init()) return false;
        if (g_isModActive) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            auto label = CCLabelBMFont::create("smooth spam used", "chatFont.fnt");
            label->setPosition({winSize.width - 5.0f, 5.0f});
            label->setAnchorPoint({1.0f, 0.0f});
            label->setScale(0.5f);
            label->setOpacity(150);
            label->setColor({255, 50, 50});
            this->addChild(label);
        }
        return true;
    }
};

// Логика выравнивания
class $modify(MyPlayer, PlayerObject) {
    float m_clickTimer = 0.0f;
    float m_lastY = 0.0f;
    float m_slope = 0.0f;
    bool m_isSpamming = false;

    void pushButton(PlayerButton btn) {
        if (!g_isModActive || !this->m_isDart || !Mod::get()->getSettingValue<bool>("enable-wave")) {
            PlayerObject::pushButton(btn);
            return;
        }

        float cps = (m_fields->m_clickTimer > 0.01f) ? 1.0f / m_fields->m_clickTimer : 0.0f;
        
        if (cps >= 8.0f) {
            float curY = this->getPositionY();
            float newSlope = (curY - m_fields->m_lastY) / m_fields->m_clickTimer;
            m_fields->m_slope = m_fields->m_isSpamming ? (m_fields->m_slope * 0.5f + newSlope * 0.5f) : newSlope;
            m_fields->m_isSpamming = true;
        }

        if (cps <= Mod::get()->getSettingValue<int64_t>("max-cps")) {
            PlayerObject::pushButton(btn);
        }
        
        m_fields->m_clickTimer = 0.0f;
        m_fields->m_lastY = this->getPositionY();
    }

    void update(float dt) {
        PlayerObject::update(dt);
        if (!g_isModActive) return;

        m_fields->m_clickTimer += dt;
        if (m_fields->m_clickTimer > 0.3f) m_fields->m_isSpamming = false;

        // Волна
        if (this->m_isDart && m_fields->m_isSpamming && Mod::get()->getSettingValue<bool>("enable-wave")) {
            float strength = Mod::get()->getSettingValue<double>("lock-strength");
            this->m_yVelocity *= 0.7f;
            this->setPositionY(this->getPositionY() + (m_fields->m_slope * dt * strength));
        }

        // Корабль
        if (this->m_isShip && Mod::get()->getSettingValue<bool>("enable-ship") && std::abs(this->m_yVelocity) < 5.0f) {
            this->m_yVelocity *= 0.93f;
        }
    }
};
