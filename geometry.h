#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <memory>
#include <vector>
#include "slm/vec3.h"

class Geometry
{
public:
  struct Vertex_t
  {
    slm::vec3 vert;
    slm::vec3 norm;
  };
  using Index_t = unsigned int;

  using t_VertexVec = std::vector<Vertex_t>;
  using t_IndexVec = std::vector<Index_t>;

  t_VertexVec vertices;
  t_IndexVec tris;
  t_IndexVec quads;
};

using GeometryPtr = std::unique_ptr<Geometry>;

#endif  // GEOMETRY_H
