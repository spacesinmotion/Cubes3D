#ifndef MAINMENUBUTTON_H
#define MAINMENUBUTTON_H

#include "overlayobject.h"

class QRect;

class MainMenuButton : public OverlayObject
{
public:
  MainMenuButton();

  // OverlayObject interface
public:
  void draw(const QSize &s, QPainter &p) override;
  bool hit(const QSize &size, const QPoint &mouse) const override;

  void onMouseEnter() override;
  void onMouseLeave() override;
  void onMouseRelease(Qt::MouseButton b) override;

private:
  QRect boundingRect(const QSize &s) const;

  void updateMouseCounter();
  qreal interpolation() const;

private:
  bool m_mouseOver;
  int m_mouseOverCounter;
};

#endif  // MAINMENUBUTTON_H
