#include "renderobject.h"

#include <QOpenGLShaderProgram>

PrimitiveProvider *RenderObject::primitives{nullptr};

RenderDisplayObject::RenderDisplayObject(const SharedDisplayObject &o)
  : m_displayObject(o)
{}

void RenderDisplayObject::draw(QOpenGLShaderProgram &program, const QMatrix4x4 &t, bool)
{
  program.setUniformValue("objectColor", m_color);

  auto s = t;
  s.scale(m_scale.x, m_scale.y, m_scale.z);
  program.setUniformValue("object_transformation", s);
  program.setUniformValue("object_normal", s.inverted().transposed());

  m_displayObject->draw(program);
}

void RenderContainer::add(std::unique_ptr<RenderObject> ro)
{
  m_children.emplace_back(std::move(ro));
}

void RenderContainer::draw(QOpenGLShaderProgram &p, const QMatrix4x4 &t, bool helper)
{
  for (const auto &ro : m_children)
    ro->draw(p, t, helper);
}

void HelperContainer::draw(QOpenGLShaderProgram &p, const QMatrix4x4 &t, bool helper)
{
  if (helper)
    RenderContainer::draw(p, t, helper);
}

void ScaleContainer::draw(QOpenGLShaderProgram &p, const QMatrix4x4 &t, bool helper)
{
  QMatrix4x4 s = t;
  s.scale(*m_scale[0], *m_scale[1], *m_scale[2]);
  RenderContainer::draw(p, s, helper);
}

void TranslateContainer::draw(QOpenGLShaderProgram &p, const QMatrix4x4 &t, bool helper)
{
  QMatrix4x4 s = t;
  s.translate(*m_translate[0], *m_translate[1], *m_translate[2]);
  RenderContainer::draw(p, s, helper);
}

void RotateContainer::draw(QOpenGLShaderProgram &p, const QMatrix4x4 &t, bool helper)
{
  QMatrix4x4 s = t;
  s.rotate(*m_angle, *m_axis[0], *m_axis[1], *m_axis[2]);
  RenderContainer::draw(p, s, helper);
}
