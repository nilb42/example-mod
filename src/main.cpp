#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

// Память "мозга" мода
float g_timeSinceClick = 0.0f;
float g_spamCenterY = 0.0f;
bool g_isSpamming = false;

class $modify(MyPlayer, PlayerObject) {
    
    void pushButton(PlayerButton btn) {
        PlayerObject::pushButton(btn);
        
        // Если мод выключен в настройках Geode - ничего не делаем
        if (!Mod::get()->getSettingValue<bool>("enable-wave")) return;

        // "Мозг" анализирует CPS (скорость кликов)
        if (g_timeSinceClick < 0.2f) { // Если клики идут быстро (высокий CPS)
            if (!g_isSpamming) {
                // Игрок начал спамить. Запоминаем начальную высоту (нашу "невидимую линию")
                g_spamCenterY = this->getPositionY();
                g_isSpamming = true;
            } else {
                // Игрок продолжает спамить. Плавно смещаем линию за ним, если он чуть уходит вверх/вниз
                g_spamCenterY = (g_spamCenterY * 0.7f) + (this->getPositionY() * 0.3f);
            }
        }
        
        // Обнуляем таймер при каждом клике
        g_timeSinceClick = 0.0f; 
    }

    void update(float dt) {
        PlayerObject::update(dt);
        
        if (!Mod::get()->getSettingValue<bool>("enable-wave")) return;

        // Считаем время после последнего клика
        g_timeSinceClick += dt;

        // Если игрок специально отпустил кнопку (не кликает дольше 0.15 секунд)
        if (g_timeSinceClick > 0.15f) {
            g_isSpamming = false; // Отключаем защиту, даем свободно падать
        }

        // Если прямо сейчас идет быстрый спам
        if (g_isSpamming) {
            float currentY = this->getPositionY();
            float diff = g_spamCenterY - currentY;
            
            // Если игрок сбился со спама и начал улетать слишком сильно (больше 2 пикселей)
            if (std::abs(diff) > 2.0f) {
                // Мод берет управление на себя и мягко притягивает фигурку обратно к невидимой линии
                this->setPositionY(currentY + diff * 0.1f);
            }
        }
    }
};
