#ifndef SCENEHANDLER_H
#define SCENEHANDLER_H

#include <functional>
#include <memory>

using RenderObjectPtr = std::shared_ptr<class RenderObject>;

class QString;
namespace slm
{
class vec3;
}

using Tick = std::function<void(float)>;

class SceneHandler
{
public:
  virtual ~SceneHandler() = default;

  virtual void add_animation(const QString &, float l, const slm::vec3 &, RenderObjectPtr) = 0;
  virtual void on_tick(const Tick &) = 0;
};

#endif // SCENEHANDLER_H
