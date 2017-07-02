#ifndef PLYIMPORT_H
#define PLYIMPORT_H

#include <memory>
#include "geometry.h"

typedef struct t_ply_ *p_ply;

class plyImport
{
public:
  plyImport(const char *plyFile);
  ~plyImport();

  bool isValid() const;
  GeometryPtr read();

private:
  p_ply m_file;
};

#endif  // PLYIMPORT_H
