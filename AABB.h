#ifndef AABB_H_
#define AABB_H_

#include <glm\glm.hpp>

class AABB{
public:
	glm::vec3 m_min;
	glm::vec3 m_max;
	bool colliding=false;
	int id;


public:
	AABB(glm::vec3 min, glm::vec3 max);
	void collide(bool collide);
	bool AABB::checkCollision(AABB other);
	virtual ~AABB();
};

#endif