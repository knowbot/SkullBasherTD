#pragma once

#include "GuiManager.hpp"

class GameManager;

class LevelGuiManager : public GuiManager
{
public:
    explicit LevelGuiManager();
    ~LevelGuiManager() override;
    void onGui() override;
private:
    void guiGameInfo();
    void guiCrosshair();
    void guiTowers();
    void guiBuildPopUp(ImVec2 size);
    void guiQuitScreen();
    void guiWinLooseScreen();
    void guiWaveInfo();

    std::shared_ptr<sre::Texture> powerbar;
    std::shared_ptr<sre::Texture> crosshair;
    std::shared_ptr<sre::Texture> crossBowIcon;
    glm::vec2 powerbarSize;
    std::shared_ptr<sre::Texture> heartIcons[3];
    glm::vec2 heartSize;
    glm::vec2 velocity;

    std::string message = "Hi! AWSD to control.";

    glm::ivec2 heroSize;

    ImVec4 selectedBorderColor;

    float totalTime = 0.0f;
};
