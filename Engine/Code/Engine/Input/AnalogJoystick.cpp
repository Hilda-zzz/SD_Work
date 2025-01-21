#include "AnalogJoystick.hpp"
#include "Engine/Math/MathUtils.hpp"

AnalogJoystick::AnalogJoystick()
{
    m_correctedPosition = Vec2{ 0.0f,0.0f};
    m_rawPosition = Vec2{ 0.0f,0.0f };
}

Vec2 AnalogJoystick::GetPosition() const
{
    return m_correctedPosition;
}

float AnalogJoystick::GetMagnitude() const
{
    return m_correctedPosition.GetLength();
}

float AnalogJoystick::GetOrientationDegrees() const
{
    return  m_correctedPosition.GetOrientationDegrees();
}

Vec2 AnalogJoystick::GetRawUncorrectedPosition() const
{
    return m_rawPosition;
}

float AnalogJoystick::GetInnerDeadZoneFraction() const
{
    return m_innerDeadZoneFraction;
}

float AnalogJoystick::GetOuterDeadZoneFraction() const
{
    return m_outerDeadZoneFraction;
}

void AnalogJoystick::Reset()
{
    //?
    m_correctedPosition =Vec2 { 0.f,0.f };
    m_rawPosition = Vec2{ 0.f,0.f };
}

void AnalogJoystick::SetDeadZoneThresholds(float normalizedInnerDeadzoneThreshold, float normalizedOuterDeadzonesThreshold)
{
    m_innerDeadZoneFraction = normalizedInnerDeadzoneThreshold;
    m_outerDeadZoneFraction = normalizedOuterDeadzonesThreshold;
}

void AnalogJoystick::UpdatePosition(float rawNormalizedX, float rawNormalizedY)
{
    m_rawPosition = Vec2{ rawNormalizedX ,rawNormalizedY };
    Vec2 tempPos = m_rawPosition;
    float rawLen= tempPos.GetLength();
    float correctedLen= RangeMapClamped(rawLen, m_innerDeadZoneFraction, m_outerDeadZoneFraction, 0.f, 1.f);
    tempPos.SetLength(correctedLen);
    m_correctedPosition = tempPos;
}
