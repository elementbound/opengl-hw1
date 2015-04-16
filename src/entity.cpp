#include "entity.h"
#include "glwrap/util.h"
#include <glm/gtc/matrix_transform.hpp>

void entity::calculateTransform()
{
	m_Transform = glm::mat4(1.0f);
	
	//Apply rotation
	m_Transform = glm::rotate(m_Transform, orientation.x, glm::vec3(1.0f,0.0f,0.0f));
	m_Transform = glm::rotate(m_Transform, orientation.y, glm::vec3(0.0f,1.0f,0.0f));
	m_Transform = glm::rotate(m_Transform, orientation.z, glm::vec3(0.0f,0.0f,1.0f));
	
	//Apply translation
	m_Transform = glm::translate(m_Transform, position);
}

void entity::calculateView()
{
	glm::vec3 from, to;
	from = position;
	to = from + dirvec(orientation.z, orientation.x);
	
	m_Transform = glm::lookAt(from, to, glm::vec3(0.0, 0.0, 1.0));
}

glm::mat4& 		 entity::transform() { return m_Transform; } 
const glm::mat4& entity::transform() const { return m_Transform; }
glm::mat4&		 entity::operator()() { return m_Transform; }
const glm::mat4& entity::operator()() const { return m_Transform; }