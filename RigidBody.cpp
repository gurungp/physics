
#include "RigidBody.h"
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <glm\gtx\string_cast.hpp>

RigidBody::RigidBody(float mass, mat3x3 Ibody, mat3x3 IbodyInv, vec3 pos, mat3x3 orientation, vec3 omega,
					 vec3 angMoment, vec3 linMoment, vec3 force, vec3 torque)
{
	m_mass = 10000;
	m_Ibody = Ibody;
	m_IbodyInv = IbodyInv;

	m_position = pos;
	m_rotation = orientation;
	
	m_force = force;
	m_torque = torque;
	m_I = m_Ibody * m_rotation * m_IbodyInv;


	m_linearMomentum = linMoment;

	m_velocity = linMoment ;
	//m_omega = m_I * angMoment;
	m_omega = omega;
	m_angularMomentum = angMoment;
	//m_omega = angMoment/m_mass;
}

void RigidBody::update(float delTime, std::vector<glm::vec3> *vertices,int size) {


	//rotational
	
	mat3x3 star(0, -m_omega.z, m_omega.y,
				m_omega.z, 0, -m_omega.x,
				-m_omega.y, m_omega.x, 0);
	
	m_linearMomentum += m_force*delTime;
	
	m_rotation += (star*m_rotation * (delTime*1000));
	m_rotation[0] = m_rotation[0] / glm::length(m_rotation[0]);

	m_rotation[1] = glm::cross(m_rotation[2], (m_rotation[0]));
	m_rotation[1] = m_rotation[1] / glm::length(m_rotation[1]);

	m_rotation[2] = glm::cross(m_rotation[0], (m_rotation[1]));
	m_rotation[2] = m_rotation[2] / glm::length(m_rotation[2]);

	m_velocity = m_velocity + (m_velocity * (delTime));

	for (int i = 0; i < size; i++) {
		
		glm::vec3 torque = glm::cross(((*vertices)[i] - m_linearMomentum), m_force); // applying in various random positions rather than a fixed position
		m_angularMomentum = torque*(delTime/10);
 		m_omega = m_angularMomentum * glm::inverse(m_I);
		(*vertices)[i] = ((*vertices)[i] * m_rotation) + (m_linearMomentum + m_position);
	}

}


RigidBody::~RigidBody() {

}
