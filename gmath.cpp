#include "gmath.h"
using namespace GMath;

template<typename T,int n> const GVect<T,n> GVect<T,n>::zero;
template<typename T,int n> const GVect<T,n> GVect<T,n>::one;

template<> const vec2  vec2::zero  = {0,0};
template<> const vec3  vec3::zero  = {0,0,0};
template<> const vec4  vec4::zero  = {0,0,0,0};

template<> const vec3f vec3f::one  = {1.0f,1.0f,1.0f};
template<> const vec3f vec3f::zero = {0.0f,0.0f,0.0f};

template<> const vec2i vec2i::one = {1,1};
template<> const vec2i vec2i::zero = {0,0};
