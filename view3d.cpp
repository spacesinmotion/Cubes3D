#include "view3d.h"

#include "dispayobject.h"
#include "ray.h"

#include <QApplication>
#include <QMouseEvent>

View3D::View3D(QWidget *parent)
  : QOpenGLWidget(parent), m_needPick(false), m_cubeDisplay(nullptr)
{
  setMouseTracking(true);
  setFocusPolicy(Qt::StrongFocus);
}

View3D::~View3D()
{
}

void View3D::initializeGL()
{
  initializeOpenGLFunctions();

  glClearColor(0.0, 0.0, 0.0, 1.0);

  initShaders();

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glEnable(GL_NORMALIZE);
  // glShadeModel(GL_SMOOTH);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHTING);
  glEnable(GL_COLOR_MATERIAL);

  glPolygonOffset(0.5, 10.0);

  m_cam.setViewCenter(slm::vec3(0, 0, -1));

  m_cubeDisplay = new DisplayObject("dummy");

  startTimer(20);
}

void View3D::paintGL()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  auto mvp = QMatrix4x4(
      slm::transpose(m_cam.projection() * m_cam.modelView()).begin());

  auto normalMatrix =
      QMatrix4x4(slm::transpose(slm::inverse(m_cam.modelView())).begin());

  m_program.setUniformValue("mvp_matrix", mvp);
  m_program.setUniformValue("normal_matrix", normalMatrix);

  drawObjects();

  assert(glGetError() == GL_NO_ERROR);
}

void View3D::resizeGL(int width, int height)
{
  m_cam.setViewPort(slm::vec2(width, height));
  glViewport(0, 0, (GLint)width, (GLint)height);
}

void View3D::keyPressEvent(QKeyEvent *ke)
{
}

void View3D::keyReleaseEvent(QKeyEvent *ke)
{
}

slm::vec3 View3D::mouseInSpace(const QPoint &mp)
{
  auto r = calcPickRay(mp);
  slm::vec3 p(0);
  float t = 0;
  if (r.hitPlane(t, p, r.s()))
    return r.relPoint(t);
  return slm::vec3(0);
}

void View3D::mousePressEvent(QMouseEvent *me)
{
}

void View3D::mouseReleaseEvent(QMouseEvent *me)
{
}

void View3D::mouseMoveEvent(QMouseEvent *me)
{
  if (QApplication::mouseButtons() == Qt::MiddleButton) {
    m_cam.rotationEvent(
        slm::vec2(me->x() - m_lastPos.x(), (me->y() - m_lastPos.y())));
  } else if (QApplication::mouseButtons() == Qt::LeftButton) {
  } else if ((me->pos() - m_lastPos).manhattanLength() > 0)
    m_needPick = true;

  m_lastPos = me->pos();
  QOpenGLWidget::mouseMoveEvent(me);
}

void View3D::wheelEvent(QWheelEvent *we)
{
  m_cam.zoomEvent(slm::vec2(0.0f, (float)we->delta()));
  QOpenGLWidget::wheelEvent(we);
}

void View3D::timerEvent(QTimerEvent *te)
{
  if (m_needPick) {
    m_needPick = false;
    performPick();
  }
  m_cam.tick();

  applyCursor();
  update();

  QOpenGLWidget::timerEvent(te);
}

void View3D::drawObjects()
{
  m_cubeDisplay->draw(m_program);
}

void View3D::initShaders()
{
  if (!m_program.addShaderFromSourceFile(QOpenGLShader::Vertex,
                                         "shader/vshader.glsl"))
    close();

  if (!m_program.addShaderFromSourceFile(QOpenGLShader::Fragment,
                                         "shader/fshader.glsl"))
    close();

  if (!m_program.link())
    close();

  if (!m_program.bind())
    close();
}

slm::vec3 View3D::unProject(const QPoint &mousePos, float winz) const
{
  const auto &vp = m_cam.viewPort();
  GLint viewport[4] = {0, 0, vp.x, vp.y};
  const auto projection = m_cam.projection();
  const auto modelview = m_cam.modelView();

  float winx = mousePos.x();
  float winy = viewport[3] - mousePos.y();
  // Transformation of normalized coordinates between -1 and 1
  slm::vec4 in;
  in[0] = (winx - (float)viewport[0]) / (float)viewport[2] * 2.0 - 1.0;
  in[1] = (winy - (float)viewport[1]) / (float)viewport[3] * 2.0 - 1.0;
  in[2] = 2.0 * winz - 1.0;
  in[3] = 1.0;

  slm::vec4 out = slm::inverse(projection * modelview) * in;
  out[3] = 1.0 / out[3];
  return slm::vec3(out[0] * out[3], out[1] * out[3], out[2] * out[3]);
}

Ray View3D::calcPickRay(const QPoint &mousePos) const
{
  const slm::vec3 point = unProject(mousePos, 0.0);
  const slm::vec3 slope = unProject(mousePos, 1.0);
  return Ray(point, slope - point);
}

void View3D::performPick()
{
}

void View3D::applyCursor()
{
  if (QApplication::mouseButtons() == Qt::MiddleButton)
    setCursor(Qt::SizeAllCursor);
  else
    setCursor(Qt::ArrowCursor);
}
