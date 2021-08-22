#include "view3d.h"

#include "ray.h"

#include <QApplication>
#include <QDebug>
#include <QFileInfo>
#include <QMouseEvent>
#include <QOffscreenSurface>
#include <QOpenGLFramebufferObject>
#include <QPainter>
#include <QShortcut>

View3D::View3D(QWidget *parent)
  : QOpenGLWidget(parent)
{
  setMouseTracking(true);
  setFocusPolicy(Qt::StrongFocus);

  RenderObject::primitives = this;
}

View3D::~View3D() = default;

void View3D::showScene(std::unique_ptr<RenderContainer> scene)
{
  m_scene = std::move(scene);
}

void View3D::initializeGL()
{
  initializeOpenGLFunctions();

  glClearColor(0.1f, 0.22f, 0.2f, 1.0f);

  initShaders();

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_CULL_FACE);

  m_cam.setViewCenter(slm::vec3(0, 0, 2));

  connect(new QShortcut(Qt::Key_2, this), &QShortcut::activated, this, [this] { m_cam.set_front(); });
  connect(new QShortcut(Qt::Key_6, this), &QShortcut::activated, this, [this] { m_cam.set_right(); });
  connect(new QShortcut(Qt::Key_4, this), &QShortcut::activated, this, [this] { m_cam.set_left(); });
  connect(new QShortcut(Qt::Key_8, this), &QShortcut::activated, this, [this] { m_cam.set_back(); });

  startTimer(16);
  m_timer.start();
}

void View3D::paintGL()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  m_program.bind();
  m_program.setUniformValue("projection", QMatrix4x4(slm::transpose(m_cam.projection()).begin()));
  m_program.setUniformValue("model_view", QMatrix4x4(slm::transpose(m_cam.modelView()).begin()));
  m_program.setUniformValue("normal_matrix", QMatrix4x4(slm::inverse(m_cam.modelView()).begin()));

  m_program.setUniformValue("object_transformation", QMatrix4x4());
  m_program.setUniformValue("object_normal", QMatrix4x4());

  //  drawLine(Qt::red, {slm::vec3(0.0), slm::vec3(10.0, 0.0, 0.0)});
  //  drawLine(Qt::green, {slm::vec3(0.0), slm::vec3(0.0, 10.0, 0.0)});
  //  drawLine(Qt::blue, {slm::vec3(0.0), slm::vec3(0.0, 0.0, 10.0)});

  if (m_drawHelper)
  {
    drawLine(Qt::red, {slm::vec3(0), slm::transpose(m_cam.rotation())[0].xyz() / m_cam.zoom()});
    drawLine(Qt::green, {slm::vec3(0), slm::transpose(m_cam.rotation())[1].xyz() / m_cam.zoom()});

    drawLine(Qt::lightGray,
             {slm::vec3(-1.0, -1.0, 4.0), slm::vec3(1.0, -1.0, 4.0), slm::vec3(-1.0, -1.0, 4.0),
              slm::vec3(-1.0, 1.0, 4.0), slm::vec3(-1.0, 1.0, 4.0), slm::vec3(1.0, 1.0, 4.0), slm::vec3(1.0, -1.0, 4.0),
              slm::vec3(1.0, 1.0, 4.0), slm::vec3(-1.0, -1.0, 0.0), slm::vec3(1.0, -1.0, 0.0),
              slm::vec3(-1.0, -1.0, 0.0), slm::vec3(-1.0, 1.0, 0.0), slm::vec3(-1.0, 1.0, 0.0),
              slm::vec3(1.0, 1.0, 0.0), slm::vec3(1.0, -1.0, 0.0), slm::vec3(1.0, 1.0, 0.0)});
  }
  drawObjects();

  m_program.release();
}

void View3D::resizeGL(int width, int height)
{
  m_cam.setViewPort(slm::vec2(width, height));
  glViewport(0, 0, GLint(width), GLint(height));
}

void View3D::keyPressEvent(QKeyEvent *) {}

void View3D::keyReleaseEvent(QKeyEvent *) {}

slm::vec3 View3D::mouseInSpace(const QPoint &mp)
{
  auto r = calcPickRay(mp);
  slm::vec3 p(0);
  float t = 0;
  if (r.hitPlane(t, p, r.s()))
    return r.relPoint(t);
  return slm::vec3(0);
}

SharedDisplayObject View3D::loadObject(const QString &path)
{
  auto res = m_displayObjects.value(path).lock();
  if (!res)
  {
    res = std::make_shared<DisplayObject>(path);
    m_displayObjects.insert(path, res);
  }
  return res;
}

SharedDisplayObject View3D::cube()
{
  if (QFileInfo("assets/cube.ply").isFile())
    return loadObject("assets/cube.ply");
  return loadObject("../assets/cube.ply");
}

void View3D::clear_scene()
{
  m_scene = std::make_unique<RenderContainer>();
}

void View3D::show_in_scene(std::unique_ptr<RenderObject> ro)
{
  m_scene->add(std::move(ro));
}

void View3D::on_tick(const Tick &tick)
{
  m_ticker.emplace_back(tick);
}

void View3D::mousePressEvent(QMouseEvent *) {}

void View3D::mouseReleaseEvent(QMouseEvent *) {}

void View3D::mouseMoveEvent(QMouseEvent *me)
{
  if (QApplication::mouseButtons() == Qt::MiddleButton)
  {
    m_cam.rotationEvent(slm::vec2(me->x() - m_lastPos.x(), (me->y() - m_lastPos.y())));
  }
  else if (QApplication::mouseButtons() == Qt::LeftButton)
  {
    if (me->modifiers() == Qt::ShiftModifier)
      m_cam.zoomEvent(10.0 * slm::vec2(me->x() - m_lastPos.x(), (me->y() - m_lastPos.y())));
    else if (me->modifiers() == Qt::ControlModifier)
      m_cam.rotationEvent(slm::vec2(me->x() - m_lastPos.x(), (me->y() - m_lastPos.y())));
    else
      m_cam.translationEvent(slm::vec2(me->x() - m_lastPos.x(), (me->y() - m_lastPos.y())));
  }
  else if ((me->pos() - m_lastPos).manhattanLength() > 0)
    m_needPick = true;

  m_lastPos = me->pos();
  QOpenGLWidget::mouseMoveEvent(me);
}

void View3D::wheelEvent(QWheelEvent *we)
{
  m_cam.zoomEvent(slm::vec2(0.0f, -float(we->delta())));
  QOpenGLWidget::wheelEvent(we);
}

void View3D::timerEvent(QTimerEvent *te)
{
  if (m_needPick)
  {
    m_needPick = false;
    performPick();
  }
  m_cam.tick();

  for (const auto &t : m_ticker)
    t(double(m_timer.elapsed()) / 1000.0);

  applyCursor();
  update();

  QOpenGLWidget::timerEvent(te);
}

void View3D::drawObjects()
{
  if (m_scene)
    m_scene->draw(m_program, QMatrix4x4());
}

void View3D::drawLine(const QColor &c, const std::vector<slm::vec3> &l)
{
  m_program.bind();
  m_program.setUniformValue("objectColor", c);
  m_program.setUniformValue("useLight", false);

  int vertexLocation = m_program.attributeLocation("a_position");
  m_program.enableAttributeArray(vertexLocation);
  m_program.setAttributeArray(vertexLocation, reinterpret_cast<const GLfloat *>(l.data()->begin()), 3);

  glDrawArrays(GL_LINES, 0, GLsizei(l.size()));
  m_program.disableAttributeArray(vertexLocation);

  m_program.setUniformValue("useLight", true);
}

void View3D::initShaders()
{
  if (!m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":shader/vshader.glsl"))
    qFatal("Failed to load vertex shader");

  if (!m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":shader/fshader.glsl"))
    qFatal("Failed to load fragment shader");

  if (!m_program.link())
    qFatal("Failed to link shader programm");

  m_program.bind();
}

slm::vec3 View3D::unProject(const QPoint &mousePos, float winz) const
{
  const auto &vp = m_cam.viewPort();
  GLint viewport[4] = {0, 0, int(vp.x), int(vp.y)};
  const auto projection = m_cam.projection();
  const auto modelview = m_cam.modelView();

  float winx = mousePos.x();
  float winy = viewport[3] - mousePos.y();
  // Transformation of normalized coordinates between -1 and 1
  slm::vec4 in;
  in[0] = (winx - float(viewport[0])) / float(viewport[2]) * 2.0f - 1.0f;
  in[1] = (winy - float(viewport[1])) / float(viewport[3]) * 2.0f - 1.0f;
  in[2] = 2.0f * winz - 1.0f;
  in[3] = 1.0f;

  slm::vec4 out = slm::inverse(projection * modelview) * in;
  out[3] = 1.0f / out[3];
  return slm::vec3(out[0] * out[3], out[1] * out[3], out[2] * out[3]);
}

Ray View3D::calcPickRay(const QPoint &mousePos) const
{
  const slm::vec3 point = unProject(mousePos, 0.0);
  const slm::vec3 slope = unProject(mousePos, 1.0);
  return Ray(point, slope - point);
}

void View3D::performPick() {}

void View3D::applyCursor()
{
  if (QApplication::mouseButtons() == Qt::MiddleButton)
    setCursor(Qt::SizeAllCursor);
  else
    setCursor(Qt::ArrowCursor);
}

QImage View3D::toImage(int w, int h)
{
  QOpenGLContext context;
  context.setShareContext(this->context());
  if (!context.create())
  {
    qDebug() << "Can't create GL context.";
  }
  QOffscreenSurface surface;
  surface.setFormat(this->context()->format());
  surface.create();
  if (!surface.isValid())
  {
    qDebug() << "Surface not valid.";
  }

  if (!context.makeCurrent(&surface))
  {
    qDebug() << "Can't make context current.";
  }

  QOpenGLFramebufferObjectFormat f;
  f.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
  QOpenGLFramebufferObject fbo(w, h, f);
  m_cam.setViewPort(slm::vec2(w, h));
  fbo.bind();
  context.functions()->glViewport(0, 0, w, h);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);

  m_drawHelper = false;
  paintGL();
  m_drawHelper = true;

  auto i = fbo.toImage(true);
  m_cam.setViewPort(slm::vec2(width(), height()));
  return i;
}
