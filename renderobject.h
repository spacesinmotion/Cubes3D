#ifndef RENDEROBJECT_H
#define RENDEROBJECT_H

#include <QColor>
#include <QMatrix4x4>
#include <memory>

#include "displayobject.h"

using s_float = std::shared_ptr<float>;
inline s_float shared(float f)
{
  return std::make_shared<float>(f);
}

using s_vec3 = std::shared_ptr<slm::vec3>;
inline s_vec3 shared(slm::vec3 f)
{
  return std::make_shared<slm::vec3>(f);
}

class QOpenGLShaderProgram;

class PrimitiveProvider
{
public:
  virtual ~PrimitiveProvider() = default;

  virtual SharedDisplayObject cube() = 0;
};

class RenderObject
{
public:
  virtual ~RenderObject() = default;

  virtual void draw(QOpenGLShaderProgram &, const QMatrix4x4 &) = 0;

  static PrimitiveProvider *primitives;
};

class RenderDisplayObject : public RenderObject
{
public:
  explicit RenderDisplayObject(const SharedDisplayObject &disp);

  void draw(QOpenGLShaderProgram &program, const QMatrix4x4 &t) final;

  void setColor(const QColor &c) { m_color = c; }
  void set_scale(const slm::vec3 &s) { m_scale = s; }

private:
  SharedDisplayObject m_displayObject;
  QColor m_color{"#56a2b2"};
  slm::vec3 m_scale{1.0};
};

class RenderContainer : public RenderObject
{
public:
  RenderContainer() = default;

  void add(std::unique_ptr<RenderObject> ro);

  void draw(QOpenGLShaderProgram &, const QMatrix4x4 &) override;

private:
  std::vector<std::unique_ptr<RenderObject>> m_children;
};

class ScaleContainer : public RenderContainer
{
public:
  ScaleContainer() = default;
  ScaleContainer(const slm::vec3 &s) : m_scale{s} {}

  void set_scale(const slm::vec3 &s) { m_scale = s; }

  void draw(QOpenGLShaderProgram &p, const QMatrix4x4 &t) final;

private:
  slm::vec3 m_scale{1.0};
};

class TranslateContainer : public RenderContainer
{
public:
  TranslateContainer() = default;
  TranslateContainer(const s_vec3 &t) : m_translate{t} {}
  TranslateContainer(const slm::vec3 &t) : m_translate{shared(t)} {}

  void set_translate(const s_vec3 &t) { m_translate = t; }
  void set_translate(const slm::vec3 &t) { set_translate(shared(t)); }

  void draw(QOpenGLShaderProgram &p, const QMatrix4x4 &t) final;

private:
  s_vec3 m_translate{shared(slm::vec3(1.0))};
};

class RotateContainer : public RenderContainer
{
public:
  RotateContainer() = default;
  RotateContainer(float a, const slm::vec3 &t) : m_angle{shared(a)}, m_axis{t} {}
  RotateContainer(s_float a, const slm::vec3 &t) : m_angle{a}, m_axis{t} {}

  void set_rotate(s_float a, const slm::vec3 &ax)
  {
    m_angle = a;
    m_axis = ax;
  }
  void set_rotate(float a, const slm::vec3 &ax) { set_rotate(shared(a), ax); }

  void draw(QOpenGLShaderProgram &p, const QMatrix4x4 &t) final;

private:
  s_float m_angle{shared(0.0f)};
  slm::vec3 m_axis{1.0};
};

#endif  // RENDEROBJECT_H
