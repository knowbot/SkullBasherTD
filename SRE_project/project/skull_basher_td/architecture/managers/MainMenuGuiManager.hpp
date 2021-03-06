#pragma once

#include "sre/SDLRenderer.hpp"
#include "sre/Material.hpp"
#include "GuiManager.hpp"


class MainMenuGuiManager : public GuiManager
{
public:
    explicit MainMenuGuiManager();
    ~MainMenuGuiManager() override;
    void onGui() override;
private:
    void guiMainMenu();
    void guiPickLevel();
    void guiLevelGrid();

    bool pickLevel = false;
};
