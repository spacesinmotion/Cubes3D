#pragma once

#include "slm/vec3.h"

class Ray
{
  using vec3 = slm::vec3;

public: // construction
  Ray(const vec3 &p, const vec3 &s);

  const vec3 &          p() const {return m_point;}
  const vec3 &          s() const {return m_slope;}

public: // hit calculation
  bool                  hitSphere(float &t, const vec3 c, float r) const;
  bool                  hitPlane(float &t, const vec3 &p, const vec3 &n) const;
  bool                  hitTri(float &t, const vec3 &a, const vec3 &b, const vec3 &c) const;
  bool                  hitQuad(float &t, const vec3 &a, const vec3 &b, const vec3 &c, const vec3 &d) const;

  vec3                  relPoint(float t) const { return m_point + (t*m_slope); }

private: // helper
  static bool           pointInTriangle(const vec3 &p, const vec3 &a, const vec3 &b, const vec3 &c);
  static float          pointRayDistance(const slm::vec3 &p0, const slm::vec3 &s0, const slm::vec3 &p);

private: // data
  vec3                  m_point;
  vec3                  m_slope;

  vec3                  m_up;
  float                 m_upD;
  vec3                  m_left;
  float                 m_leftD;
};
