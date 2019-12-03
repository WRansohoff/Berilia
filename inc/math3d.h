#ifndef BERILIA_MATH3D
#define BERILIA_MATH3D

#include <math.h>
#include <string>

/** Rough approximation of pi to 5 digits. */
#define PI 3.14159

using std::string;
using std::to_string;

// Math struct forward declarations.
struct v3;
struct v4;
struct m4;
struct quat;
struct aabb;

/** Struct representing a vector with a length of 3. */
struct v3 {
  /** Float buffer that holds the vector elements. */
  float v[ 3 ];
  v3();
  v3( float x, float y, float z );
  v3( const v3& other );
  v3( const v4& other );

  v3 operator+( const v3& other );
  v3 operator+( float scalar );
  v3& operator+=( const v3& other );
  v3& operator+=( float scalar );
  v3 operator-( const v3& other );
  v3 operator-( float scalar );
  v3& operator-=( const v3& other );
  v3& operator-=( float scalar );
  v3 operator*( float scalar );
  v3& operator*=( float scalar );
  v3 operator/( float scalar );
  v3& operator/=( float scalar);
  v3 operator/( v3 other );
  v3& operator/=( v3 other );
  v3& operator=( const v3& other );
  bool operator==( const v3& other ) const;
};

/** Struct representing a vector with a length of 4. */
struct v4 {
  /** Float buffer that holds the vector elements. */
  float v[ 4 ];
  v4();
  v4( float x, float y, float z, float w );
  v4( const v3& other, float w );

  v4 operator-( const v4& other );
  v4 operator-( float scalar );
  v4 operator-=( const v4& other );
  v4 operator-=( float scalar );
  v4 operator+( const v4& other );
  v4 operator+( float scalar );
  v4 operator+=( const v4& other );
  v4 operator+=( float scalar );
  v4 operator*( float scalar );
  v4& operator*=( float scalar );
  v4 operator/( float scalar );
  v4& operator/=( float scalar );
};

/**
 * Struct representing a 4x4 matrix. Buffer ordering:
 * 0  1  2  3
 * 4  5  6  7
 * 8  9  10 11
 * 12 13 14 15
 */
struct m4 {
  /** Float buffer that holds the matrix elements. */
  float m[ 16 ];
  m4();
  m4( float a, float b, float c, float d,
      float e, float f, float g, float h,
      float i, float j, float k, float l,
      float n, float o, float p, float q );

  v4 operator*( const v4& other );
  m4 operator*( const m4& other );
  m4 operator*( const float scalar );
  m4& operator=( const m4& other );

  v4 row( const int row );
  v4 col( const int row );
  v3 translation();
  float t_x();
  float t_y();
  float t_z();
};

/**
 * Struct representing a quaternion. It holds both the calculated
 * quaternion W / X / Y / Z values, and an axis-angle representation
 * which is easier for people to reason with.
 */
struct quat {
  /** Float buffer that holds the calculated quaternion elements. */
  float q[ 4 ];
  /** Float buffer that holds the raw axis / angle elements. */
  float r[ 4 ];

  quat();
  quat( float a, float x, float y, float z );

  quat operator/( float scalar );
  quat& operator/=( float scalar );
  // q1 * q2 roughly means, "rotate by q2, then rotate by q1".
  quat operator*( quat other );
  quat& operator*=( quat& other );
  quat& operator=( const quat& other );

  void r_u();
};

/** Struct representing an axis-aligned bounding box. */
struct aabb {
  /** width along the X-axis. */
  float x_w;
  /** Height along the Y-axis. */
  float y_h;
  /** Depth along the Z-axis. */
  float z_d;

  aabb();
  aabb( float x_width, float y_height, float z_depth );
};

string print( v3 vec );
string print( v4 vec );
string print( m4 mat );
string print( quat q );
string print( aabb b );

float ang_to_rad( float a );
float rad_to_ang( float r );

float magnitude( v3& vec );
float magnitude2( v3 vec );
v3 normalize( v3 vec );
v3 cross( v3& vec1, v3& vec2 );
float dot( v3& vec1, v3& vec2 );
v3 lerp( v3& vec1, v3& vec2, float prog );
float distance( v3 vec1, v3 vec2 );
float distance2( v3 vec1, v3 vec2 );

float determinant( m4 mat );
m4 id4();
m4 inverse( m4 orig );
m4 transpose( m4 orig );
m4 rotate_x( float degrees );
m4 rotate_x_rads( float rads );
m4 rotate_y( float degrees );
m4 rotate_y_rads( float rads );
m4 rotate_z( float degrees );
m4 rotate_z_rads( float rads );
m4 translation_matrix( float x, float y, float z );
m4 translation_matrix( v3 t );
m4 quaternion_to_rotation( quat q );
m4 view_matrix( m4 translation, m4 rotation );
m4 perspective( float near,
                float far,
                float fov,
                float aspect_ratio );

float dot( quat q1, quat q2 );
float norm( quat q );
quat set_quat( float a, v3 vec );
quat set_quat( float a, float x, float y, float z );
quat normalize( quat q );
quat conjugate( quat q );
quat inverse( quat q );
quat slerp( quat q1, quat q2, float prog );

#endif
