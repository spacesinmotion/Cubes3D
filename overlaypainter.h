#ifndef OVERLAYPAINTER_H
#define OVERLAYPAINTER_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

class OverlayPainter : protected QOpenGLFunctions
{
public:
  OverlayPainter();

  void begin(int w, int h);
  void end();

  void draw();

private:
  void initShader();

private:
  QOpenGLShaderProgram m_program;

  int m_mvpUniform;
  int m_colorUniform;
  int m_vertexAttribute;
};

#endif // OVERLAYPAINTER_H
