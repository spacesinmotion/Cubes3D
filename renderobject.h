#ifndef RENDEROBJECT_H
#define RENDEROBJECT_H

#include <QColor>
#include <QMatrix4x4>

#include "displayobject.h"

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

  static std::unique_ptr<RenderObject> from(const QJsonObject &o);

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
  explicit RenderContainer(const QJsonObject &o);

  void add(std::unique_ptr<RenderObject> ro);

  void draw(QOpenGLShaderProgram &, const QMatrix4x4 &) override;

private:
  std::vector<std::unique_ptr<RenderObject>> m_children;
};

class ScaleContainer : public RenderContainer
{
public:
  ScaleContainer() = default;
  explicit ScaleContainer(const QJsonObject &o);
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
  explicit TranslateContainer(const QJsonObject &o);
  TranslateContainer(const slm::vec3 &t) : m_translate{t} {}

  void set_translate(const slm::vec3 &t) { m_translate = t; }

  void draw(QOpenGLShaderProgram &p, const QMatrix4x4 &t) final;

private:
  slm::vec3 m_translate{1.0};
};

class RotateContainer : public RenderContainer
{
public:
  RotateContainer() = default;
  explicit RotateContainer(const QJsonObject &o);
  RotateContainer(float a, const slm::vec3 &t) : m_angle{a}, m_axis{t} {}

  void set_rotate(float a, const slm::vec3 &ax)
  {
    m_angle = a;
    m_axis = ax;
  }

  void draw(QOpenGLShaderProgram &p, const QMatrix4x4 &t) final;

private:
  float m_angle{0.0f};
  slm::vec3 m_axis{1.0};
};

#endif  // RENDEROBJECT_H
