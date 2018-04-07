
#include "AABB.h"


AABB::AABB(glm::vec3 min, glm::vec3 max) {
	m_min = min;
	m_max = max;
}

bool AABB::checkCollision(AABB other)
{
	return true;
}

void AABB::collide(bool c) {
	colliding = c;
}

AABB::~AABB()
{

}
