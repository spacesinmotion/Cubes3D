#include "plyimport.h"

#include <cassert>
#include <clocale>

#include "rply/rply.h"

plyImport::plyImport(const char *plyFile) : m_file(nullptr)
{
  m_file = ply_open(plyFile, 0, 0, 0);
  if (m_file && !ply_read_header(m_file))
  {
    assert(false);
    ply_close(m_file);
    m_file = nullptr;
  }
}

plyImport::~plyImport()
{
  ply_close(m_file);
}

bool plyImport::isValid() const
{
  return m_file != NULL;
}

Geometry::Vertex_t &vertex__cb_base(p_ply_argument argument, bool push = false)
{
  Geometry *geometry = nullptr;
  long unused;
  ply_get_argument_user_data(argument, ((void **)&geometry), &unused);
  long length, value_index;
  p_ply_property prop;
  ply_get_argument_property(argument, &prop, &length, &value_index);

  if (push)
    geometry->vertices.emplace_back();

  return geometry->vertices.back();
}

int x__cb(p_ply_argument argument)
{
  vertex__cb_base(argument, true).vert[0] = (float)ply_get_argument_value(argument);
  return 1;
}
int y__cb(p_ply_argument argument)
{
  vertex__cb_base(argument).vert[1] = (float)ply_get_argument_value(argument);
  return 1;
}
int z__cb(p_ply_argument argument)
{
  vertex__cb_base(argument).vert[2] = (float)ply_get_argument_value(argument);
  return 1;
}
int nx__cb(p_ply_argument argument)
{
  vertex__cb_base(argument).norm[0] = (float)ply_get_argument_value(argument);
  return 1;
}
int ny__cb(p_ply_argument argument)
{
  vertex__cb_base(argument).norm[1] = (float)ply_get_argument_value(argument);
  return 1;
}
int nz__cb(p_ply_argument argument)
{
  vertex__cb_base(argument).norm[2] = (float)ply_get_argument_value(argument);
  return 1;
}
int tx__cb(p_ply_argument argument)
{
  vertex__cb_base(argument);
  return 1;
}
int ty__cb(p_ply_argument argument)
{
  vertex__cb_base(argument);
  return 1;
}

int face__cb(p_ply_argument argument)
{
  Geometry *geometry = nullptr;
  long unused;
  ply_get_argument_user_data(argument, ((void **)&geometry), &unused);
  long length, value_index;
  ply_get_argument_property(argument, NULL, &length, &value_index);

  if (length == 3 && value_index >= 0 && value_index < 3)
  {
    geometry->tris.push_back((unsigned int)ply_get_argument_value(argument));
  }
  else if (length == 4 && value_index >= 0 && value_index < 4)
  {
    geometry->quads.push_back((unsigned int)ply_get_argument_value(argument));
  }

  return 1;
}

GeometryPtr plyImport::read()
{
  if (!isValid())
    return nullptr;

  auto geometry = std::make_unique<Geometry>();

  const auto nbVertices = ply_set_read_cb(m_file, "vertex", "x", x__cb, geometry.get(), 0);
  ply_set_read_cb(m_file, "vertex", "y", y__cb, geometry.get(), 0);
  ply_set_read_cb(m_file, "vertex", "z", z__cb, geometry.get(), 0);
  ply_set_read_cb(m_file, "vertex", "nx", nx__cb, geometry.get(), 0);
  ply_set_read_cb(m_file, "vertex", "ny", ny__cb, geometry.get(), 0);
  ply_set_read_cb(m_file, "vertex", "nz", nz__cb, geometry.get(), 0);
  ply_set_read_cb(m_file, "vertex", "s", tx__cb, geometry.get(), 0);
  ply_set_read_cb(m_file, "vertex", "t", ty__cb, geometry.get(), 0);

  const auto nbFaces = ply_set_read_cb(m_file, "face", "vertex_indices", face__cb, geometry.get(), 0);

  geometry->vertices.reserve(nbVertices * 3);
  geometry->tris.reserve(nbFaces * 3);
  geometry->quads.reserve(nbFaces * 3);

  char *saved_locale;
  saved_locale = setlocale(LC_NUMERIC, "C");
  if (!ply_read(m_file))
    return GeometryPtr();

  setlocale(LC_NUMERIC, saved_locale);

  return geometry;
}
