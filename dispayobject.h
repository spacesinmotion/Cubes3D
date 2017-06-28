#ifndef DISPAYOBJECT_H
#define DISPAYOBJECT_H

#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QString>

class QOpenGLShaderProgram;

class DisplayObject : protected QOpenGLFunctions
{
public:
  DisplayObject(const QString &filename);
  ~DisplayObject();

  void draw(QOpenGLShaderProgram &program);

private:  // helper
  void init();

  bool isInitialized() const;

private:  // data
  QString m_filename;

  QOpenGLBuffer m_arrayBuf;
  QOpenGLBuffer m_indexBuf;
  int m_numberIndices;
};

#endif  // DISPAYOBJECT_H
