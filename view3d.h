#ifndef VIEW3D_H
#define VIEW3D_H

#include "SceneHandler.h"
#include "camera.h"
#include "displayobject.h"
#include "renderobject.h"

#include <QElapsedTimer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <memory>

class DisplayObject;
class OverlayObject;
class Ray;

class View3D
  : public QOpenGLWidget
  , public PrimitiveProvider
  , public SceneHandler
  , protected QOpenGLFunctions
{
public:
  explicit View3D(QWidget *parent = nullptr);
  ~View3D();

  void showAnimation(const QString &name);
  void toggleHelper();

  slm::vec3 mouseInSpace(const QPoint &mp);

  SharedDisplayObject loadObject(const QString &path);
  SharedDisplayObject cube() final;

  void clear_scene();
  void add_animation(const QString &name, float l, const slm::vec3 &lp, RenderObjectPtr) final;
  void on_tick(const Tick &) final;

  QImage toImage(double t, int w, int h);
  QVector<QPixmap> allFrames(int w, int h);

  QStringList animations() const;

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

  struct Animation
  {
    QString name;
    float length{0.5};
    slm::vec3 light_pos{0.0, -2.0, 8.0};
    RenderObjectPtr scene;

    Animation(const QString &n, float l, const slm::vec3 &lp, RenderObjectPtr s)
      : name{n}
      , length{l}
      , light_pos{lp}
      , scene{std::move(s)}
    {}
  };
  const Animation *m_animation{nullptr};
  std::vector<Animation> m_animations;

  QElapsedTimer m_timer;
  std::vector<Tick> m_ticker;
};

#endif // VIEW3D_H
