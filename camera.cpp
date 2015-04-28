#define _USE_MATH_DEFINES
#include "camera.h"
#include "cameranode.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

glm::mat4 Camera::projectionMatrix() const
{
	//return glm::ortho(-1.0f*m_sideRatio, 1.0f*m_sideRatio, -1.0f, 1.0f, 0.001f, 100000.0f);
	if (m_cameraNode.expired())
		return glm::perspective<float>(M_PI / 3.0f, m_sideRatio, 0.001f, 100000.0f);
	auto realNode = m_cameraNode.lock();
	return realNode->projMatrix(m_sideRatio);
}

glm::mat4 Camera::viewMatrix() const
{
	if (m_cameraNode.expired())
		return glm::lookAt(m_cameraPosition, m_cameraPosition + m_cameraDirection, m_upVector);
	auto realNode = m_cameraNode.lock();
	return realNode->viewMatrix();
}

void Camera::setSideRatio(float sideRatio)
{
	m_sideRatio = sideRatio;
}

float Camera::setPhi(float phi)
{
	m_phi = glm::clamp(phi, (float)-M_PI_2, (float)M_PI_2);
	recalcView();
	return m_phi;
}

float Camera::setTheta(float theta)
{
	m_theta = theta;
	recalcView();
	return m_theta;
}

void Camera::recalcView()
{
	m_cameraDirection = glm::vec3{ cos(m_theta)*cos(m_phi), sin(m_theta)*cos(m_phi), sin(m_phi) };
	m_cameraDirection = glm::normalize(m_cameraDirection);
}

void Camera::goforward(float distance)
{
	m_cameraPosition += m_cameraDirection*distance;
}

void Camera::gobackward(float distance)
{
	m_cameraPosition -= m_cameraDirection*distance;
}

void Camera::setActiveCameraNode(std::weak_ptr<CameraNode> const& node)
{
	m_cameraNode = node;
}

