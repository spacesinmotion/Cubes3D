#ifndef SCENEHANDLER_H
#define SCENEHANDLER_H

#include <functional>
#include <memory>

class RenderObject;
class QString;

using Tick = std::function<void(float)>;

class SceneHandler
{
public:
  virtual ~SceneHandler() = default;

  virtual void add_animation(const QString &, std::unique_ptr<RenderObject>) = 0;
  virtual void on_tick(const Tick &) = 0;
};

#endif // SCENEHANDLER_H
