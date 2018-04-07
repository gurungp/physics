#ifndef RIGIDBODY_H_
#define RIGIDBODY_H_

#include <glm\glm.hpp>
#include <iostream>
#include <vector>

using namespace glm;

class RigidBody {
public:
	//Constants
	float m_mass;
	mat3x3 m_Ibody,
		   m_IbodyInv;

	//State Variables
	vec3 m_position;
	mat3x3 m_rotation;
	vec3 m_linearMomentum;
	vec3 m_angularMomentum;

	//Derived Quantities
	mat3x3 m_I;
	vec3 m_velocity;
	vec3 m_omega;

	//Computed Quantities
	vec3 m_force;
	vec3 m_torque;
	mat3x3 m_star;
	vec3 position;


public:
	RigidBody(float mass, mat3x3 Ibody, mat3x3 IbodyInv, vec3 pos, mat3x3 orientation, vec3 omega,
			  vec3 angMoment, vec3 linMoment, vec3 force, vec3 torque);
	virtual ~RigidBody();
	void update(float delTime, std::vector<glm::vec3> *vertices,int size);
};




#endif
