#include "overlaypainter.h"

#include <QColor>
#include <slm/vec2.h>

OverlayPainter::OverlayPainter()
  : m_mvpUniform(-1)
  , m_colorUniform(-1)
  , m_vertexAttribute(-1)
{}

void OverlayPainter::begin(int w, int h)
{
  if (Q_UNLIKELY(!m_program.isLinked()))
    initShader();
  else
    m_program.bind();

  QMatrix4x4 proj;
  proj.ortho(0.0f, (float)w, 0.0f, (float)h, -1.0f, 1.0f);

  m_program.setUniformValue(m_mvpUniform, proj);
  m_program.enableAttributeArray(m_vertexAttribute);
}

void OverlayPainter::end()
{
  m_program.disableAttributeArray(m_vertexAttribute);
  m_program.release();
}

void OverlayPainter::draw()
{
  m_program.setUniformValue(m_colorUniform, QColor(150, 240, 230, 100));
  float x = 450;
  float y = 450;
  float r = 45;

  std::vector<slm::vec2> circleCoords;
  for (int i = 0; i < 360; i += 15)
  {
    const auto angleRAD = slm::radians(i);
    circleCoords.emplace_back(x + r * cos(angleRAD), y + r * sin(angleRAD));
  }

  m_program.enableAttributeArray(m_vertexAttribute);
  m_program.setAttributeArray(m_vertexAttribute, GL_FLOAT, (GLfloat *)circleCoords.data(), 2, 0);
  glDrawArrays(GL_TRIANGLE_FAN, 0, (GLsizei)circleCoords.size());
  m_program.disableAttributeArray(m_vertexAttribute);
}

void OverlayPainter::initShader()
{
  initializeOpenGLFunctions();

  if (!m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, "shader/overlay.vert"))
    throw std::runtime_error("error compile overlay vertex shader");

  if (!m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, "shader/overlay.frag"))
    throw std::runtime_error("error compile overlay fragment shader");

  if (!m_program.link())
    throw std::runtime_error("error link overlay shader");

  if (!m_program.bind())
    throw std::runtime_error("error first time bind overlay shader");

  m_mvpUniform = m_program.uniformLocation("mvp");
  m_colorUniform = m_program.uniformLocation("color");
  m_vertexAttribute = m_program.attributeLocation("vertex");

  qDebug() << m_mvpUniform << m_colorUniform << m_vertexAttribute;
}
