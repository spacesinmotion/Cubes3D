#include "camera.h"

#include <algorithm>

Camera::Camera() : m_viewPort(640, 480), m_xRot(45.0f) {}

void Camera::tick()
{
  //m_currentCenter = slm::mix(m_currentCenter, m_center, 0.1f);
  m_zoom = slm::mix(m_zoom, m_zoomEnd, 0.1f);
}

const slm::vec2 &Camera::viewPort() const
{
  return m_viewPort;
}

slm::mat4 Camera::projection() const
{
  if (m_viewPort.x > m_viewPort.y)
    return slm::orthoRH(m_zoom, (m_viewPort.y / m_viewPort.x) * m_zoom, 0.1f, 10.0f);
  return slm::orthoRH((m_viewPort.x / m_viewPort.y) * m_zoom, m_zoom, 0.1f, 10.0f);
}

slm::mat4 Camera::rotation() const
{
  auto res = slm::rotationX(slm::radians(m_xRot));
  res *= slm::rotationY(slm::radians(m_zRot));
  res *= slm::rotationX(slm::radians(-90.0));
  return res;
}

slm::mat4 Camera::modelView() const
{
  auto res = rotation();
  res *= slm::translation(-m_currentCenter);
  return res;
}

void Camera::translationEvent(const slm::vec2 &move2D)
{
  auto mv = slm::transpose(rotation());
  const auto m2 = move2D * m_zoom / std::max(m_viewPort.x, m_viewPort.y);
  m_currentCenter += (-m2.x * mv[0] + m2.y * mv[1]).xyz();
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

void Camera::set_front()
{
  m_zoomEnd = 10.0;
  m_xRot = 60.0;
  m_zRot = 0.0;
}

void Camera::set_back()
{
  m_zoomEnd = 10.0;
  m_xRot = 60.0;
  m_zRot = 180.0;
}

void Camera::set_right()
{
  m_zoomEnd = 10.0;
  m_xRot = 60.0;
  m_zRot = 90.0;
}

void Camera::set_left()
{
  m_zoomEnd = 10.0;
  m_xRot = 60.0;
  m_zRot = -90.0;
}

void Camera::jump()
{
  m_zoom = m_zoomEnd;
  //m_currentCenter = m_center;
}
