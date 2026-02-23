#pragma once
#include <string>

class ImguiUIPanel
{
public:
    ImguiUIPanel(const std::string& panelName);
    virtual ~ImguiUIPanel();

    // Core interface
    virtual void Render() = 0;
    virtual void Update(float deltaSeconds);

    // Visibility control
    void Show() { m_isVisible = true; }
    void Hide() { m_isVisible = false; }
    void ToggleVisibility() { m_isVisible = !m_isVisible; }
    bool IsVisible() const { return m_isVisible; }
    void SetVisible(bool visible) { m_isVisible = visible; }

    // Panel properties
    const std::string& GetPanelName() const { return m_panelName; }

protected:
    std::string m_panelName;
    bool m_isVisible;
};
