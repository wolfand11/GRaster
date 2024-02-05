#ifndef GMATH_H
#define GMATH_H

#include <cassert>
#include <cmath>
#include <iostream>
#include <string>

#define _USE_MATH_DEFINES
#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif
#ifndef M_PIl
#define M_PIl (3.14159265358979323846264338327950288)
#endif

namespace GMath
{
template <typename T, int n>
struct GVect
{
    GVect() = default;
    GVect(T x){ data[0]=x; }
    GVect(T x, T y){ data[0]=x; data[1]=y;}
    GVect(T x, T y, T z){ data[0]=x; data[1]=y; data[2]=z; }
    GVect(T x, T y, T z, T w){ data[0]=x; data[1]=y; data[2]=z; data[3]=w; }

    T& operator[](const int i)
    {
        assert(i>=0 && i<n);
        return data[i];
    }
    T operator[](const int i) const
    {
        assert(i>=0 && i<n);
        return data[i];
    }

    template<typename T1> operator GVect<T1,n>() const
    {
        GVect<T1,n> ret;
        for(int i=0; i<n; i++)
        {
            ret[i] = (T1)((*this)[i]);
        }
        return ret;
    }

    T length2() const { return dot(*this,*this); }
    T length() const { return std::sqrt(dot(*this,*this)); }
    GVect& normalize() { *this = (*this)/length(); return *this; }
    GVect& inverse()
    {
        for(int i=0; i<n; i++)
        {
            data[i] = -data[i];
        }
        return *this;
    }

    static const GVect<T,n> zero;
    static const GVect<T,n> one;

    T x() const { return data[0]; }
    T& x() { return data[0]; }
    void SetX(T v){ data[0]=v; }

    T y() const { return data[1]; }
    T& y() { return data[1]; }
    void SetY(T v){ data[1]=v; }

    T z() const { return data[2]; }
    T& z() { return data[2]; }
    void SetZ(T v){ data[2]=v; }

    T w() const { return data[3]; }
    T& w() { return data[3]; }
    void SetW(T v){ data[3]=v; }

    GVect<T, 3> xyz() const
    {
        GVect<T, 3> ret(data[0], data[1], data[2]);
        return ret;
    }
    void SetXYZ(T x, T y, T z)
    {
        data[0] = x;
        data[1] = y;
        data[2] = z;
    }
    void SetXYZ(GVect<T, 3> xyz)
    {
        data[0] = xyz[0];
        data[1] = xyz[1];
        data[2] = xyz[2];
    }

    T data[n] = {};
};

template <typename T, int n>
GVect<T,n> operator+(const GVect<T,n>& lhs, const GVect<T,n>& rhs)
{
    GVect<T,n> ret = lhs;
    for(int i=0; i<n; i++)
    {
        ret[i] += rhs[i];
    }
    return ret;
}

template <typename T, int n>
GVect<T,n> operator-(const GVect<T,n>& lhs, const GVect<T,n>& rhs)
{
    GVect<T,n> ret = lhs;
    for(int i=0; i<n; i++)
    {
        ret[i] -= rhs[i];
    }
    return ret;
}

template <typename T, typename ST, int n>
GVect<T,n> operator*(const ST& lhs, const GVect<T,n>& rhs)
{
    GVect<T,n> ret = rhs;
    for(int i=0; i<n; i++)
    {
        ret[i] *= lhs;
    }
    return ret;
}

template <typename T, typename ST, int n>
GVect<T,n> operator*(const GVect<T,n>& lhs, const ST& rhs)
{
    GVect<T,n> ret = lhs;
    for(int i=0; i<n; i++)
    {
        ret[i] *= rhs;
    }
    return ret;
}

template <typename T, typename ST, int n>
GVect<T,n> operator/(const GVect<T,n>& lhs, const ST& rhs)
{
    GVect<T,n> ret = lhs;
    for(int i=0; i<n; i++)
    {
        ret[i] /= rhs;
    }
    return ret;
}

template <typename T, int n>
GVect<T,n> operator*(const GVect<T,n>& lhs, const GVect<T,n>& rhs)
{
    GVect<T,n> ret = lhs;
    for(int i=0; i<n; i++)
    {
        ret[i] *= rhs[i];
    }
    return ret;
}

template <typename T, int n>
GVect<T,n> operator/(const GVect<T,n>& lhs, const GVect<T,n>& rhs)
{
    GVect<T,n> ret = lhs;
    for(int i=0; i<n; i++)
    {
        ret[i] /= rhs[i];
    }
    return ret;
}


template <typename T, int n>
T dot(const GVect<T,n>& lhs, const GVect<T,n>& rhs)
{
    T ret = 0;
    for(int i=0; i<n; i++)
    {
        ret += lhs[i]*rhs[i];
    }
    return ret;
}

template <typename T, int n, int m>
GVect<T,n> embed(const GVect<T,m>& v, const T fill)
{
    assert(n>=m);
    GVect<T,n> ret;
    for(int i=0; i<n; i++)
    {
        ret[i] = i<m ? v[i] : fill;
    }
    return ret;
}

template <typename T, int n, int m>
GVect<T,n> proj(const GVect<T,m>& v)
{
    assert(n<=m);
    GVect<T,n> ret;
    for(int i=0; i<n; i++)
    {
        ret[i] = v[i];
    }
    return ret;
}

template <typename T, int n>
GVect<T,n> lerp(const GVect<T,n>& v0, const GVect<T,n>& v1, float factor)
{
    GVect<T,n> ret;
    for(int i=0; i<n; i++)
    {
        ret[i] = v0[i]*(1-factor) + v1[i]*factor;
    }
    return ret;
}

template <typename T, int n>
std::ostream& operator <<(std::ostream& out, const GVect<T,n>& v)
{
    for(int i=0; i<n; i++) out << std::to_string(v[i]) << " ";
    return out;
}

typedef GVect<double, 2> vec2;
typedef GVect<double, 3> vec3;
typedef GVect<double, 4> vec4;
typedef GVect<float, 2>  vec2f;
typedef GVect<float, 3>  vec3f;
typedef GVect<float, 4>  vec4f;
typedef GVect<int, 2>  vec2i;
typedef GVect<int, 3>  vec3i;
typedef GVect<int, 4>  vec4i;

template<typename T>
GVect<T,3> cross(const GVect<T,3>& v1, const GVect<T,3>& v2)
{
    GVect<T,3> ret = GVect<T,3>{v1.y()*v2.z()-v1.z()*v2.y(), v1.z()*v2.x()-v1.x()*v2.z(), v1.x()*v2.y()-v1.y()*v2.x()};
    return ret;
}

template<typename T, int nrows, int ncols> struct GMatrix;

template<typename T, int n>
static double det(GMatrix<T,n,n>& mat)
{
    double ret = 0;
    for(int i=0; i<n; i++)
    {
        ret += mat[0][i] * mat.cofactor(0, i);
    }
    return ret;
}

template<typename T>
static double det(GMatrix<T,1,1>& mat)
{
    double ret = mat[0][0];
    return ret;
}

template<typename T, int nrows, int ncols>
struct GMatrix
{
    GVect<T, ncols> rows[nrows] = {{}};
    GMatrix() = default;
    GVect<T, ncols>& operator[](const int idx)            {assert(idx>=0&&idx<nrows); return rows[idx];}
    const GVect<T, ncols>& operator[](const int idx)const {assert(idx>=0&&idx<nrows); return rows[idx];}
    template<typename T1> operator GMatrix<T1,nrows,ncols>()
    {
        GMatrix<T1,nrows,ncols> ret;
        for(int i=0; i<nrows; i++)
        {
            for(int j=0; j<ncols; j++)
            {
                ret[i][j] = rows[i][j];
            }
        }
        return ret;
    }

    GVect<T, nrows> col(const int idx) const
    {
        assert(idx>=0 && idx<ncols);
        GVect<T, nrows> ret;
        for(int i=0; i<nrows; i++) { ret[i] = rows[i][idx]; }
        return ret;
    }

    void set_col(const int idx, const GVect<T, nrows>& v)
    {
        assert(idx>=0 && idx<ncols);
        for(int i=0; i<nrows; i++) { rows[i][idx] = v[i]; }
    }

    GMatrix<T, nrows, ncols>& identity()
    {
        for(int i=0; i<nrows; i++)
        {
            for(int j=0; j<ncols; j++)
            {
                rows[i][j] = (i==j);
            }
        }
        return *this;
    }

    GMatrix<T, nrows, ncols>& zero()
    {
        for(int i=0; i<nrows; i++)
        {
            for(int j=0; j<ncols; j++)
            {
                rows[i][j] = 0;
            }
        }
        return *this;
    }

    double det()
    {
        return GMath::det(*this);
    }
    GMatrix<T,nrows-1, ncols-1> get_minor(const int row, const int col) const
    {
        assert(nrows==ncols && nrows>1);

        GMatrix<T,nrows-1,ncols-1> ret;
        for(int i=0; i<nrows-1; i++)
        {
            for(int j=0; j<ncols-1; j++)
            {
                ret[i][j] = rows[i<row?i:i+1][j<col?j:j+1];
            }
        }
        return ret;
    }
    double cofactor(const int row, const int col) const
    {
        return get_minor(row,col).det()*((row+col)%2 ? -1 : 1);
    }
    GMatrix<T,nrows, ncols> adjugate() const
    {
        GMatrix<T,nrows,ncols> ret;
        for(int i=0; i<nrows; i++)
        {
            for(int j=0; j<ncols; j++)
            {
                // non transpose
                ret[i][j] = cofactor(i,j);
            }
        }
        return ret;
    }
    GMatrix<T,nrows,ncols> invert_transpose() const
    {
        GMatrix<T,nrows,ncols> ret = adjugate();
        return ret / dot(ret[0],rows[0]);
    }
    GMatrix<T,nrows,ncols> invert() const
    {
        return this->invert_transpose().transpose();
    }
    GMatrix<T,ncols,nrows> transpose() const
    {
        GMatrix<T,ncols,nrows> ret;
        for(int i=0; i<ncols; i++)
        {
            ret.rows[i] = this->col(i);
        }
        return ret;
    }
};
template <typename T, int nrows,int ncols>
GVect<T,nrows> operator*(const GMatrix<T,nrows,ncols>& lhs, const GVect<T,ncols>& rhs)
{
    GVect<T,nrows> ret;
    for(int i=0; i<nrows; i++)
    {
        ret[i] = dot(lhs[i],rhs);
    }
    return ret;
}
template<typename T, int R1, int C1, int C2>
GMatrix<T,R1,C2> operator*(const GMatrix<T,R1,C1>& lhs, const GMatrix<T,C1,C2>& rhs)
{
    GMatrix<T,R1,C2> ret;
    for(int i=0; i<R1; i++)
    {
        for(int j=0; j<C2; j++)
        {
            ret[i][j] = dot(lhs[i],rhs.col(j));
        }
    }
    return ret;
}
template <typename T, int nrows,int ncols>
GMatrix<T,nrows,ncols> operator*(const GMatrix<T,nrows,ncols>& lhs, const double rhs)
{
    GMatrix<T,nrows,ncols> ret;
    for(int i=0; i<nrows; i++)
    {
        ret[i] = lhs[i]*rhs;
    }
    return ret;
}
template <typename T, int nrows,int ncols>
GMatrix<T,nrows,ncols> operator/(const GMatrix<T,nrows,ncols>& lhs, const double rhs)
{
    assert(rhs!=0.0);
    GMatrix<T,nrows,ncols> ret;
    for(int i=0; i<nrows; i++)
    {
        ret[i] = lhs[i]/rhs;
    }
    return ret;
}
template <typename T, int nrows,int ncols>
GMatrix<T,nrows,ncols> operator+(const GMatrix<T,nrows,ncols>& lhs, const GMatrix<T,nrows,ncols>& rhs)
{
    assert(rhs!=0.0);
    GMatrix<T,nrows,ncols> ret;
    for(int i=0; i<nrows; i++)
    {
        ret[i] = lhs[i] + rhs[i];
    }
    return ret;
}
template <typename T, int nrows,int ncols>
GMatrix<T,nrows,ncols> operator-(const GMatrix<T,nrows,ncols>& lhs, const GMatrix<T,nrows,ncols>& rhs)
{
    assert(rhs!=0.0);
    GMatrix<T,nrows,ncols> ret;
    for(int i=0; i<nrows; i++)
    {
        ret[i] = lhs[i] - rhs[i];
    }
    return ret;
}
template<typename T,int nrows,int ncols>
std::ostream& operator<<(std::ostream& out, const GMatrix<T,nrows,ncols>& m)
{
    for (int i=0; i<nrows; i++) out << m[i] << std::endl;
    return out;
}
typedef GMatrix<float,3,3>  mat3f;
typedef GMatrix<double,3,3> mat3;
typedef GMatrix<double,2,3> mat23;
typedef GMatrix<float,4,4>  mat4f;
typedef GMatrix<double,4,4> mat4;
}

#endif // GMATH_H
