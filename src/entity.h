#ifndef _H_ENTITY_H_
#define _H_ENTITY_H_

#include <glm/glm.hpp>

class entity
{
	private: 
		glm::mat4 m_Transform();
		
	public: 
		glm::vec3 position;
		glm::vec3 orientation;
		glm::vec3 velocity;
		
		void calculateTransform();
		void calculateView();
		
		glm::mat4& 		 transform();
		const glm::mat4& transform() const;
		glm::mat4&		 operator()();
		const glm::mat4& operator()() const;
};

#endif