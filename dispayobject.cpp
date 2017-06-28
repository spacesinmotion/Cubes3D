#include "dispayobject.h"

#include <QOpenGLShaderProgram>

DisplayObject::DisplayObject(const QString &filename)
  : m_filename(filename),
    m_indexBuf(QOpenGLBuffer::IndexBuffer),
    m_numberIndices(0)
{
  initializeOpenGLFunctions();
}

DisplayObject::~DisplayObject()
{
  if (isInitialized()) {
    m_arrayBuf.destroy();
    m_indexBuf.destroy();
  }
}

void DisplayObject::draw(QOpenGLShaderProgram &program)
{
  if (!isInitialized())
    init();
  if (!isInitialized())
    return;

  m_arrayBuf.bind();
  m_indexBuf.bind();

  int vertexLocation = program.attributeLocation("a_position");
  int normalLocation = program.attributeLocation("a_normal");

  program.enableAttributeArray(vertexLocation);
  program.enableAttributeArray(normalLocation);

  quintptr offset = 0;
  program.setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3,
                             sizeof(GLfloat[6]));
  offset += sizeof(GLfloat[3]);

  program.setAttributeBuffer(normalLocation, GL_FLOAT, offset, 3,
                             sizeof(GLfloat[6]));

  glDrawElements(GL_QUADS, m_numberIndices, GL_UNSIGNED_INT, 0);
}

void DisplayObject::init()
{
  static GLfloat vertices[] = {
      0.5,  0.5,  0.5,  0,  0,  1,  -0.5, 0.5,  0.5,  0,  0,  1,
      -0.5, -0.5, 0.5,  0,  0,  1,  0.5,  -0.5, 0.5,  0,  0,  1,
      0.5,  0.5,  0.5,  1,  0,  0,  0.5,  -0.5, 0.5,  1,  0,  0,
      0.5,  -0.5, -0.5, 1,  0,  0,  0.5,  0.5,  -0.5, 1,  0,  0,
      0.5,  0.5,  0.5,  0,  1,  0,  0.5,  0.5,  -0.5, 0,  1,  0,
      -0.5, 0.5,  -0.5, 0,  1,  0,  -0.5, 0.5,  0.5,  0,  1,  0,
      -0.5, 0.5,  0.5,  -1, 0,  0,  -0.5, 0.5,  -0.5, -1, 0,  0,
      -0.5, -0.5, -0.5, -1, 0,  0,  -0.5, -0.5, 0.5,  -1, 0,  0,
      -0.5, -0.5, -0.5, 0,  -1, 0,  0.5,  -0.5, -0.5, 0,  -1, 0,
      0.5,  -0.5, 0.5,  0,  -1, 0,  -0.5, -0.5, 0.5,  0,  -1, 0,
      0.5,  -0.5, -0.5, 0,  0,  -1, -0.5, -0.5, -0.5, 0,  0,  -1,
      -0.5, 0.5,  -0.5, 0,  0,  -1, 0.5,  0.5,  -0.5, 0,  0,  -1};

  static GLuint indices[] = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11,
                             12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};

  m_arrayBuf.create();
  m_indexBuf.create();

  m_arrayBuf.bind();
  m_arrayBuf.allocate(vertices, sizeof(vertices));

  m_indexBuf.bind();
  m_indexBuf.allocate(indices, sizeof(indices));

  m_numberIndices = sizeof(indices) / sizeof(GLuint);
}

bool DisplayObject::isInitialized() const
{
  return m_numberIndices > 0 && m_arrayBuf.isCreated() &&
         m_indexBuf.isCreated();
}
