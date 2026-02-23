#pragma once
#include "Engine/Math/Vec2.hpp"

class TentacleSegment
{
public:
    TentacleSegment();
    TentacleSegment(Vec2 const& start, Vec2 const& end, float startRadius, float endRadius);
    ~TentacleSegment();

    // Getters
    Vec2 GetStartPos() const { return m_startPos; }
    Vec2 GetEndPos() const { return m_endPos; }
    float GetStartRadius() const { return m_startRadius; }
    float GetEndRadius() const { return m_endRadius; }
    float GetLength() const;
    Vec2 GetDirection() const;

    // Setters
    void SetStartPos(Vec2 const& pos) { m_startPos = pos; }
    void SetEndPos(Vec2 const& pos) { m_endPos = pos; }
    void SetStartRadius(float radius) { m_startRadius = radius; }
    void SetEndRadius(float radius) { m_endRadius = radius; }

    // Rendering
    void Render() const;

private:
    Vec2 m_startPos;
    Vec2 m_endPos;
    float m_startRadius;
    float m_endRadius;
};
