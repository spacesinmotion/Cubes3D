#pragma once

#include "slm/vec3.h"

class AABB
{
public: // construction
  AABB();
  AABB(const slm::vec3 &min, const slm::vec3 &max);
  AABB(float, float, float, float, float, float);

public: // operator
  AABB &                operator = (const AABB &other);

  AABB                  operator + (const AABB &other) const;
  AABB                  operator * (float expand) const;

  AABB &                operator += (const AABB &other);
  AABB &                operator *= (float expand);
  
  bool                  operator == (const AABB &other) const;
  bool                  operator != (const AABB &other) const;

public: // getter
  bool                  isValid() const;
  bool                  isNull() const;

  float                 deltaX() const {return m_max[0]-m_min[0];}
  float                 deltaY() const {return m_max[1]-m_min[1];}
  float                 deltaZ() const {return m_max[2]-m_min[2];}

  const slm::vec3  &    min() const { return m_min; }
  const slm::vec3  &    max() const { return m_max; }

  slm::vec3             center() const;

  slm::vec3             farPointIn(const float orientation[3]) const;
  slm::vec3             hyperCubeCoords(int i) const;
  float                 diameter() const;
  slm::vec3             scale() const;
  float                 averageDimensionSize() const;

  float                 maxDelta() const;
  float                 minDelta() const;

public: // setter
  void                  reset();
  void                  setNull();

public: // transform
  AABB &                translate(const slm::vec3 &trans);
  void                  insertPoint(const slm::vec3 &point);

public: // intersection
  bool                  intersectsRay(const slm::vec3  &rayOrigin, const slm::vec3  &rayDir, float &t);
  bool                  intersects(const AABB &other) const;
  bool                  isInside(const slm::vec3 &) const;

private:  // attributes
  slm::vec3             m_min;
  slm::vec3             m_max;
};
