#include "fewrap.h"

#include "SceneHandler.h"
#include "renderobject.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <string>
#include <variant>
#include <vector>

extern "C"
{
#include "fe/fe.h"
}

using slm::vec3;

using CustomPtr = std::variant<RenderObjectPtr, s_float, s_vec3, vec3, QColor, FeWrap *>;

static char read_fn(fe_Context *, void *vit)
{
  auto *it = reinterpret_cast<QByteArray::iterator *>(vit);
  return *((*it)++);
}

static void write_fn(fe_Context *, void *udata, char c)
{
  auto *string = reinterpret_cast<QString *>(udata);
  return string->push_back(QChar(c));
}

static QString from_string(fe_Context *ctx, fe_Object *o)
{
  if (!o)
    return QStringLiteral("nil");
  QString t;
  fe_write(ctx, o, write_fn, &t, false);
  return t;
}

template <typename T> fe_Object *custom(fe_Context *ctx, T o)
{
  return fe_ptr(ctx, new CustomPtr{std::move(o)});
}

template <typename T, typename... Args> fe_Object *create_custom(fe_Context *ctx, Args &&...args)
{
  return custom(ctx, new T(std::forward<Args>(args)...));
}

template <typename T> static bool is(fe_Context *ctx, fe_Object *o)
{
  if (fe_type(ctx, o) == FE_TPTR)
    if (auto *p = reinterpret_cast<CustomPtr *>(fe_toptr(ctx, o)))
      return std::get_if<T>(p) != nullptr;
  return false;
}

template <typename T> static T get(fe_Context *ctx, fe_Object *o)
{
  if (!is<T>(ctx, o))
  {
    static const auto err = "Expect " + std::string(typeid(T).name()) + "!";
    fe_error(ctx, err.c_str());
  }

  return std::get<T>(*reinterpret_cast<CustomPtr *>(fe_toptr(ctx, o)));
}

FeWrap *_self(fe_Context *ctx)
{
  return get<FeWrap *>(ctx, fe_eval(ctx, fe_symbol(ctx, "self")));
}

SceneHandler *_scene(fe_Context *ctx)
{
  return _self(ctx)->scene();
}

static std::vector<std::string> keys;
std::string unique_key()
{
  static int count = 0;

  std::string key;
  if (keys.empty())
  {
    key = "__g_" + std::to_string(count++);
  }
  else
  {
    key = keys.back();
    keys.pop_back();
  }
  return key;
}

template <typename R> std::shared_ptr<R> shared_d(fe_Context *ctx, fe_Object *o, const R &v)
{
  const auto key = unique_key();
  int gc = fe_savegc(ctx);
  fe_set(ctx, fe_symbol(ctx, key.c_str()), o);
  fe_restoregc(ctx, gc);

  auto r = shared(v, [ctx, key](auto *x) {
    delete x;
    int gc = fe_savegc(ctx);
    fe_restoregc(ctx, gc);
    keys.push_back(key);
  });
  return r;
}

static s_float s_number(fe_Context *ctx, fe_Object *o)
{
  if (is<s_float>(ctx, o))
    return get<s_float>(ctx, o);

  if (fe_type(ctx, o) == FE_TFUNC)
  {
    auto r = shared_d(ctx, o, 0.0f);

    _scene(ctx)->on_tick([r, ctx, o](auto t) {
      int gc = fe_savegc(ctx);

      fe_Object *objs[2];
      objs[0] = o;
      objs[1] = fe_number(ctx, t);
      *r = fe_tonumber(ctx, fe_eval(ctx, fe_list(ctx, objs, 2)));

      fe_restoregc(ctx, gc);
    });
    return r;
  }

  return shared(fe_tonumber(ctx, o));
}

static s_vec3 s_vec(fe_Context *ctx, fe_Object *o)
{
  if (is<s_vec3>(ctx, o))
    return get<s_vec3>(ctx, o);

  if (fe_type(ctx, o) == FE_TFUNC)
  {
    auto r = s_vec3{shared_d(ctx, o, 0.0f), shared_d(ctx, o, 0.0f), shared_d(ctx, o, 0.0f)};
    _scene(ctx)->on_tick([r, ctx, o](auto t) {
      int gc = fe_savegc(ctx);

      fe_Object *objs[2];
      objs[0] = o;
      objs[1] = fe_number(ctx, t);
      const auto v = get<s_vec3>(ctx, fe_eval(ctx, fe_list(ctx, objs, 2)));
      *r[0] = *v[0];
      *r[1] = *v[1];
      *r[2] = *v[2];
      fe_restoregc(ctx, gc);
    });
    return r;
  }

  return shared(get<slm::vec3>(ctx, o));
}

static RenderObjectPtr _uobj(fe_Context *ctx, fe_Object *o)
{
  if (!is<RenderObjectPtr>(ctx, o))
    fe_error(ctx, "Expect RenderObject!");

  auto *custom = reinterpret_cast<CustomPtr *>(fe_toptr(ctx, o));
  return std::get<RenderObjectPtr>(*custom);
}

FeWrap::FeWrap(SceneHandler &s)
  : m_data{malloc(m_size)}
  , m_fe{fe_open(m_data, m_size)}
  , m_scene{s}
{
  init_fn(m_fe);
}

FeWrap::~FeWrap()
{
  fe_close(m_fe);
  free(m_data);
}

QString FeWrap::newSession(const QString &f)
{
  m_hasChanges = false;
  m_mainFile = QFileInfo(f).fileName();
  m_fileContents.clear();
  QDir::setCurrent(QFileInfo(f).absolutePath());
  return m_mainFile;
}

QString FeWrap::eval()
{
  setlocale(LC_ALL, "C");
  m_evalStackBackup = fe_savegc(m_fe);
  // qDebug() << "stack" << m_evalStackBackup;

  const auto last_text = from_string(m_fe, _eval(m_fe, codeOf(m_mainFile)));

  fe_restoregc(m_fe, m_evalStackBackup);
  setlocale(LC_ALL, "");
  return last_text;
}

bool FeWrap::codeExists(const QString &f) const
{
  return m_fileContents.contains(f);
}

QString FeWrap::codeOf(const QString &f)
{
  if (!m_fileContents.contains(f))
  {
    QFile fi(f);
    if (!fi.open(QFile::ReadOnly))
      fe_error(m_fe, ("can't open file '" + f + "'").toLocal8Bit());
    m_fileContents[f] = QString(fi.readAll());
  }
  return m_fileContents.value(f);
}

void FeWrap::setCodeOf(const QString &f, const QString &c)
{
  m_fileContents[f] = QString(c);
  m_hasChanges = true;
}

int format_handle_line_break(const QString &fe, int &i)
{
  int nb_ln = 0;
  while (fe.size() > i && fe.at(i).isSpace())
  {
    if (fe.at(i) == '\n')
      ++nb_ln;
    ++i;
  }
  return nb_ln;
}

bool format_handle_line_break(QString &made, const QString &fe, int &i, int indent)
{
  if (0 < format_handle_line_break(fe, i))
  {
    made += '\n' + QString(indent * 2, QChar(' '));
    return true;
  }
  made.push_back(' ');
  return false;
}

void format_list(QString &made, const QString &fe, int &i, int indent, int d = 0)
{
  bool had_break = false;
  while (fe.size() > i)
  {
    if (fe.at(i) == ';')
    {
      made.push_back(';');
      ++i;
      if (fe.size() > i && !fe.at(i).isSpace())
        made.push_back(' ');
      while (fe.size() > i && fe.at(i) != '\n')
        made.push_back(fe.at(i++));
      if (format_handle_line_break(made, fe, i, d == 0 ? 0 : indent) && d > 0)
        had_break = true;
    }
    else if (fe.at(i) == '(')
    {
      made.push_back(fe.at(i));
      ++i;
      format_list(made, fe, i, had_break ? (indent + 1) : indent, d + 1);
      if (d == 0)
      {
        const auto c = std::max(1, std::min(2, format_handle_line_break(fe, i)));
        for (int j = 0; j < c; ++j)
          made += "\n";
      }
      else if (format_handle_line_break(made, fe, i, indent))
        had_break = true;
    }
    else if (fe.at(i) == ')')
    {
      auto xx = made.toStdString();
      while (made.back().isSpace())
        made.chop(1);
      made.push_back(fe.at(i));
      ++i;
      return;
    }
    else if (fe.at(i) == '"')
    {
      do
      {
        made.push_back(fe.at(i));
        ++i;
      } while (fe.size() > i && fe.at(i) != '"');
      if (fe.size() > i)
      {
        auto x = fe.at(i).toLatin1();
        made.push_back(fe.at(i));
        ++i;
      }
      if (format_handle_line_break(made, fe, i, indent))
        had_break = true;
    }
    else if (!fe.at(i).isSpace())
    {
      do
      {
        made.push_back(fe.at(i));
        ++i;
      } while (fe.size() > i && !fe.at(i).isSpace() && fe.at(i) != '(' && fe.at(i) != ')');
      if (format_handle_line_break(made, fe, i, indent))
        had_break = true;
    }
    else
      ++i;
  }
}

QString FeWrap::format(const QString &fe)
{
  QString made;
  int i = 0;
  format_list(made, fe, i, 1);
  if (made.size() > 2)
    while (made.back() == '\n' && made[made.size() - 2] == '\n')
      made.chop(1);
  return made;
}

static QString asDefinition(fe_Context *ctx, fe_Object *o)
{
  if (fe_type(ctx, o) == FE_TPAIR)
  {
    auto *s = fe_nextarg(ctx, &o);
    if (fe_type(ctx, s) == FE_TSYMBOL)
    {
      if (from_string(ctx, s) == "=")
      {
        s = fe_nextarg(ctx, &o);
        if (fe_type(ctx, s) == FE_TSYMBOL)
          return from_string(ctx, s);
      }
    }
  }

  return QString();
}

void FeWrap::eachDefinitionAtLine(const QString &fe, const LineDefinitionCB &cb)
{
  const auto fet = fe.toLocal8Bit();
  auto it = fet.begin();

  int gc = fe_savegc(m_fe);

  int line = 1;
  for (;;)
  {
    while (it < fet.end() && QChar(*it).isSpace())
    {
      if (*it == '\n')
        ++line;
      ++it;
    }
    auto oldIt = it;
    auto *r = fe_read(m_fe, read_fn, &it);
    if (!r)
      break;

    const auto def = asDefinition(m_fe, r);
    if (!def.isEmpty())
    {
      cb(line, def);
    }
    for (; oldIt < it; ++oldIt)
      if (*oldIt == '\n')
        line++;

    fe_restoregc(m_fe, gc);
  }
}

void FeWrap::saveFiles()
{
  for (auto it = m_fileContents.begin(); it != m_fileContents.end(); ++it)
  {
    QFileInfo(it.key()).absoluteDir().mkpath(".");
    QFile f(it.key());
    if (f.open(QFile::WriteOnly))
      f.write(it.value().toLocal8Bit());
  }
  m_hasChanges = false;
}

QStringList FeWrap::usedFiles() const
{
  return m_fileContents.keys();
}

fe_Object *FeWrap::_mod(fe_Context *ctx, fe_Object *arg)
{
  auto a = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
  while (!fe_isnil(ctx, arg))
  {
    const auto b = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
    a = a - std::floor(a / b) * b;
  }
  return fe_number(ctx, a);
}

fe_Object *FeWrap::_max(fe_Context *ctx, fe_Object *arg)
{
  auto a = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
  while (!fe_isnil(ctx, arg))
  {
    const auto b = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
    a = (a < b) ? b : a;
  }
  return fe_number(ctx, a);
}

fe_Object *FeWrap::_min(fe_Context *ctx, fe_Object *arg)
{
  auto a = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
  while (!fe_isnil(ctx, arg))
  {
    const auto b = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
    a = (a < b) ? a : b;
  }
  return fe_number(ctx, a);
}

fe_Object *FeWrap::_vec3(fe_Context *ctx, fe_Object *arg)
{
  auto x = s_number(ctx, fe_nextarg(ctx, &arg));
  if (fe_isnil(ctx, arg))
    return custom(ctx, s_vec3{x, x, x});

  auto y = s_number(ctx, fe_nextarg(ctx, &arg));
  auto z = s_number(ctx, fe_nextarg(ctx, &arg));

  return custom(ctx, s_vec3{x, y, z});
}

fe_Object *FeWrap::_color(fe_Context *ctx, fe_Object *arg)
{
  auto *arg1 = fe_nextarg(ctx, &arg);
  if (fe_type(ctx, arg1) == FE_TSTRING)
    return custom(ctx, QColor(from_string(ctx, arg1)));

  auto r = int(fe_tonumber(ctx, arg1));
  auto g = int(fe_tonumber(ctx, fe_nextarg(ctx, &arg)));
  auto b = int(fe_tonumber(ctx, fe_nextarg(ctx, &arg)));
  auto a = 255;
  if (!fe_isnil(ctx, arg))
    a = int(fe_tonumber(ctx, fe_nextarg(ctx, &arg)));
  return custom(ctx, QColor(r, g, b, a));
}

fe_Object *FeWrap::_cube(fe_Context *ctx, fe_Object *arg)
{
  auto c = std::make_unique<RenderDisplayObject>(RenderObject::primitives->cube());
  if (!fe_isnil(ctx, arg))
  {
    auto v = s_vec(ctx, fe_nextarg(ctx, &arg));
    c->set_scale(slm::vec3(*v[0], *v[1], *v[2]));
  }
  if (!fe_isnil(ctx, arg))
    c->setColor(get<QColor>(ctx, fe_nextarg(ctx, &arg)));

  return custom(ctx, std::move(c));
}

void FeWrap::add_all(RenderContainer &c, fe_Context *ctx, fe_Object **arg)
{
  while (!fe_isnil(ctx, *arg))
    c.add(_uobj(ctx, fe_nextarg(ctx, arg)));
}

fe_Object *FeWrap::_animation(fe_Context *ctx, fe_Object *arg)
{
  auto *sh = _scene(ctx);
  const auto name = from_string(ctx, fe_nextarg(ctx, &arg));
  const auto length = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
  auto v = s_vec(ctx, fe_nextarg(ctx, &arg));
  const auto lp = vec3(*v[0], *v[1], *v[2]);
  sh->add_animation(name, length, lp, _uobj(ctx, fe_nextarg(ctx, &arg)));

  return fe_bool(ctx, false);
}

fe_Object *FeWrap::_group(fe_Context *ctx, fe_Object *arg)
{
  auto c = std::make_unique<RenderContainer>();
  add_all(*c, ctx, &arg);
  return custom(ctx, std::move(c));
}

fe_Object *FeWrap::_helper(fe_Context *ctx, fe_Object *arg)
{
  auto c = std::make_unique<HelperContainer>();
  add_all(*c, ctx, &arg);
  return custom(ctx, std::move(c));
}

fe_Object *FeWrap::_translate(fe_Context *ctx, fe_Object *arg)
{
  auto c = std::make_unique<TranslateContainer>(s_vec(ctx, fe_nextarg(ctx, &arg)));
  add_all(*c, ctx, &arg);
  return custom(ctx, std::move(c));
}

fe_Object *FeWrap::_rotate(fe_Context *ctx, fe_Object *arg)
{
  auto a = s_number(ctx, fe_nextarg(ctx, &arg));
  auto t = s_vec(ctx, fe_nextarg(ctx, &arg));
  auto c = std::make_unique<RotateContainer>(a, t);

  add_all(*c, ctx, &arg);

  return custom(ctx, std::move(c));
}

fe_Object *FeWrap::_rotateX(fe_Context *ctx, fe_Object *arg)
{
  auto a = s_number(ctx, fe_nextarg(ctx, &arg));
  auto c = std::make_unique<RotateContainer>(a, vec3(1, 0, 0));

  add_all(*c, ctx, &arg);

  return custom(ctx, std::move(c));
}

fe_Object *FeWrap::_rotateY(fe_Context *ctx, fe_Object *arg)
{
  auto a = s_number(ctx, fe_nextarg(ctx, &arg));
  auto c = std::make_unique<RotateContainer>(a, vec3(0, 1, 0));

  add_all(*c, ctx, &arg);

  return custom(ctx, std::move(c));
}

fe_Object *FeWrap::_rotateZ(fe_Context *ctx, fe_Object *arg)
{
  auto a = s_number(ctx, fe_nextarg(ctx, &arg));
  auto c = std::make_unique<RotateContainer>(a, vec3(0, 0, 1));

  add_all(*c, ctx, &arg);

  return custom(ctx, std::move(c));
}

fe_Object *FeWrap::_scale(fe_Context *ctx, fe_Object *arg)
{
  auto *a1 = fe_nextarg(ctx, &arg);

  auto c = std::make_unique<ScaleContainer>();
  if (is<s_vec3>(ctx, a1))
    c->set_scale(get<s_vec3>(ctx, a1));
  else if (is<vec3>(ctx, a1))
    c->set_scale(get<vec3>(ctx, a1));
  else
    c->set_scale(vec3(fe_tonumber(ctx, a1)));

  add_all(*c, ctx, &arg);

  return custom(ctx, std::move(c));
}

template <typename T> fe_Object *_lfo_i(fe_Context *ctx, T center, T amp, float frequency)
{
  auto value = shared(center);

  _scene(ctx)->on_tick([value, center, amp, frequency](auto t) {
    *value = center + amp * float(sin(double(t) * M_PI * 2.0 * double(frequency)));
  });

  return custom(ctx, value);
}

fe_Object *FeWrap::_lfo(fe_Context *ctx, fe_Object *arg)
{
  auto *a1 = fe_nextarg(ctx, &arg);
  const auto center = fe_tonumber(ctx, a1);
  const auto amp = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
  const auto frequency = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
  return _lfo_i(ctx, center, amp, frequency);
}

fe_Object *FeWrap::_eval(fe_Context *ctx, const QString &fe)
{
  const auto fet = fe.toLocal8Bit();
  auto it = fet.begin();

  fe_Object *last{nullptr};
  int gc = fe_savegc(ctx);
  for (;;)
  {
    auto *r = fe_read(ctx, read_fn, &it);
    if (!r)
      break;
    last = fe_eval(ctx, r);

    fe_restoregc(ctx, gc);
    fe_pushgc(ctx, last);
  }
  return last;
}

fe_Object *FeWrap::_require(fe_Context *ctx, fe_Object *arg)
{
  fe_Object *last{nullptr};
  int gc = fe_savegc(ctx);
  while (!fe_isnil(ctx, arg))
  {
    const auto f = from_string(ctx, fe_nextarg(ctx, &arg)).replace(".", QDir::separator()).append(".fe");
    last = _eval(ctx, _self(ctx)->codeOf(f));

    fe_restoregc(ctx, gc);
    fe_pushgc(ctx, last);
  }
  return last;
}

[[noreturn]] void FeWrap::on_error(fe_Context *ctx, const char *err, fe_Object *cl)
{
  auto x = QString(err);
  while (!fe_isnil(ctx, cl))
    x += "\n=> " + from_string(ctx, fe_nextarg(ctx, &cl));

  fe_restoregc(ctx, _self(ctx)->m_evalStackBackup);
  throw std::runtime_error(x.toLocal8Bit());
}

static fe_Object *on_gc(fe_Context *ctx, fe_Object *o)
{
  delete reinterpret_cast<CustomPtr *>(fe_toptr(ctx, o));
  return nullptr;
}

void FeWrap::init_fn(fe_Context *ctx)
{
  auto *h = fe_handlers(ctx);
  h->error = on_error;
  h->gc = on_gc;

  fe_set(ctx, fe_symbol(ctx, "%"), fe_cfunc(ctx, _mod));
  fe_set(ctx, fe_symbol(ctx, "max"), fe_cfunc(ctx, _max));
  fe_set(ctx, fe_symbol(ctx, "min"), fe_cfunc(ctx, _min));
  fe_set(ctx, fe_symbol(ctx, "pi"), fe_number(ctx, M_PI));
  fe_set(ctx, fe_symbol(ctx, "rad"), fe_cfunc(ctx, [](fe_Context *ctx, fe_Object *arg) {
           const auto n = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
           return fe_number(ctx, M_PI * (n / 180.0));
         }));
  fe_set(ctx, fe_symbol(ctx, "deg"), fe_cfunc(ctx, [](fe_Context *ctx, fe_Object *arg) {
           const auto n = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
           return fe_number(ctx, 180.0 * (n / M_PI));
         }));
  fe_set(ctx, fe_symbol(ctx, "fsin"), fe_cfunc(ctx, [](fe_Context *ctx, fe_Object *arg) {
           const auto n = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
           auto f = 1.0;
           if (!fe_isnil(ctx, arg))
             f = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
           return fe_number(ctx, sin(f * 2.0 * n * M_PI));
         }));
  fe_set(ctx, fe_symbol(ctx, "fcos"), fe_cfunc(ctx, [](fe_Context *ctx, fe_Object *arg) {
           const auto n = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
           auto f = 1.0;
           if (!fe_isnil(ctx, arg))
             f = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
           return fe_number(ctx, cos(f * 2.0 * n * M_PI));
         }));

#define _func(f)                                                                                                       \
  fe_set(ctx, fe_symbol(ctx, #f), fe_cfunc(ctx, [](fe_Context *ctx, fe_Object *arg) {                                  \
           const auto n = fe_tonumber(ctx, fe_nextarg(ctx, &arg));                                                     \
           return fe_number(ctx, f(n));                                                                                \
         }))
  _func(floor);
  _func(ceil);
  _func(abs);
  _func(sqrt);
  _func(sin);
  _func(cos);
  _func(tan);
  _func(acos);
  _func(atan);
  _func(atan);

  fe_set(ctx, fe_symbol(ctx, "self"), custom(ctx, this));

  fe_set(ctx, fe_symbol(ctx, "vec3"), fe_cfunc(ctx, _vec3));
  fe_set(ctx, fe_symbol(ctx, "color"), fe_cfunc(ctx, _color));

  fe_set(ctx, fe_symbol(ctx, "cube"), fe_cfunc(ctx, _cube));
  fe_set(ctx, fe_symbol(ctx, "group"), fe_cfunc(ctx, _group));

  fe_set(ctx, fe_symbol(ctx, "helper"), fe_cfunc(ctx, _helper));

  fe_set(ctx, fe_symbol(ctx, "lfo"), fe_cfunc(ctx, _lfo));

  fe_set(ctx, fe_symbol(ctx, "translate"), fe_cfunc(ctx, _translate));
  fe_set(ctx, fe_symbol(ctx, "rotate"), fe_cfunc(ctx, _rotate));
  fe_set(ctx, fe_symbol(ctx, "rotateX"), fe_cfunc(ctx, _rotateX));
  fe_set(ctx, fe_symbol(ctx, "rotateY"), fe_cfunc(ctx, _rotateY));
  fe_set(ctx, fe_symbol(ctx, "rotateZ"), fe_cfunc(ctx, _rotateZ));
  fe_set(ctx, fe_symbol(ctx, "scale"), fe_cfunc(ctx, _scale));

  fe_set(ctx, fe_symbol(ctx, "animation"), fe_cfunc(ctx, _animation));

  fe_set(ctx, fe_symbol(ctx, "require"), fe_cfunc(ctx, _require));
}
