#pragma once
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <memory>
class CameraNode;
class Camera
{
public:
	glm::mat4 projectionMatrix() const;
	glm::mat4 viewMatrix() const;

	void setSideRatio(float sideRatio);

	float setPhi(float phi);
	float setTheta(float theta);

	void recalcView();

	void goforward(float distance);
	void gobackward(float distance);

	void setActiveCameraNode(std::weak_ptr<CameraNode> const&);
protected:
	// ... todo

	float m_sideRatio = 1.0f;
	glm::vec3 m_cameraPosition, m_cameraDirection = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 m_upVector = glm::vec3(0.0f, 0.0f, 1.0f);

	float m_phi = 0.0f, m_theta = 0.0f;

	std::weak_ptr<CameraNode> m_cameraNode;
};