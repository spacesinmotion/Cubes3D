#ifndef RENDEROBJECT_H
#define RENDEROBJECT_H

#include "displayobject.h"

#include <QColor>
#include <QMatrix4x4>
#include <memory>

using s_float = std::shared_ptr<float>;
using s_float_deleter = std::function<void(float *)>;
inline s_float shared(float f, const s_float_deleter &d = std::default_delete<float>{})
{
  return std::shared_ptr<float>(new float(f), d);
}

using s_vec3 = std::shared_ptr<slm::vec3>;
using s_vec3_deleter = std::function<void(slm::vec3 *)>;
inline s_vec3 shared(const slm::vec3 &f, const s_vec3_deleter &d = std::default_delete<slm::vec3>{})
{
  return std::shared_ptr<slm::vec3>(new slm::vec3(f), d);
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

  virtual void draw(QOpenGLShaderProgram &, const QMatrix4x4 &, bool) = 0;

  static PrimitiveProvider *primitives;
};

class RenderDisplayObject : public RenderObject
{
public:
  explicit RenderDisplayObject(const SharedDisplayObject &disp);

  void draw(QOpenGLShaderProgram &program, const QMatrix4x4 &t, bool) final;

  void setColor(const QColor &c) { m_color = c; }
  void set_scale(const slm::vec3 &s) { m_scale = s; }

private:
  SharedDisplayObject m_displayObject;
  QColor m_color{0x56, 0xa2, 0xb2};
  slm::vec3 m_scale{1.0};
};

class RenderContainer : public RenderObject
{
public:
  RenderContainer() = default;

  void add(std::unique_ptr<RenderObject> ro);

  void draw(QOpenGLShaderProgram &, const QMatrix4x4 &, bool) override;

private:
  std::vector<std::unique_ptr<RenderObject>> m_children;
};

class HelperContainer : public RenderContainer
{
public:
  HelperContainer() = default;

  void draw(QOpenGLShaderProgram &, const QMatrix4x4 &, bool) final;
};

class ScaleContainer : public RenderContainer
{
public:
  ScaleContainer() = default;
  ScaleContainer(const s_vec3 &s)
    : m_scale{s}
  {}
  ScaleContainer(const slm::vec3 &s)
    : m_scale{shared(s)}
  {}

  void set_scale(const s_vec3 &s) { m_scale = s; }
  void set_scale(const slm::vec3 &s) { set_scale(shared(s)); }

  void draw(QOpenGLShaderProgram &p, const QMatrix4x4 &t, bool) final;

private:
  s_vec3 m_scale{shared(slm::vec3(1.0))};
};

class TranslateContainer : public RenderContainer
{
public:
  TranslateContainer() = default;
  TranslateContainer(const s_vec3 &t)
    : m_translate{t}
  {}
  TranslateContainer(const slm::vec3 &t)
    : m_translate{shared(t)}
  {}

  void set_translate(const s_vec3 &t) { m_translate = t; }
  void set_translate(const slm::vec3 &t) { set_translate(shared(t)); }

  void draw(QOpenGLShaderProgram &p, const QMatrix4x4 &t, bool) final;

private:
  s_vec3 m_translate{shared(slm::vec3(1.0))};
};

class RotateContainer : public RenderContainer
{
public:
  RotateContainer() = default;
  RotateContainer(s_float a, const s_vec3 &t)
    : m_angle{a}
    , m_axis{t}
  {}
  RotateContainer(s_float a, const slm::vec3 &t)
    : m_angle{a}
    , m_axis{shared(t)}
  {}

  void set_rotate(s_float a, const s_vec3 &ax)
  {
    m_angle = a;
    m_axis = ax;
  }
  void set_rotate(float a, const slm::vec3 &ax) { set_rotate(shared(a), shared(ax)); }
  void set_rotate(float a, const s_vec3 &ax) { set_rotate(shared(a), ax); }
  void set_rotate(s_float a, const slm::vec3 &ax) { set_rotate(a, shared(ax)); }

  void draw(QOpenGLShaderProgram &p, const QMatrix4x4 &t, bool) final;

private:
  s_float m_angle{shared(0.0f)};
  s_vec3 m_axis{shared(slm::vec3(1.0))};
};

#endif // RENDEROBJECT_H
