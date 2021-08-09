#include "fewrap.h"

#include <QDebug>

#include "renderobject.h"

extern "C" {
#include "fe/fe.h"
}

struct CustomPtr
{
  union
  {
    RenderObject *obj;
    slm::vec3 vec;
    QColor col;
  } d{nullptr};

  enum Type
  {
    Object,
    Vec3,
    Color
  };
  Type type{Object};

  bool moved{false};
};

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
  QString t;
  fe_write(ctx, o, write_fn, &t, false);
  return t;
}

fe_Object *custom(fe_Context *ctx, const slm::vec3 &o)
{
  auto *wrap = new CustomPtr;
  wrap->type = CustomPtr::Vec3;
  wrap->d.vec = o;
  return fe_ptr(ctx, wrap);
}

fe_Object *custom(fe_Context *ctx, const QColor &o)
{
  auto *wrap = new CustomPtr{};
  wrap->type = CustomPtr::Color;
  wrap->d.col = o;
  return fe_ptr(ctx, wrap);
}

fe_Object *custom(fe_Context *ctx, RenderObject *o)
{
  auto *wrap = new CustomPtr{};
  wrap->type = CustomPtr::Object;
  wrap->d.obj = o;
  return fe_ptr(ctx, wrap);
}

template <typename T, typename... Args>
fe_Object *create_custom(fe_Context *ctx, Args &&...args)
{
  return custom(ctx, new T(std::forward<Args>(args)...));
}

template <CustomPtr::Type t>
static bool is(fe_Context *ctx, fe_Object *o)
{
  if (fe_type(ctx, o) == FE_TPTR)
    if (auto *p = reinterpret_cast<CustomPtr *>(fe_toptr(ctx, o)))
      return p->type == t;
  return false;
}

static slm::vec3 _vec3(fe_Context *ctx, fe_Object *o)
{
  if (!is<CustomPtr::Vec3>(ctx, o))
    fe_error(ctx, "Expect vec3!");

  return reinterpret_cast<CustomPtr *>(fe_toptr(ctx, o))->d.vec;
}

static QColor _color(fe_Context *ctx, fe_Object *o)
{
  if (!is<CustomPtr::Color>(ctx, o))
    fe_error(ctx, "Expect vec3 object!");

  return reinterpret_cast<CustomPtr *>(fe_toptr(ctx, o))->d.col;
}

static RenderObject *_obj(fe_Context *ctx, fe_Object *o)
{
  if (!is<CustomPtr::Object>(ctx, o))
    fe_error(ctx, "Expect RenderObject!");

  return reinterpret_cast<CustomPtr *>(fe_toptr(ctx, o))->d.obj;
}

static std::unique_ptr<RenderObject> _uobj(fe_Context *ctx, fe_Object *o)
{
  if (!is<CustomPtr::Object>(ctx, o))
    fe_error(ctx, "Expect RenderObject!");

  auto *custom = reinterpret_cast<CustomPtr *>(fe_toptr(ctx, o));
  if (custom->moved)
    fe_error(ctx, "Working with deleted data!");

  custom->moved = true;
  return std::unique_ptr<RenderObject>(custom->d.obj);
}

FeWrap::FeWrap() : m_data{malloc(m_size)}, m_fe{fe_open(m_data, m_size)}
{
  init_fn(m_fe);
}

FeWrap::~FeWrap()
{
  fe_close(m_fe);
  free(m_data);
}

void FeWrap::load(const QString &f)
{
  FILE *fp = fopen(f.toLocal8Bit(), "rb");
  int gc = fe_savegc(m_fe);

  for (;;)
  {
    fe_Object *obj = fe_readfp(m_fe, fp);
    if (!obj)
      break;

    fe_eval(m_fe, obj);
    fe_restoregc(m_fe, gc);
  }

  fclose(fp);
}

std::unique_ptr<RenderObject> FeWrap::eval(const QString &fe, const std::function<void(const QString &)> &out)
{
  const auto fet = fe.toLocal8Bit();
  auto it = fet.begin();

  int gc = fe_savegc(m_fe);

  auto group = std::make_unique<RenderContainer>();
  for (;;)
  {
    auto *r = fe_read(m_fe, read_fn, &it);
    if (!r)
      break;

    auto *o = fe_eval(m_fe, r);
    if (out)
      out(from_string(m_fe, o));
    if (is<CustomPtr::Object>(m_fe, o))
      if (auto rp = _uobj(m_fe, o))
        group->add(std::move(rp));

    fe_restoregc(m_fe, gc);
  }

  return std::move(group);
}

static bool format_need_break(fe_Context *ctx, fe_Object *o)
{
  if (fe_type(ctx, o) == FE_TPAIR)
  {
    auto *a = fe_nextarg(ctx, &o);
    if (fe_type(ctx, a) == FE_TSYMBOL)
    {
      char buffer[256] = {0};
      fe_tostring(ctx, a, buffer, 255);
      for (const auto *key : {"vec3", "color", "fn", "mac", "=", "+", "-", "*", "/"})
        if (strcmp(buffer, key) == 0)
          return false;
    }
    return true;
  }
  return false;
}

static int format_force_no_break(fe_Context *ctx, fe_Object *o)
{
  if (fe_type(ctx, o) == FE_TPAIR)
  {
    auto *a = fe_nextarg(ctx, &o);
    if (fe_type(ctx, a) == FE_TSYMBOL)
    {
      char buffer[256] = {0};
      fe_tostring(ctx, a, buffer, 255);
      for (const auto *key : {"fn", "=", "mac"})
        if (strcmp(buffer, key) == 0)
          return 2;
    }
  }
  return 0;
}

static void format__(fe_Context *ctx, fe_Object *o, QString &out, bool force_no_break = false, QString ind = "  ")
{
  if (!out.isEmpty() && out.back() != '(' && out.back() != '\n')
    out += " ";
  if (fe_type(ctx, o) != FE_TPAIR)
  {
    out += from_string(ctx, o);
    return;
  }

  if (!force_no_break && !out.isEmpty() && out.back() != '\n' && format_need_break(ctx, o))
  {
    out += "\n" + ind;
    ind += "  ";
  }
  out += "(";
  int no_break = format_force_no_break(ctx, o);
  while (!fe_isnil(ctx, o))
  {
    format__(ctx, fe_nextarg(ctx, &o), out, no_break > 0, ind);
    no_break--;
  }
  out += ")";
}

QString FeWrap::format(const QString &fe)
{
  const auto fet = fe.toLocal8Bit();
  auto it = fet.begin();

  int gc = fe_savegc(m_fe);

  QString made;
  QString br;

  for (;;)
  {
    auto *r = fe_read(m_fe, read_fn, &it);
    if (!r)
      break;

    made += br;
    format__(m_fe, r, made);
    br = "\n\n";

    fe_restoregc(m_fe, gc);
  }
  return made;
}

fe_Object *FeWrap::vec3(fe_Context *ctx, fe_Object *arg)
{
  auto x = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
  if (fe_isnil(ctx, arg))
    return custom(ctx, slm::vec3(x));

  auto y = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
  auto z = fe_tonumber(ctx, fe_nextarg(ctx, &arg));

  return custom(ctx, slm::vec3(x, y, z));
}

fe_Object *FeWrap::color(fe_Context *ctx, fe_Object *arg)
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

fe_Object *FeWrap::cube(fe_Context *ctx, fe_Object *arg)
{
  auto c = std::make_unique<RenderDisplayObject>(RenderObject::primitives->cube());
  if (!fe_isnil(ctx, arg))
    c->set_scale(_vec3(ctx, fe_nextarg(ctx, &arg)));
  if (!fe_isnil(ctx, arg))
    c->setColor(_color(ctx, fe_nextarg(ctx, &arg)));

  return custom(ctx, c.release());
}

void FeWrap::add_all(RenderContainer &c, fe_Context *ctx, fe_Object **arg)
{
  while (!fe_isnil(ctx, *arg))
    c.add(_uobj(ctx, fe_nextarg(ctx, arg)));
}

fe_Object *FeWrap::group(fe_Context *ctx, fe_Object *arg)
{
  auto c = std::make_unique<RenderContainer>();
  add_all(*c, ctx, &arg);
  return custom(ctx, c.release());
}

fe_Object *FeWrap::translate(fe_Context *ctx, fe_Object *arg)
{
  auto c = std::make_unique<TranslateContainer>(_vec3(ctx, fe_nextarg(ctx, &arg)));

  add_all(*c, ctx, &arg);

  return custom(ctx, c.release());
}

fe_Object *FeWrap::rotate(fe_Context *ctx, fe_Object *arg)
{
  auto a = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
  auto t = _vec3(ctx, fe_nextarg(ctx, &arg));
  auto c = std::make_unique<RotateContainer>(a, t);

  add_all(*c, ctx, &arg);

  return custom(ctx, c.release());
}

fe_Object *FeWrap::rotateX(fe_Context *ctx, fe_Object *arg)
{
  auto a = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
  auto c = std::make_unique<RotateContainer>(a, slm::vec3(1, 0, 0));

  add_all(*c, ctx, &arg);

  return custom(ctx, c.release());
}

fe_Object *FeWrap::rotateY(fe_Context *ctx, fe_Object *arg)
{
  auto a = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
  auto c = std::make_unique<RotateContainer>(a, slm::vec3(0, 1, 0));

  add_all(*c, ctx, &arg);

  return custom(ctx, c.release());
}

fe_Object *FeWrap::rotateZ(fe_Context *ctx, fe_Object *arg)
{
  auto a = fe_tonumber(ctx, fe_nextarg(ctx, &arg));
  auto c = std::make_unique<RotateContainer>(a, slm::vec3(0, 0, 1));

  add_all(*c, ctx, &arg);

  return custom(ctx, c.release());
}

fe_Object *FeWrap::scale(fe_Context *ctx, fe_Object *arg)
{
  auto *a1 = fe_nextarg(ctx, &arg);
  auto c = std::make_unique<ScaleContainer>((fe_type(ctx, a1) == FE_TNUMBER) ? slm::vec3(fe_tonumber(ctx, a1))
                                                                             : _vec3(ctx, a1));

  add_all(*c, ctx, &arg);

  return custom(ctx, c.release());
}

[[noreturn]] static void on_error(fe_Context *ctx, const char *err, fe_Object *cl)
{
  auto x = QString(err);
  while (!fe_isnil(ctx, cl))
    x += "\n=> " + from_string(ctx, fe_nextarg(ctx, &cl));
  throw std::runtime_error(x.toLocal8Bit());
}

static fe_Object *on_gc(fe_Context *ctx, fe_Object *o)
{
  auto *custom = reinterpret_cast<CustomPtr *>(fe_toptr(ctx, o));
  if (!custom->moved && custom->type == CustomPtr::Object)
    delete custom->d.obj;
  delete custom;
  return nullptr;
}

void FeWrap::init_fn(fe_Context *ctx)
{
  auto *h = fe_handlers(ctx);
  h->error = on_error;
  h->gc = on_gc;

  fe_set(ctx, fe_symbol(ctx, "vec3"), fe_cfunc(ctx, vec3));
  fe_set(ctx, fe_symbol(ctx, "color"), fe_cfunc(ctx, color));

  fe_set(ctx, fe_symbol(ctx, "cube"), fe_cfunc(ctx, cube));
  fe_set(ctx, fe_symbol(ctx, "group"), fe_cfunc(ctx, group));

  fe_set(ctx, fe_symbol(ctx, "translate"), fe_cfunc(ctx, translate));
  fe_set(ctx, fe_symbol(ctx, "rotate"), fe_cfunc(ctx, rotate));
  fe_set(ctx, fe_symbol(ctx, "rotateX"), fe_cfunc(ctx, rotateX));
  fe_set(ctx, fe_symbol(ctx, "rotateY"), fe_cfunc(ctx, rotateY));
  fe_set(ctx, fe_symbol(ctx, "rotateZ"), fe_cfunc(ctx, rotateZ));
  fe_set(ctx, fe_symbol(ctx, "scale"), fe_cfunc(ctx, scale));
}
