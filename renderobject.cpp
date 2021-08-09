#include "renderobject.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QOpenGLShaderProgram>

PrimitiveProvider *RenderObject::primitives{nullptr};

inline float tofloat(const QJsonObject &o, const QString &key)
{
  return float(o[key].toDouble());
}

inline slm::vec3 tovec3(const QJsonObject &o, const QString &key, const slm::vec3 &d = {})
{
  const auto vec = o[key].toArray();
  if (vec.size() != 3)
    return d;
  return {float(vec[0].toDouble()), float(vec[1].toDouble()), float(vec[2].toDouble())};
}

RenderDisplayObject::RenderDisplayObject(const SharedDisplayObject &o) : m_displayObject(o) {}

void RenderDisplayObject::draw(QOpenGLShaderProgram &program, const QMatrix4x4 &t)
{
  program.setUniformValue("objectColor", m_color);

  auto s = t;
  s.scale(m_scale.x, m_scale.y, m_scale.z);
  program.setUniformValue("object_transformation", s);

  m_displayObject->draw(program);
}

RenderContainer::RenderContainer(const QJsonObject &o)
{
  for (auto v : o["children"].toArray())
    m_children.emplace_back(from(v.toObject()));
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

ScaleContainer::ScaleContainer(const QJsonObject &o) : RenderContainer{o}, m_scale{tovec3(o, "scale")} {}

void ScaleContainer::draw(QOpenGLShaderProgram &p, const QMatrix4x4 &t)
{
  QMatrix4x4 s = t;
  s.scale(m_scale.x, m_scale.y, m_scale.z);
  RenderContainer::draw(p, s);
}

TranslateContainer::TranslateContainer(const QJsonObject &o) : RenderContainer(o), m_translate{tovec3(o, "translate")}
{}

void TranslateContainer::draw(QOpenGLShaderProgram &p, const QMatrix4x4 &t)
{
  QMatrix4x4 s = t;
  s.translate(m_translate.x, m_translate.y, m_translate.z);
  RenderContainer::draw(p, s);
}

RotateContainer::RotateContainer(const QJsonObject &o)
  : RenderContainer{o}, m_angle{tofloat(o, "angle")}, m_axis{tovec3(o, "axis")}
{}

void RotateContainer::draw(QOpenGLShaderProgram &p, const QMatrix4x4 &t)
{
  QMatrix4x4 s = t;
  s.rotate(m_angle, m_axis.x, m_axis.y, m_axis.z);
  RenderContainer::draw(p, s);
}

std::unique_ptr<RenderObject> RenderObject::from(const QJsonObject &o)
{
  if (o.contains("cube"))
  {
    auto p = std::make_unique<RenderDisplayObject>(primitives->cube());
    p->setColor(QColor{o["color"].toString()});
    p->set_scale(tovec3(o, "cube", slm::vec3{1.0f}));
    return p;
  }
  if (o.contains("translate"))
    return std::make_unique<TranslateContainer>(o);
  else if (o.contains("rotate"))
    return std::make_unique<RotateContainer>(o);
  else if (o.contains("scale"))
    return std::make_unique<ScaleContainer>(o);

  qFatal("unknown Object type");
}
