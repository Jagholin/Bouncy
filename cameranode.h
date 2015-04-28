#pragma once
#include "scenenode.h"

class CameraNode : public SceneNode
{
public:
	void setFov(float fov);
	void setNear(float near);
	void setFar(float far);

	friend SceneNode;

	glm::vec3 eyePos();
	glm::vec3 direction();
	glm::vec3 up();

	glm::mat4 viewMatrix()const;
	glm::mat4 projMatrix(float aspectR)const;
protected:
	float m_fov, m_near, m_far;

	CameraNode(GraphicsScene* scene, std::string const& name);
};