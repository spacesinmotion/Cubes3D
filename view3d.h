#ifndef VIEW3D_H
#define VIEW3D_H

#include "camera.h"

#include <memory>

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>

class DisplayObject;
class Ray;

class View3D : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
  explicit View3D(QWidget *parent = nullptr);
  ~View3D();

  slm::vec3 mouseInSpace(const QPoint &mp);

protected:  // override
  void initializeGL() override;
  void paintGL() override;
  void resizeGL(int width, int height) override;

  void keyPressEvent(QKeyEvent *ke) override;
  void keyReleaseEvent(QKeyEvent *ke) override;

  void mousePressEvent(QMouseEvent *me) override;
  void mouseReleaseEvent(QMouseEvent *me) override;
  void mouseMoveEvent(QMouseEvent *me) override;
  void wheelEvent(QWheelEvent *we) override;

  void timerEvent(QTimerEvent *te);

private:  // helper
  void drawObjects();

  void initShaders();

  slm::vec3 unProject(const QPoint &mousePos, float winz) const;
  Ray calcPickRay(const QPoint &mousePos) const;
  void performPick();

  void applyCursor();

private:  // data
  bool m_needPick;

  Camera m_cam;
  QPoint m_lastPos;

  DisplayObject *m_cubeDisplay;

  QOpenGLShaderProgram m_program;
};

#endif  // VIEW3D_H
