#ifndef VIEW3D_H
#define VIEW3D_H

#include <QElapsedTimer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <memory>

#include "SceneHandler.h"
#include "camera.h"
#include "displayobject.h"
#include "renderobject.h"

class DisplayObject;
class OverlayObject;
class Ray;

class View3D
    : public QOpenGLWidget,
      public PrimitiveProvider,
      public SceneHandler,
      protected QOpenGLFunctions
{
public:
  explicit View3D(QWidget *parent = nullptr);
  ~View3D();

  void showScene(std::unique_ptr<RenderContainer> scene);

  slm::vec3 mouseInSpace(const QPoint &mp);

  SharedDisplayObject loadObject(const QString &path);
  SharedDisplayObject cube() final;

  void clear_scene() final;
  void show_in_scene(std::unique_ptr<RenderObject>) final;
  void on_tick(const Tick &) final;

  QImage toImage(int w, int h);

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

private: // helper
  void drawObjects();
  void drawLine(const QColor &c, const std::vector<slm::vec3> &l);

  void initShaders();

  slm::vec3 unProject(const QPoint &mousePos, float winz) const;
  Ray calcPickRay(const QPoint &mousePos) const;
  void performPick();

  void applyCursor();

private: // data
  bool m_needPick{true};
  bool m_drawHelper{true};

  Camera m_cam;
  QPoint m_lastPos;

  QOpenGLShaderProgram m_program;

  QHash<QString, WeakDisplayObject> m_displayObjects;
  std::unique_ptr<RenderContainer> m_scene;

  QElapsedTimer m_timer;
  std::vector<Tick> m_ticker;
};

#endif // VIEW3D_H
