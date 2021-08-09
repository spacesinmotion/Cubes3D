#ifndef FEWRAP_H
#define FEWRAP_H

#include <QString>
#include <functional>
#include <memory>

typedef struct fe_Object fe_Object;
typedef struct fe_Context fe_Context;

class RenderObject;
class RenderContainer;

class FeWrap
{
public:
  FeWrap();
  ~FeWrap();

  void load(const QString& f);

  std::unique_ptr<RenderObject> eval(const QString& fe, const std::function<void(const QString&)>& out);

  QString format(const QString& fe);

private:
  static fe_Object* vec3(fe_Context* ctx, fe_Object* arg);
  static fe_Object* color(fe_Context* ctx, fe_Object* arg);

  static fe_Object* cube(fe_Context* ctx, fe_Object* arg);

  static void add_all(RenderContainer& c, fe_Context* ctx, fe_Object** arg);

  static fe_Object* group(fe_Context* ctx, fe_Object* arg);
  static fe_Object* translate(fe_Context* ctx, fe_Object* arg);
  static fe_Object* rotate(fe_Context* ctx, fe_Object* arg);
  static fe_Object* rotateX(fe_Context* ctx, fe_Object* arg);
  static fe_Object* rotateY(fe_Context* ctx, fe_Object* arg);
  static fe_Object* rotateZ(fe_Context* ctx, fe_Object* arg);
  static fe_Object* scale(fe_Context* ctx, fe_Object* arg);

  static void init_fn(fe_Context* ctx);

private:
  static const int m_size{1024 * 1024};
  void* m_data{nullptr};
  fe_Context* m_fe{nullptr};
};

#endif  // FEWRAP_H
