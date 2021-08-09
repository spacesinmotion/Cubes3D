#include "mainmenubutton.h"

#include <QDebug>
#include <QPainter>
#include <QSize>

#include <math.h>

#define M_PI 3.1415926535897932384626433832795

MainMenuButton::MainMenuButton() : m_mouseOver(false), m_mouseOverCounter(0) {}

qreal cosine(qreal a, qreal b, qreal t)
{
  qreal t2 = (1.0 - std::cos(t * M_PI)) / 2.0;
  return (a * (1.0 - t2) + b * t2);
}

QColor cosine(const QColor &a, const QColor &b, qreal t)
{
  return QColor::fromRgbF(cosine(a.redF(), b.redF(), t), cosine(a.greenF(), b.greenF(), t),
                          cosine(a.blueF(), b.blueF(), t), cosine(a.alphaF(), b.alphaF(), t));
}

void MainMenuButton::draw(const QSize &s, QPainter &p)
{
  auto r = boundingRect(s);
  const auto t = interpolation();

  r.setLeft(cosine(r.left(), r.left() - 24, t));
  r.setWidth(cosine(r.width(), r.width() + 48, t * t));

  p.setPen(Qt::transparent);

  p.setBrush(QColor(100, 100, 100, cosine(50, 90, t)));
  p.drawRoundedRect(r, 5, 5);

  auto backUp = p.transform();
  p.setTransform(QTransform::fromTranslate(r.left(), r.top()));

  static const QColor a(180, 180, 180, 100);
  static const QColor b(150, 210, 230, 180);
  p.setBrush(cosine(a, b, t * t));
  auto c = cosine(0, 24, t);
  p.drawRect(6 - 2, 6, 12 + 2 + c, 2);
  p.drawRect(6 - 2, 12 - 1, 12 + 2 + c, 2);
  p.drawRect(6 - 2, 18 - 2, 12 + 2 + c, 2);

  p.setTransform(backUp);

  updateMouseCounter();
}

bool MainMenuButton::hit(const QSize &size, const QPoint &mouse) const
{
  return boundingRect(size).contains(mouse);
}

void MainMenuButton::onMouseEnter()
{
  m_mouseOver = true;
}

void MainMenuButton::onMouseLeave()
{
  m_mouseOver = false;
}

void MainMenuButton::onMouseRelease(Qt::MouseButton b)
{
  qDebug() << b;
}

QRect MainMenuButton::boundingRect(const QSize &s) const
{
  static const QSize b(24, 24);
  return QRect(s.width() - b.width(), 0, b.width(), b.height());
}

void MainMenuButton::updateMouseCounter()
{
  if (m_mouseOver && m_mouseOverCounter < 5)
    ++m_mouseOverCounter;
  else if (!m_mouseOver && m_mouseOverCounter > 0)
    --m_mouseOverCounter;
}

qreal MainMenuButton::interpolation() const
{
  return qreal(m_mouseOverCounter) / 5.0;
}
