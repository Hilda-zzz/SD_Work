#pragma once
#include "Entity.hpp"
#include "Engine/Math/OBB2.hpp"
class Bullet :public Entity
{
public:
    Bullet(EntityType type,EntityFaction faction);
    ~Bullet(){}
    void Update(float deltaTime) override;
    void Render() const override;
    void Die() override;
    void HitWall(IntVec2 const& normal);
    void HitWall(Vec2 const& normal);
    void HitEntity();
public:
    Texture* m_texBullet=nullptr;
    float m_causeHurtPoint;
private:
    std::vector<Vertex_PCU> m_verts_body;
    OBB2	m_bulletBox;
    float	m_speed; 
    int m_bounceTime = 0;
    float m_spinOri = 0.f;
    float m_startTime = 0.f;
};