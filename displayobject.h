#ifndef DISPAYOBJECT_H
#define DISPAYOBJECT_H

#include "geometry.h"

#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QString>

class QOpenGLShaderProgram;

class DisplayObject : protected QOpenGLFunctions
{
public:
  explicit DisplayObject(const QString &filename);
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

  GeometryPtr m_data;
};

using WeakDisplayObject = std::weak_ptr<DisplayObject>;
using SharedDisplayObject = std::shared_ptr<DisplayObject>;
using UniqueDisplayObject = std::weak_ptr<DisplayObject>;

#endif  // DISPAYOBJECT_H
