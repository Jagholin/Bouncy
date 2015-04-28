#define GLM_SWIZZLE
#include "cameranode.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>

CameraNode::CameraNode(GraphicsScene* scene, std::string const& name) :
SceneNode(scene, name)
{

}

void CameraNode::setFov(float fov)
{
	m_fov = fov;
}

void CameraNode::setNear(float near)
{
	m_near = near;
}

void CameraNode::setFar(float far)
{
	m_far = far;
}

glm::vec3 CameraNode::eyePos()
{
	glm::mat4 cameraTransform = worldTransform();
	return (cameraTransform * glm::vec4(0, 0, 0, 1.0f)).xyz;
}

glm::mat4 CameraNode::viewMatrix() const
{
	return glm::inverse(worldTransform());
}

glm::mat4 CameraNode::projMatrix(float aspectR) const
{
	return glm::perspective(m_fov, aspectR, m_near, m_far);
}
