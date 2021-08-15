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

  void set_front();
  void set_back();
  void set_right();
  void set_left();
  void jump();

private:
  slm::vec2 m_viewPort;

  float m_zoom{10.0};
  float m_zoomEnd{10.0f};

  float m_xRot{60.0f};
  float m_zRot{0.0f};

  slm::vec3 m_currentCenter{0.0};
  slm::vec3 m_center{0.0};
};

#endif  // CAMERA_H
