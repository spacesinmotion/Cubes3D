#include "ray.h"

#include <limits>

Ray::Ray(const vec3 &p, const vec3 &s)
  : m_point(p),
    m_slope(s),
    m_up(0.0, 0.0, 1.0),
    m_upD(0.0),
    m_left(0.0, 0.0, 1.0),
    m_leftD(0.0)
{
  const float tmpSP = slm::dot(vec3(0.0, 0.0, 1.0), m_slope);
  if ( tmpSP > 0.9 || tmpSP < -0.9 )
    m_up = slm::cross(m_slope, vec3(0.0, 1.0, 0.0));
  else
    m_up = slm::cross(m_slope, vec3(0.0, 0.0, 1.0));
  m_left = slm::cross( m_slope, m_up);
  m_upD = slm::dot(m_up, m_point);
  m_leftD = slm::dot(m_left, m_point);
}

inline float Ray::pointRayDistance(const slm::vec3 &p0, const slm::vec3 &s0, const slm::vec3 &p)
{
  const float n = slm::dot(s0,s0);
  if(fabs(n) < std::numeric_limits<float>::epsilon())
    return slm::distance(p0, p);
  float t0 = slm::dot(s0,p-p0) / n;
  if(t0<=0)
    return slm::distance(p,p0);
  return slm::distance(p, s0 * t0 + p0);
}

bool Ray::hitSphere(float &t, const vec3 c, float r) const
{
  if ( pointRayDistance(m_point, m_slope, c) < r )
  {
    t = slm::dot(c, m_slope) - slm::dot(m_point, m_slope);
    return t>=0;
  }
  return false;
}

bool Ray::hitPlane(float &t, const vec3 &p, const vec3 &n) const
{
  const float dg0 = slm::dot(n, m_point);
  const float dg1 = slm::dot(n, m_point + m_slope);
  if ( fabs(dg1 - dg0) < std::numeric_limits<float>::epsilon() )

    return false;

  const float de = slm::dot(n, p);
  t = (de - dg0) / (dg1 - dg0);

  return t >= 0.0;
}

bool Ray::hitTri(float &t, const vec3 &a, const vec3 &b, const vec3 &c) const
{
  /**
  * all points are over/ under the ray -> no hit
  */
  const bool uA = (m_upD < slm::dot(m_up, a));
  const bool uB = (m_upD < slm::dot(m_up, b));
  const bool uC = (m_upD < slm::dot(m_up, c));
  if ( uA == uB && uA == uC )
    return false;

  /**
  * all points are left/right the ray -> no hit
  */
  const bool lA = (m_leftD < slm::dot(m_left, a));
  const bool lB = (m_leftD < slm::dot(m_left, b));
  const bool lC = (m_leftD < slm::dot(m_left, c));
  if ( lA == lB && lA == lC )
    return false;

  vec3 normal = slm::facenormal_ccw(a, b, c);

  t = slm::dot(m_slope, normal); // temporary misused float value

  //MB: this test does not work in welding monitor at the moment, but it
  //    should filter faces pointing away from the watcher
  //     if ( dist >= 0.0)
  //       return false;

  t = -(slm::dot(m_point, normal) - slm::dot(a, normal)) / t;

  return t >= 0.0 && pointInTriangle(relPoint(t), a, b, c);
}

bool Ray::hitQuad(float &dist, const vec3 &a, const vec3 &b, const vec3 &c, const vec3 &d) const
{
  return hitTri(dist, a, b, c) || hitTri(dist, c, d, a);
}

bool Ray::pointInTriangle(const vec3 &p, const vec3 &a, const vec3 &b, const vec3 &c)
{
  const vec3 v0 = c-a;
  const vec3 v1 = b-a;
  const vec3 v2 = p-a;

  const float dot00 = slm::dot(v0,v0);
  const float dot01 = slm::dot(v0,v1);
  const float dot02 = slm::dot(v0,v2);
  const float dot11 = slm::dot(v1,v1);
  const float dot12 = slm::dot(v1,v2);

  float invDenom = (dot00*dot11 - dot01*dot01);
  if (invDenom == 0.0f)
    return false;
  invDenom = 1.0f / invDenom;

  float u = (dot11*dot02 - dot01*dot12) * invDenom;
  float v = (dot00*dot12 - dot01*dot02) * invDenom;

  return (u >= 0) && (v >= 0) && (u + v < 1);
}
