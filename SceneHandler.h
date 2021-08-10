#ifndef SCENEHANDLER_H
#define SCENEHANDLER_H

#include <memory>

class RenderObject;

class SceneHandler
{
public:
  virtual ~SceneHandler() = default;

  virtual void clear_scene() = 0;
  virtual void show_in_scene(std::unique_ptr<RenderObject>) = 0;
};

#endif  // SCENEHANDLER_H
