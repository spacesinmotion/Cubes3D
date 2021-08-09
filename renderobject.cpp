#include "renderobject.h"

#include <QOpenGLShaderProgram>

PrimitiveProvider *RenderObject::primitives{nullptr};

RenderDisplayObject::RenderDisplayObject(const SharedDisplayObject &o) : m_displayObject(o) {}

void RenderDisplayObject::draw(QOpenGLShaderProgram &program, const QMatrix4x4 &t)
{
  program.setUniformValue("objectColor", m_color);

  auto s = t;
  s.scale(m_scale.x, m_scale.y, m_scale.z);
  program.setUniformValue("object_transformation", s);

  m_displayObject->draw(program);
}

void RenderContainer::add(std::unique_ptr<RenderObject> ro)
{
  m_children.emplace_back(std::move(ro));
}

void RenderContainer::draw(QOpenGLShaderProgram &p, const QMatrix4x4 &t)
{
  for (const auto &ro : m_children)
    ro->draw(p, t);
}

void ScaleContainer::draw(QOpenGLShaderProgram &p, const QMatrix4x4 &t)
{
  QMatrix4x4 s = t;
  s.scale(m_scale.x, m_scale.y, m_scale.z);
  RenderContainer::draw(p, s);
}

void TranslateContainer::draw(QOpenGLShaderProgram &p, const QMatrix4x4 &t)
{
  QMatrix4x4 s = t;
  s.translate(m_translate.x, m_translate.y, m_translate.z);
  RenderContainer::draw(p, s);
}

void RotateContainer::draw(QOpenGLShaderProgram &p, const QMatrix4x4 &t)
{
  QMatrix4x4 s = t;
  s.rotate(m_angle, m_axis.x, m_axis.y, m_axis.z);
  RenderContainer::draw(p, s);
}
