#ifndef FEWRAP_H
#define FEWRAP_H

#include <QString>
#include <functional>
#include <memory>

typedef struct fe_Object fe_Object;
typedef struct fe_Context fe_Context;

class RenderObject;
class RenderContainer;
class SceneHandler;

class FeWrap
{
public:
  FeWrap();
  ~FeWrap();

  void load(const QString &f);

  QString eval(const QString &fe, SceneHandler &sh);

  QString format(const QString &fe);

private:
  static fe_Object *_mod(fe_Context *ctx, fe_Object *arg);

  static fe_Object *_max(fe_Context *ctx, fe_Object *arg);
  static fe_Object *_min(fe_Context *ctx, fe_Object *arg);

  static fe_Object *_vec3(fe_Context *ctx, fe_Object *arg);
  static fe_Object *_color(fe_Context *ctx, fe_Object *arg);

  static fe_Object *_cube(fe_Context *ctx, fe_Object *arg);

  static void add_all(RenderContainer &c, fe_Context *ctx, fe_Object **arg);

  static fe_Object *_group(fe_Context *ctx, fe_Object *arg);
  static fe_Object *_translate(fe_Context *ctx, fe_Object *arg);
  static fe_Object *_rotate(fe_Context *ctx, fe_Object *arg);
  static fe_Object *_rotateX(fe_Context *ctx, fe_Object *arg);
  static fe_Object *_rotateY(fe_Context *ctx, fe_Object *arg);
  static fe_Object *_rotateZ(fe_Context *ctx, fe_Object *arg);
  static fe_Object *_scale(fe_Context *ctx, fe_Object *arg);

  static fe_Object *_animation(fe_Context *ctx, fe_Object *arg);

  static fe_Object *_lfo(fe_Context *ctx, fe_Object *arg);

  static void init_fn(fe_Context *ctx);

private:
  static const int m_size{1024 * 100};
  void *m_data{nullptr};
  fe_Context *m_fe{nullptr};
};

#endif // FEWRAP_H
