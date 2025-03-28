#include "Projectile.hpp"

Projectile::Projectile(Vec3 const& position, EulerAngles const& orientation, Rgba8 const& color, bool isStatic, ZCylinder cylinder)
	:Actor(position, orientation, color, isStatic, cylinder)
{
}

Projectile::~Projectile()
{
}
