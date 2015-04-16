#include "entity.h"
#include <glm/gtc/matrix_transform.hpp>

void entity::calculateTransform()
{
	m_Transform = glm::mat4();
	
	//Apply rotation
	glm::rotate(m_Transform, orientation.x, glm::vec3(1.0f,0.0f,0.0f));
	glm::rotate(m_Transform, orientation.y, glm::vec3(0.0f,1.0f,0.0f));
	glm::rotate(m_Transform, orientation.z, glm::vec3(0.0f,0.0f,1.0f));
	
	//Apply translation
	glm::translate(m_Transform, position);
}

void entity::calculateView()
{
	m_Transform = glm::mat4();
	
	//Apply rotation
	glm::rotate(m_Transform, -orientation.x, glm::vec3(1.0f,0.0f,0.0f));
	glm::rotate(m_Transform, -orientation.y, glm::vec3(0.0f,1.0f,0.0f));
	glm::rotate(m_Transform, -orientation.z, glm::vec3(0.0f,0.0f,1.0f));
	
	//Apply translation
	glm::translate(m_Transform, -position);
}

glm::mat4& 		 entity::transform() { return m_Transform; } 
const glm::mat4& entity::transform() const { return m_Transform; }
glm::mat4&		 entity::operator()() { return m_Transform; }
const glm::mat4& entity::operator()() const { return m_Transform; }