#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

// Global memory for the smart logic
float g_timeSinceClick = 0.0f;
float g_spamCenterY = 0.0f;
bool g_isSpamming = false;

class $modify(MyPlayer, PlayerObject) {
    
    void pushButton(PlayerButton btn) {
        PlayerObject::pushButton(btn);
        
        // If the mod is disabled in Geode settings, do nothing
        if (!Mod::get()->getSettingValue<bool>("enable-wave")) return;

        // Spam tracking logic
        if (g_timeSinceClick < 0.2f) { // If clicking fast enough
            if (!g_isSpamming) {
                // Start a new spam session, remember the height
                g_spamCenterY = this->getPositionY();
                g_isSpamming = true;
            } else {
                // If spamming is inaccurate, the center smoothly follows the player
                g_spamCenterY = (g_spamCenterY * 0.7f) + (this->getPositionY() * 0.3f);
            }
        }
        
        // Reset the timer on every click
        g_timeSinceClick = 0.0f; 
    }

    void update(float dt) {
        PlayerObject::update(dt);
        
        if (!Mod::get()->getSettingValue<bool>("enable-wave")) return;

        // Track time since the last click
        g_timeSinceClick += dt;

        // IF THE PLAYER INTENTIONALLY RELEASES THE BUTTON (no clicks for 0.15 sec)
        // The mod disables protection, allowing the wave to fall naturally
        if (g_timeSinceClick > 0.15f) {
            g_isSpamming = false;
        }

        // SMART AUTO-CORRECTION FOR INACCURATE SPAM
        // Works only on the wave and only while actively spamming
        if (this->m_isDart && g_isSpamming) {
            float currentY = this->getPositionY();
            float diff = g_spamCenterY - currentY;
            
            // If the wave starts drifting too far (spam mistake)
            if (std::abs(diff) > 5.0f) {
                // Dampen the momentum to prevent crashing into the floor/ceiling
                this->m_yVelocity *= 0.85f; 
                // Gently pull the wave back to the safe trajectory
                this->setPositionY(currentY + diff * 0.15f);
            }
        }
    }
};
