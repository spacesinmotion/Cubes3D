#ifndef CAMERA_H
#define CAMERA_H

#include "slm/mat4.h"

class Camera
{
public:
  Camera();

public:
  void tick();

public:  // getter
  const slm::vec2 &viewPort() const;

  slm::mat4 projection() const;
  slm::mat4 modelView() const;

public:  // setter
  void setViewPort(const slm::vec2 &s) { m_viewPort = s; }
  void setViewCenter(const slm::vec3 &c) { m_center = c; }

  void rotationEvent(const slm::vec2 &move2D);
  void zoomEvent(const slm::vec2 &move2D);

private:
  slm::vec2 m_viewPort;

  float m_zoom{1.0};
  float m_zoomEnd{10.0f};

  float m_xRot;
  float m_zRot;

  slm::vec3 m_currentCenter;
  slm::vec3 m_center;
};

#endif  // CAMERA_H
