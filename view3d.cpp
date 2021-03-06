#include "view3d.h"

#include "ray.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMouseEvent>
#include <QOffscreenSurface>
#include <QOpenGLFramebufferObject>
#include <QPainter>
#include <QShortcut>
#include <ranges>

View3D::View3D(QWidget *parent)
  : QOpenGLWidget(parent)
{
  setMouseTracking(true);
  setFocusPolicy(Qt::StrongFocus);

  RenderObject::primitives = this;
}

View3D::~View3D() = default;

void View3D::showAnimation(const QString &name)
{
  m_animation = nullptr;
  for (const auto &a : m_animations)
    if (a.name == name)
      m_animation = &a;
}

void View3D::toggleHelper()
{
  m_drawHelper = !m_drawHelper;
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
  if (m_animation)
  {
    const auto &l = m_animation->light_pos;
    m_program.setUniformValue("light_pos", l.x, l.y, l.z, 1.0);
  }
  m_program.setUniformValue("object_transformation", QMatrix4x4());
  m_program.setUniformValue("object_normal", QMatrix4x4());

  //  drawLine(Qt::red, {slm::vec3(0.0), slm::vec3(10.0, 0.0, 0.0)});
  //  drawLine(Qt::green, {slm::vec3(0.0), slm::vec3(0.0, 10.0, 0.0)});
  //  drawLine(Qt::blue, {slm::vec3(0.0), slm::vec3(0.0, 0.0, 10.0)});

  if (m_drawHelper)
  {
    // drawLine(Qt::red, {slm::vec3(0), slm::transpose(m_cam.rotation())[0].xyz() / m_cam.zoom()});
    // drawLine(Qt::green, {slm::vec3(0), slm::transpose(m_cam.rotation())[1].xyz() / m_cam.zoom()});

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
  const auto d = QFileInfo(QApplication::applicationFilePath()).absoluteDir();
  if (QFileInfo(d.filePath("assets/cube.ply")).isFile())
    return loadObject("assets/cube.ply");
  return loadObject(d.filePath("../assets/cube.ply"));
}

void View3D::clear_scene()
{
  m_animation = nullptr;
  m_animations.clear();
  m_ticker.clear();
  m_timer.restart();
}

void View3D::add_animation(const QString &name, float l, const slm::vec3 &lp, RenderObjectPtr o)
{
  m_animations.emplace_back(name, l, lp, std::move(o));
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

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
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
  if (m_animation)
    m_animation->scene->draw(m_program, QMatrix4x4(), m_drawHelper);
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

QImage View3D::toImage(double t, int w, int h)
{
  for (const auto &ticker : m_ticker)
    ticker(t);

  QOpenGLContext context;
  context.setShareContext(this->context());
  if (!context.create())
  {
    qDebug() << "Can't create GL context.";
  }
  QOffscreenSurface surface;
  auto format = this->context()->format();
  format.setAlphaBufferSize(8);
  surface.setFormat(format);
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
  const auto revert = m_cam.to_animation_view(w, h);
  fbo.bind();
  context.functions()->glViewport(0, 0, w, h);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);

  const auto drawHelper = m_drawHelper;
  m_drawHelper = false;
  paintGL();
  m_drawHelper = drawHelper;

  auto i = fbo.toImage(true);
  revert();
  return std::move(i);
}

QVector<QPixmap> View3D::allFrames(int w, int h)
{
  if (!m_animation)
    return {};
  const auto count = int(20.0 * m_animation->length);
  const auto step = m_animation->length / count;
  QVector<QPixmap> frames;
  for (int i = 0; i < count; ++i)
    frames << QPixmap::fromImage(toImage(double(i) * step, w, h));
  return frames;
}

QStringList View3D::animations() const
{
  QStringList names;
  for (const auto &a : m_animations)
    names.append(a.name);
  return names;
}
