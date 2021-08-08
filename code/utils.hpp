#ifndef __UTILS_H__
#define __UTILS_H__

#include <cstdio>
#include <math.h>
#include <cstdlib>

const double PI = acos(-1);

double clamp(double x)
{
    if (x > 1.0)
        return 1;
    else if (x < 0.0)
        return 0.0;
    else
        return x;
}


double rand_double(double minf=0.0, double maxf=1.0) {
    double a = 0.0;
    do{
        a = drand48();
    } while (a > maxf || a < minf);
    return a;
}


struct Vec3
{
    double x, y, z;
    Vec3(double x_ = 0, double y_ = 0, double z_ = 0) : x(x_), y(y_), z(z_) {}
    Vec3 operator+(const Vec3 &b) const { return Vec3(x + b.x, y + b.y, z + b.z); }
    Vec3 operator-(const Vec3 &b) const { return Vec3(x - b.x, y - b.y, z - b.z); }
    Vec3 operator*(double b) const { return Vec3(x * b, y * b, z * b); }
    Vec3 operator/(double b) const { return Vec3(x / b, y / b, z / b); }
    Vec3 operator+=(const Vec3 &b) { return *this = *this + b; }
    Vec3 operator-=(const Vec3 &b) { return *this = *this - b; }
    Vec3 operator*=(double b) { return *this = *this * b; }
    Vec3 operator/=(double b) { return *this = *this / b; }

    Vec3 mult(const Vec3 &b) const { return Vec3(x * b.x, y * b.y, z * b.z); }
    void normalize() { *this = *this * (1 / sqrt(x * x + y * y + z * z)); }
    Vec3 normalized() const { return *this * (1 / sqrt(x * x + y * y + z * z)); }
    bool operator==(const Vec3 &b) { return b.x == x && b.y == y && b.z == z; }
    double len() const { return sqrt(x * x + y * y + z * z); }
    double len2() const { return x * x + y * y + z * z; }
    double& operator [] ( int i ) {
        if (i == 0) return x;
        if (i == 1) return y;
        else return z;
    }
    double dot(const Vec3 &b) const { return x * b.x + y * b.y + z * b.z; } // cross:
    Vec3 operator%(const Vec3 &b) { return Vec3(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x); }
    Vec3 clip() const { return Vec3(clamp(x), clamp(y), clamp(z)); }
    Vec3 reflect(const Vec3 &n) { return (*this) - n * 2 * n.dot(*this); }
};

Vec3 random_in_unit_sphere()
{
    Vec3 res;
    do
    {
        res = Vec3(drand48(), drand48(), drand48()) * 2 - Vec3(1, 1, 1);
    } while (res.len2() >= 1);
    return res;
}


class Quat4f
{
public:

	static const Quat4f ZERO;
	static const Quat4f IDENTITY;

	Quat4f() = default;
	// sets this quaternion to be a rotation of fRadians about v = < fx, fy, fz >, v need not necessarily be unit length
	void setAxisAngle( float radians, const Vec3& axis );
    Quat4f normalized() const;
    void normalize();
    float w() const;
	float x() const;
	float y() const;
	float z() const;

private:

	float m_elements[ 4 ];

};

float Quat4f::w() const
{
	return m_elements[ 0 ];
}

float Quat4f::x() const
{
	return m_elements[ 1 ];
}

float Quat4f::y() const
{
	return m_elements[ 2 ];
}

float Quat4f::z() const
{
	return m_elements[ 3 ];
}

Quat4f Quat4f::normalized() const
{
	Quat4f q( *this );
	q.normalize();
	return q;
}

void Quat4f::normalize()
{
    float abs =	sqrt(
		m_elements[ 0 ] * m_elements[ 0 ] +
		m_elements[ 1 ] * m_elements[ 1 ] +
		m_elements[ 2 ] * m_elements[ 2 ] +
		m_elements[ 3 ] * m_elements[ 3 ]
	);
	float reciprocalAbs = 1.f / abs;

	m_elements[ 0 ] *= reciprocalAbs;
	m_elements[ 1 ] *= reciprocalAbs;
	m_elements[ 2 ] *= reciprocalAbs;
	m_elements[ 3 ] *= reciprocalAbs;
}


void Quat4f::setAxisAngle( float radians, const Vec3& axis )
{
	m_elements[ 0 ] = cos( radians / 2 );

	float sinHalfTheta = sin( radians / 2 );
	float vectorNorm = axis.len();
	float reciprocalVectorNorm = 1.f / vectorNorm;

	m_elements[ 1 ] = axis.x * sinHalfTheta * reciprocalVectorNorm;
	m_elements[ 2 ] = axis.y * sinHalfTheta * reciprocalVectorNorm;
	m_elements[ 3 ] = axis.z * sinHalfTheta * reciprocalVectorNorm;
}



class Matrix3f
{
public:

    // Fill a 3x3 matrix with "fill", default to 0.
	Matrix3f( float fill = 0.f );
	static Matrix3f rotation( const Quat4f& rq );
    Matrix3f( float m00, float m01, float m02,
		float m10, float m11, float m12,
		float m20, float m21, float m22 );
    const float& operator () ( int i, int j ) const;

private:

	float m_elements[ 9 ];

};

const float& Matrix3f::operator () ( int i, int j ) const
{
	return m_elements[ j * 3 + i ];
}

Vec3 operator * ( const Matrix3f& m, Vec3 v )
{
	Vec3 output( 0, 0, 0 );

	for( int i = 0; i < 3; ++i )
	{
		for( int j = 0; j < 3; ++j )
		{
			output[ i ] += m( i, j ) * v[ j ];
		}
	}

	return output;
}

Matrix3f::Matrix3f( float fill )
{
	for( int i = 0; i < 9; ++i )
	{
		m_elements[ i ] = fill;
	}
}

Matrix3f::Matrix3f( float m00, float m01, float m02,
				   float m10, float m11, float m12,
				   float m20, float m21, float m22 )
{
	m_elements[ 0 ] = m00;
	m_elements[ 1 ] = m10;
	m_elements[ 2 ] = m20;

	m_elements[ 3 ] = m01;
	m_elements[ 4 ] = m11;
	m_elements[ 5 ] = m21;

	m_elements[ 6 ] = m02;
	m_elements[ 7 ] = m12;
	m_elements[ 8 ] = m22;
}



Matrix3f Matrix3f::rotation( const Quat4f& rq )
{
	Quat4f q = rq.normalized();

	float xx = q.x() * q.x();
	float yy = q.y() * q.y();
	float zz = q.z() * q.z();

	float xy = q.x() * q.y();
	float zw = q.z() * q.w();

	float xz = q.x() * q.z();
	float yw = q.y() * q.w();

	float yz = q.y() * q.z();
	float xw = q.x() * q.w();

	return Matrix3f
		(
			1.0f - 2.0f * ( yy + zz ),		2.0f * ( xy - zw ),				2.0f * ( xz + yw ),
			2.0f * ( xy + zw ),				1.0f - 2.0f * ( xx + zz ),		2.0f * ( yz - xw ),
			2.0f * ( xz - yw ),				2.0f * ( yz + xw ),				1.0f - 2.0f * ( xx + yy )
		);
}




#endif
