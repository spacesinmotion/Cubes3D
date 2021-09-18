#pragma once

#include "slm/vec3.h"

#include <array>
#include <functional>
#include <memory>

using s_float = std::shared_ptr<float>;
using s_float_deleter = std::function<void(float *)>;
inline s_float shared(float f, const s_float_deleter &d = std::default_delete<float>{})
{
  return std::shared_ptr<float>(new float(f), d);
}

using s_vec3 = std::array<s_float, 3>;
inline s_vec3 shared(const slm::vec3 &f)
{
  return std::array<s_float, 3>{shared(f.x), shared(f.y), shared(f.z)};
}
