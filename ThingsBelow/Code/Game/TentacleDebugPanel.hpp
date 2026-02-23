#pragma once
#include "ImguiUIPanel.hpp"

class Tentacle;
struct Vec2;

class TentacleEditPanel : public ImguiUIPanel
{
public:
    TentacleEditPanel();
    virtual ~TentacleEditPanel();

    virtual void Render() override;

    // Set the tentacle to debug
    void SetTentacle(Tentacle* tentacle) { m_tentacle = tentacle; }
    Tentacle* GetTentacle() const { return m_tentacle; }

private:
    Tentacle* m_tentacle;
};
