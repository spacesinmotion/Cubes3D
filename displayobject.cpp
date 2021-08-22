#include "displayobject.h"

#include "plyimport.h"

#include <QOpenGLShaderProgram>

DisplayObject::DisplayObject(const QString &filename)
  : m_filename(filename)
  , m_indexBuf(QOpenGLBuffer::IndexBuffer)
  , m_numberIndices(0)
{
  initializeOpenGLFunctions();
}

DisplayObject::~DisplayObject()
{
  if (isInitialized())
  {
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

  int offset = 0;
  program.setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(GLfloat[6]));
  offset += sizeof(GLfloat[3]);

  program.setAttributeBuffer(normalLocation, GL_FLOAT, offset, 3, sizeof(GLfloat[6]));

  glDrawElements(GL_TRIANGLES, m_numberIndices, GL_UNSIGNED_INT, nullptr);

  program.disableAttributeArray(vertexLocation);
  program.disableAttributeArray(normalLocation);

  m_indexBuf.release();
  m_arrayBuf.release();
}

void DisplayObject::init()
{
  m_data = plyImport(qPrintable(m_filename)).read();

  for (std::size_t i = 0; i < m_data->quads.size(); i += 4)
  {
    m_data->tris.push_back(m_data->quads[i + 0]);
    m_data->tris.push_back(m_data->quads[i + 1]);
    m_data->tris.push_back(m_data->quads[i + 2]);

    m_data->tris.push_back(m_data->quads[i + 2]);
    m_data->tris.push_back(m_data->quads[i + 3]);
    m_data->tris.push_back(m_data->quads[i + 0]);
  }
  m_data->quads.clear();

  m_arrayBuf.create();
  m_indexBuf.create();

  m_arrayBuf.bind();
  m_arrayBuf.allocate(m_data->vertices.data(), int(m_data->vertices.size() * sizeof(Geometry::Vertex_t)));

  m_indexBuf.bind();
  m_indexBuf.allocate(m_data->tris.data(), (int)m_data->tris.size() * sizeof(Geometry::Index_t));

  m_numberIndices = (int)m_data->tris.size();
}

bool DisplayObject::isInitialized() const
{
  return m_data && m_numberIndices > 0 && m_arrayBuf.isCreated() && m_indexBuf.isCreated();
}
