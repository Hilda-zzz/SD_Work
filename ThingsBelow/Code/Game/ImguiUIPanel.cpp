#include "Game/ImguiUIPanel.hpp"

ImguiUIPanel::ImguiUIPanel(const std::string& panelName)
    : m_panelName(panelName)
    , m_isVisible(true)
{
}

ImguiUIPanel::~ImguiUIPanel()
{
}

void ImguiUIPanel::Update(float deltaSeconds)
{
    // Base implementation does nothing
    // Derived classes can override if needed
    (void)deltaSeconds;
}
