#ifndef FEWRAP_H
#define FEWRAP_H

#include <QHash>
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
  explicit FeWrap(SceneHandler &s);
  ~FeWrap();

  QString newSession(const QString &f);
  QString eval();

  QString codeOf(const QString &f);
  void setCodeOf(const QString &f, const QString &c);

  QString format(const QString &fe);

  using LineDefinitionCB = std::function<void(int, const QString &)>;
  void eachDefinitionAtLine(const QString &fe, const LineDefinitionCB &cb);

  void saveFiles();
  bool hasChanges() const { return m_hasChanges; }

  QStringList usedFiles() const;

  SceneHandler *scene() { return &m_scene; }

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

  static fe_Object *_eval(fe_Context *ctx, const QString &);
  static fe_Object *_require(fe_Context *ctx, fe_Object *arg);

  void init_fn(fe_Context *ctx);

private:
  static const int m_size{1024 * 100};
  void *m_data{nullptr};
  fe_Context *m_fe{nullptr};

  SceneHandler &m_scene;

  QString m_mainFile;
  QHash<QString, QString> m_fileContents;
  bool m_hasChanges{false};
};

#endif // FEWRAP_H
