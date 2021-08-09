#include "camera.h"

#include <algorithm>

Camera::Camera() : m_viewPort(640, 480), m_xRot(45.0f), m_zRot(15.0f), m_currentCenter(0.0f), m_center(0.0f) {}

void Camera::tick()
{
  m_currentCenter = slm::mix(m_currentCenter, m_center, 0.1f);
  m_zoom = slm::mix(m_zoom, m_zoomEnd, 0.1f);
}

const slm::vec2 &Camera::viewPort() const
{
  return m_viewPort;
}

slm::mat4 Camera::projection() const
{
  const auto a = std::max(m_viewPort.x, m_viewPort.y) / std::min(m_viewPort.x, m_viewPort.y);
  return slm::orthoRH(a * m_zoom, m_zoom, 0.1f, 10.0f);
}

slm::mat4 Camera::modelView() const
{
  slm::mat4 res = slm::translation(slm::vec3(0, 0, -5));

  res *= slm::rotationX(slm::radians(m_xRot));
  res *= slm::rotationY(slm::radians(m_zRot));

  res *= slm::rotationX(slm::radians(-90.0));

  res *= slm::translation(-m_currentCenter);

  return res;
}

void Camera::rotationEvent(const slm::vec2 &move2D)
{
  m_xRot += 0.25f * move2D.y;
  m_zRot += 0.25f * move2D.x;
}

void Camera::zoomEvent(const slm::vec2 &move2D)
{
  m_zoomEnd = std::max(0.5f, std::min(1000.0f, 0.005f * move2D.y + m_zoomEnd));
}
