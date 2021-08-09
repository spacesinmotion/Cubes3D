#ifndef OVERLAYOBJECT_H
#define OVERLAYOBJECT_H

class QPainter;
class QSize;
class QPoint;

#include <qnamespace.h>

class OverlayObject
{
public:
  OverlayObject() = default;
  virtual ~OverlayObject() = default;

  virtual void draw(const QSize &, QPainter &) = 0;
  virtual bool hit(const QSize &, const QPoint &mouse) const = 0;

  virtual void onMouseEnter() {}
  virtual void onMouseLeave() {}
  virtual void onMousePress(Qt::MouseButton) {}
  virtual void onMouseMove(Qt::MouseButton, const QPoint &, const QPoint &) {}
  virtual void onMouseRelease(Qt::MouseButton) {}
};

#endif  // OVERLAYOBJECT_H
