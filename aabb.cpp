#include "aabb.h"

#include <algorithm>

AABB::AABB()
  : m_min(FLT_MAX)
  , m_max(-FLT_MAX)
{}

AABB::AABB(const slm::vec3 &min, const slm::vec3 &max)
  : m_min(min)
  , m_max(max)
{}

AABB::AABB(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
  : m_min(minX, minY, minZ)
  , m_max(maxX, maxY, maxZ)
{}

void AABB::reset()
{
  *this = AABB();
}

bool AABB::isValid() const
{
  return m_min[0] <= m_max[0] && m_min[1] <= m_max[1] && m_min[2] <= m_max[2];
}

AABB AABB::operator+(const AABB &other) const
{
  float miX = (min().x < other.min().x) ? min().x : other.min().x;
  float miY = (min().y < other.min().y) ? min().y : other.min().y;
  float miZ = (min().z < other.min().z) ? min().z : other.min().z;
  float maX = (max().x > other.max().x) ? max().x : other.max().x;
  float maY = (max().y > other.max().y) ? max().y : other.max().y;
  float maZ = (max().z > other.max().z) ? max().z : other.max().z;
  return AABB(miX, miY, miZ, maX, maY, maZ);
}

AABB AABB::operator*(const float expand) const
{
  float enlargeX = (max().x - min().x) * expand - (max().x - min().x);
  float enlargeY = (max().y - min().y) * expand - (max().y - min().y);
  float enlargeZ = (max().z - min().z) * expand - (max().z - min().z);
  enlargeX *= 0.5;
  enlargeY *= 0.5;
  enlargeZ *= 0.5;

  const float miX = min().x - enlargeX;
  const float miY = min().y - enlargeY;
  const float miZ = min().z - enlargeZ;
  const float maX = max().x + enlargeX;
  const float maY = max().y + enlargeY;
  const float maZ = max().z + enlargeZ;

  return AABB(miX, miY, miZ, maX, maY, maZ);
}

AABB &AABB::operator=(const AABB &other)
{
  m_min = other.m_min;
  m_max = other.m_max;
  return *this;
}

AABB &AABB::operator*=(float expand)
{
  float enlargeX = (max().x - min().x) * expand - (max().x - min().x);
  float enlargeY = (max().y - min().y) * expand - (max().y - min().y);
  float enlargeZ = (max().z - min().z) * expand - (max().z - min().z);
  enlargeX *= 0.5;
  enlargeY *= 0.5;
  enlargeZ *= 0.5;

  m_min[0] = min().x - enlargeX;
  m_min[1] = min().y - enlargeY;
  m_min[2] = min().z - enlargeZ;
  m_max[0] = max().x + enlargeX;
  m_max[1] = max().y + enlargeY;
  m_max[2] = max().z + enlargeZ;
  return *this;
}

AABB &AABB::operator+=(const AABB &other)
{
  m_min[0] = (min().x < other.min().x) ? min().x : other.min().x;
  m_min[1] = (min().y < other.min().y) ? min().y : other.min().y;
  m_min[2] = (min().z < other.min().z) ? min().z : other.min().z;
  m_max[0] = (max().x > other.max().x) ? max().x : other.max().x;
  m_max[1] = (max().y > other.max().y) ? max().y : other.max().y;
  m_max[2] = (max().z > other.max().z) ? max().z : other.max().z;
  return *this;
}

bool AABB::operator==(const AABB &other) const
{
  return (min().x == other.min().x) && (min().y == other.min().y) && (min().z == other.min().z) &&
         (max().x == other.max().x) && (max().y == other.max().y) && (max().z == other.max().z);
}

bool AABB::operator!=(const AABB &other) const
{
  return !(operator==(other));
}

slm::vec3 AABB::center() const
{
  return 0.5 * (min() + max());
}

float AABB::diameter() const
{
  return sqrt((max().x - min().x) * (max().x - min().x) + (max().y - min().y) * (max().y - min().y) +
              (max().z - min().z) * (max().z - min().z));
}

slm::vec3 AABB::scale() const
{
  return slm::vec3((max().x - min().x), (max().y - min().y), (max().z - min().z));
}

bool AABB::intersects(const AABB &other) const
{
  if (max().x < other.min().x || min().x > other.max().x || max().y < other.min().y || min().y > other.max().y ||
      max().z < other.min().z || min().z > other.max().z)
    return false;
  return true;
}

float AABB::averageDimensionSize() const
{
  const float erg = (max().x - min().x) + (max().y - min().y) + (max().z - min().z);
  return (erg / 3.0);
}

float AABB::maxDelta() const
{
  return std::max(deltaX(), std::max(deltaY(), deltaZ()));
}

float AABB::minDelta() const
{
  return std::min(deltaX(), std::min(deltaY(), deltaZ()));
}

bool AABB::isInside(const slm::vec3 &a) const
{
  return !(a[0] < m_min[0]) || (a[1] < m_min[1]) || (a[2] < m_min[2]) ||
         ((a[0] > m_max[0]) || (a[1] > m_max[1]) || (a[2] > m_max[2]));
}

AABB &AABB::translate(const slm::vec3 &trans)
{
  m_min += trans;
  m_max += trans;
  return *this;
}

bool AABB::intersectsRay(const slm::vec3 &rayOrigin, const slm::vec3 &rayDir, float &t)
{
  // r.dir is unit direction vector of ray
  const slm::vec3 dirfrac(1.0 / rayDir[0], 1.0 / rayDir[1], 1.0 / rayDir[2]);

  const float t1 = (m_min[0] - rayOrigin.x) * dirfrac.x;
  const float t2 = (m_max[0] - rayOrigin.x) * dirfrac.x;
  const float t3 = (m_min[1] - rayOrigin.y) * dirfrac.y;
  const float t4 = (m_max[1] - rayOrigin.y) * dirfrac.y;
  const float t5 = (m_min[2] - rayOrigin.z) * dirfrac.z;
  const float t6 = (m_max[2] - rayOrigin.z) * dirfrac.z;

  const float tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
  const float tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));

  // if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behing outside
  // if tmin > tmax, ray doesn't intersect AABB
  if (tmax < 0 || tmin > tmax)
  {
    t = tmax;
    return false;
  }

  t = tmin;
  return true;
}

slm::vec3 AABB::farPointIn(const float orientation[3]) const
{
  return slm::vec3(orientation[0] > 0.0 ? max().x : min().x, orientation[1] > 0.0 ? max().y : min().y,
                   orientation[2] > 0.0 ? max().z : min().z);
}

void AABB::insertPoint(const slm::vec3 &p)
{
  m_min = slm::min(m_min, p);
  m_max = slm::max(m_max, p);
}

slm::vec3 AABB::hyperCubeCoords(int i) const
{
  assert(i >= 0 && i < 8);
  return slm::vec3(((i & 1) == 1 ? max().x : min().x), ((i & 2) == 2 ? max().y : min().y),
                   ((i & 4) == 4 ? max().z : min().z));
}

bool AABB::isNull() const
{
  return 0.0 == m_min[0] && 0.0 == m_min[1] && 0.0 == m_min[2] && 0.0 == m_max[0] && 0.0 == m_max[1] && 0.0 == m_max[2];
}

void AABB::setNull()
{
  m_min[0] = m_min[1] = m_min[2] = 0.0;
  m_max[0] = m_max[1] = m_max[2] = 0.0;
}
