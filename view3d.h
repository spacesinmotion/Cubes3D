#ifndef VIEW3D_H
#define VIEW3D_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <memory>

#include "camera.h"
#include "displayobject.h"
#include "renderobject.h"

class DisplayObject;
class OverlayObject;
class Ray;

class View3D
  : public QOpenGLWidget
  , public PrimitiveProvider
  , protected QOpenGLFunctions
{
public:
  explicit View3D(QWidget *parent = nullptr);
  ~View3D();

  void showScene(std::unique_ptr<RenderObject> scene);

  slm::vec3 mouseInSpace(const QPoint &mp);

  SharedDisplayObject loadObject(const QString &path);
  SharedDisplayObject cube() final;

protected:
  void initializeGL() final;
  void paintGL() final;
  void resizeGL(int width, int height) final;

  void keyPressEvent(QKeyEvent *ke) final;
  void keyReleaseEvent(QKeyEvent *ke) final;

  void mousePressEvent(QMouseEvent *me) final;
  void mouseReleaseEvent(QMouseEvent *me) final;
  void mouseMoveEvent(QMouseEvent *me) final;
  void wheelEvent(QWheelEvent *we) final;

  void timerEvent(QTimerEvent *te) final;

private:  // helper
  void drawObjects();
  void drawLine(const QColor &c, const std::vector<slm::vec3> &l);

  void initShaders();

  slm::vec3 unProject(const QPoint &mousePos, float winz) const;
  Ray calcPickRay(const QPoint &mousePos) const;
  void performPick();

  void applyCursor();

private:  // data
  bool m_needPick;

  Camera m_cam;
  QPoint m_lastPos;

  QOpenGLShaderProgram m_program;

  QHash<QString, WeakDisplayObject> m_displayObjects;
  std::unique_ptr<RenderObject> m_scene;
};

#endif  // VIEW3D_H
