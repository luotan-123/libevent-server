#pragma once
#include "ControlHead.h"
#include <float.h>

#define	_MaxFloat				FLT_MAX
#define	_Epsilon				FLT_EPSILON
#define _Pi						3.14159265359f
#define	_Sqrt(x)				std::sqrt(x)
#define	_Atan2(y, x)			std::atan2(y, x)
#define _MaxPolygonVertices		8
#define _LinearSlop				0.005f
#define _AngularSlop			(2.0f / 180.0f * _Pi)
#define _PolygonRadius			(2.0f * _LinearSlop)

#ifdef _DEBUG
#define _Assert(A)				assert(A)
#else
#define _Assert(A)
#endif


struct CFloatVector2
{
	CFloatVector2() {}
	CFloatVector2(float x, float y) : x(x), y(y) {}
	CFloatVector2(double x, double y) : x((float)x), y((float)y) {}

	void			operator += (const CFloatVector2& v){ x += v.x; y += v.y; }
	void			operator -= (const CFloatVector2& v){ x -= v.x; y -= v.y; }
	void			operator *= (float a)				{ x *= a; y *= a; }
	CFloatVector2	operator -() const					{ CFloatVector2 v; v.Set(-x, -y); return v; }
	float			operator () (int i) const			{ return (&x)[i]; }
	float&			operator () (int i)					{ return (&x)[i]; }

	inline void		Set(float x_, float y_)				{ x = x_; y = y_; }
	inline void		SetZero()							{ x = 0.0f; y = 0.0f; }
	inline float	Length() const						{ return _Sqrt(x * x + y * y);}
	inline float	LengthSquared() const				{ return x * x + y * y; }
	inline float	Normalize()							{ float length = Length(); if (length < _Epsilon) { return 0.0f; } float invLength = 1.0f / length; x *= invLength; y *= invLength; return length; }

	float x, y;
};

struct CFloatRotate
{
	CFloatRotate() {}
	explicit CFloatRotate(float fAngle)				{ s = sinf(fAngle); c = cosf(fAngle); }

	inline void				Set(float fAngle)		{ s = sinf(fAngle); c = cosf(fAngle); }
	inline void				SetIdentity()			{ s = 0.0f; c = 1.0f; }
	inline float			GetAngle() const		{ return _Atan2(s, c); }
	inline CFloatVector2	GetXAxis() const		{ return CFloatVector2(c, s); }
	inline CFloatVector2	GetYAxis() const		{ return CFloatVector2(-s, c); }

	float s, c;
};

struct CFloatTransform
{
	CFloatTransform() {}
	CFloatTransform(const CFloatVector2& Position, const CFloatRotate& Rotation) : p(Position), q(Rotation) {}

	inline void Set(const CFloatVector2& Position, float fAngle) { p = Position; q.Set(fAngle); }
	inline void SetIdentity()									{ p.SetZero(); q.SetIdentity(); }

	CFloatVector2 p;
	CFloatRotate q;
};

class CPolygonShape
{
public:
	CPolygonShape();
	CPolygonShape( const CPolygonShape& right );

	void Set(const CFloatVector2* pVertices, int nVertexCount);
	void SetAsBox( float hX, float hY );
	void SetAsBox( float hX, float hY, const CFloatVector2& Center, float fAngle );
	bool PointInShape( const CFloatTransform& Transform, const CFloatVector2& Pt ) const;

	CFloatVector2 m_centroid;
	CFloatVector2 m_vertices[_MaxPolygonVertices];
	CFloatVector2 m_normals[_MaxPolygonVertices];
	int m_vertexCount;
};

inline CFloatVector2		operator + (const CFloatVector2& a, const CFloatVector2& b)		{ return CFloatVector2(a.x + b.x, a.y + b.y); }
inline CFloatVector2		operator - (const CFloatVector2& a, const CFloatVector2& b)		{ return CFloatVector2(a.x - b.x, a.y - b.y); }
inline CFloatVector2		operator * (float s, const CFloatVector2& a)					{ return CFloatVector2(s * a.x, s * a.y); }
inline bool					operator == (const CFloatVector2& a, const CFloatVector2& b)	{ return a.x == b.x && a.y == b.y; }
inline float				b2Dot(const CFloatVector2& a, const CFloatVector2& b)			{ return a.x * b.x + a.y * b.y; }
inline float				b2Cross(const CFloatVector2& a, const CFloatVector2& b)			{ return a.x * b.y - a.y * b.x; }
inline CFloatVector2		b2Cross(const CFloatVector2& a, float s)						{ return CFloatVector2(s * a.y, -s * a.x); }
inline CFloatVector2		b2Cross(float s, const CFloatVector2& a)						{ return CFloatVector2(-s * a.y, s * a.x); }
inline float				b2Distance(const CFloatVector2& a, const CFloatVector2& b)		{ CFloatVector2 c = a - b; return c.Length(); }
inline float				b2DistanceSquared(const CFloatVector2& a, const CFloatVector2& b){ CFloatVector2 c = a - b; return b2Dot(c, c); }
inline CFloatRotate			b2Mul(const CFloatRotate& q, const CFloatRotate& r)				{ CFloatRotate qr; qr.s = q.s * r.c + q.c * r.s; qr.c = q.c * r.c - q.s * r.s; return qr; }
inline CFloatRotate			b2MulT(const CFloatRotate& q, const CFloatRotate& r)			{ CFloatRotate qr; qr.s = q.c * r.s - q.s * r.c; qr.c = q.c * r.c + q.s * r.s; return qr; }
inline CFloatVector2		b2Mul(const CFloatRotate& q, const CFloatVector2& v)			{ return CFloatVector2(q.c * v.x - q.s * v.y, q.s * v.x + q.c * v.y); }
inline CFloatVector2		b2MulT(const CFloatRotate& q, const CFloatVector2& v)			{ return CFloatVector2(q.c * v.x + q.s * v.y, -q.s * v.x + q.c * v.y); }
inline CFloatVector2		b2Mul(const CFloatTransform& T, const CFloatVector2& v)			{ float x = (T.q.c * v.x - T.q.s * v.y) + T.p.x; float y = (T.q.s * v.x + T.q.c * v.y) + T.p.y; return CFloatVector2(x, y); }
inline CFloatVector2		b2MulT(const CFloatTransform& T, const CFloatVector2& v)		{ float px = v.x - T.p.x; float py = v.y - T.p.y; float x = (T.q.c * px + T.q.s * py); float y = (-T.q.s * px + T.q.c * py); return CFloatVector2(x, y); }
inline CFloatTransform		b2Mul(const CFloatTransform& A, const CFloatTransform& B)		{ CFloatTransform C; C.q = b2Mul(A.q, B.q); C.p = b2Mul(A.q, B.p) + A.p; return C; }
inline CFloatTransform		b2MulT(const CFloatTransform& A, const CFloatTransform& B)		{ CFloatTransform C; C.q = b2MulT(A.q, B.q); C.p = b2MulT(A.q, B.p - A.p); return C; }

bool ShapeOverlap( const CPolygonShape* pShapeA, const CPolygonShape* pShapeB, const CFloatTransform& xfA, const CFloatTransform& xfB );

// ----------------------------------------------------------------------------------
// 怪物区域定义
#define	MONSTER_VEC						6

const CFloatVector2 MonsterVec[FishType_Max][MONSTER_VEC] = 
{
	// 1
	{	 
		CFloatVector2( 12.f - 12.f, 6.f - 35.f ), 
			CFloatVector2( 18.f - 12.f, 18.f - 35.f ), 
			CFloatVector2( 17.f - 12.f, 46.f - 35.f ), 
			CFloatVector2( 11.f - 12.f, 49.f - 35.f ), 
			CFloatVector2( 7.f - 12.f, 47.f - 35.f ), 
			CFloatVector2( 6.f - 12.f, 20.f - 35.f ), 
	},

	// 2
	{	
		CFloatVector2( 10.f - 12.f, 1.f - 27.f ), 
			CFloatVector2( 15.f - 12.f, 14.f - 27.f ), 
			CFloatVector2( 15.f - 12.f, 29.f - 27.f ), 
			CFloatVector2( 9.f - 12.f, 41.f - 27.f ), 
			CFloatVector2( 3.f - 12.f, 31.f - 27.f ), 
			CFloatVector2( 2.f - 12.f, 13.f - 27.f ), 
	},

	// 3
		{	
			CFloatVector2( 27.f - 24.f, 2.f - 39.f ), 
				CFloatVector2( 34.f - 24.f, 17.f - 39.f ), 
				CFloatVector2( 35.f - 24.f, 51.f - 39.f ), 
				CFloatVector2( 24.f - 24.f, 61.f - 39.f ), 
				CFloatVector2( 11.f - 24.f, 53.f - 39.f ), 
				CFloatVector2( 11.f - 24.f, 24.f - 39.f ), 
		},

		// 4
		{	
			CFloatVector2( 32.f - 32.f, 15.f - 50.f ), 
				CFloatVector2( 57.f - 32.f, 13.f - 50.f ), 
				CFloatVector2( 39.f - 32.f, 54.f - 50.f ), 
				CFloatVector2( 33.f - 32.f, 64.f - 50.f ), 
				CFloatVector2( 22.f - 32.f, 44.f - 50.f ), 
				CFloatVector2( 16.f - 32.f, 17.f - 50.f ), 
		},

		// 5
			{	
				CFloatVector2( 21.f - 22.f, 10.f - 62.f ), 
					CFloatVector2( 38.f - 22.f, 30.f - 62.f ), 
					CFloatVector2( 35.f - 22.f, 72.f - 62.f ), 
					CFloatVector2( 23.f - 22.f, 91.f - 62.f ), 
					CFloatVector2( 7.f - 22.f, 72.f - 62.f ), 
					CFloatVector2( 7.f - 22.f, 35.f - 62.f ), 
			},

			// 6
			{	
				CFloatVector2( 34.f - 36.f, 0.f - 52.f ), 
					CFloatVector2( 53.f - 36.f, 21.f - 52.f ), 
					CFloatVector2( 51.f - 36.f, 65.f - 52.f ), 
					CFloatVector2( 32.f - 36.f, 79.f - 52.f ), 
					CFloatVector2( 15.f - 36.f, 63.f - 52.f ), 
					CFloatVector2( 16.f - 36.f, 24.f - 52.f ), 
			},

			// 7
				{	
					CFloatVector2( 27.f - 28.f, 6.f - 60.f ), 
						CFloatVector2( 44.f - 28.f, 18.f - 60.f ), 
						CFloatVector2( 39.f - 28.f, 54.f - 60.f ), 
						CFloatVector2( 29.f - 28.f, 84.f - 60.f ), 
						CFloatVector2( 18.f - 28.f, 60.f - 60.f ), 
						CFloatVector2( 11.f - 28.f, 20.f - 60.f ), 
				},

				// 8
				{	
					CFloatVector2( 37.f - 37.f, 5.f - 66.f ), 
						CFloatVector2( 50.f - 37.f, 41.f - 66.f ), 
						CFloatVector2( 41.f - 37.f, 97.f - 66.f ), 
						CFloatVector2( 30.f - 37.f, 93.f - 66.f ), 
						CFloatVector2( 13.f - 37.f, 66.f - 66.f ), 
						CFloatVector2( 7.f - 37.f, 21.f - 66.f ), 
				},

				// 9
					{	
						CFloatVector2( 35.f - 35.f, 5.f - 80.f ), 
							CFloatVector2( 45.f - 35.f, 47.f - 80.f ), 
							CFloatVector2( 55.f - 35.f, 105.f - 80.f ), 
							CFloatVector2( 32.f - 35.f, 130.f - 80.f ), 
							CFloatVector2( 13.f - 35.f, 104.f - 80.f ), 
							CFloatVector2( 11.f - 35.f, 57.f - 80.f ), 
					},

					// 10
					{	
						CFloatVector2( 70.f - 72.f, 0.f - 67.f ), 
							CFloatVector2( 96.f - 72.f, 25.f - 67.f ), 
							CFloatVector2( 101.f - 72.f, 92.f - 67.f ), 
							CFloatVector2( 69.f - 72.f, 122.f - 67.f ), 
							CFloatVector2( 39.f - 72.f, 96.f - 67.f ), 
							CFloatVector2( 47.f - 72.f, 22.f - 67.f ), 
					},

					// 11
						{	
							CFloatVector2( 52.f - 42.f, 5.f - 95.f ), 
								CFloatVector2( 72.f - 42.f, 76.f - 95.f ), 
								CFloatVector2( 59.f - 42.f, 99.f - 95.f ), 
								CFloatVector2( 46.f - 42.f, 115.f - 95.f ), 
								CFloatVector2( 21.f - 42.f, 91.f - 95.f ), 
								CFloatVector2( 29.f - 42.f, 34.f - 95.f ), 
						},

						// 12
						{	
							CFloatVector2( 80.f - 82.f, 1.f - 79.f ), 
								CFloatVector2( 157.f - 82.f, 42.f - 79.f ), 
								CFloatVector2( 122.f - 82.f, 110.f - 79.f ), 
								CFloatVector2( 77.f - 82.f, 122.f - 79.f ), 
								CFloatVector2( 28.f - 82.f, 89.f - 79.f ), 
								CFloatVector2( 12.f - 82.f, 28.f - 79.f ), 
						},

						// 13
							{	
								CFloatVector2( 43.f - 44.f, 2.f - 110.f ), 
									CFloatVector2( 66.f - 44.f, 32.f - 110.f ), 
									CFloatVector2( 67.f - 44.f, 146.f - 110.f ), 
									CFloatVector2( 42.f - 44.f, 178.f - 110.f ), 
									CFloatVector2( 25.f - 44.f, 150.f - 110.f ), 
									CFloatVector2( 19.f - 44.f, 23.f - 110.f ), 
							},

							// 14
							{	
								CFloatVector2( 48.f - 48.f, 12.f - 142.f ), 
									CFloatVector2( 65.f - 48.f, 89.f - 142.f ), 
									CFloatVector2( 62.f - 48.f, 168.f - 142.f ), 
									CFloatVector2( 43.f - 48.f, 216.f - 142.f ), 
									CFloatVector2( 22.f - 48.f, 147.f - 142.f ), 
									CFloatVector2( 23.f - 48.f, 66.f - 142.f ), 
							},

							// 15
								{	
									CFloatVector2( 119.f - 130.f, 2.f - 105.f ), 
										CFloatVector2( 246.f - 130.f, 59.f - 105.f ), 
										CFloatVector2( 164.f - 130.f, 123.f - 105.f ), 
										CFloatVector2( 122.f - 130.f, 143.f - 105.f ), 
										CFloatVector2( 79.f - 130.f, 116.f - 105.f ), 
										CFloatVector2( 21.f - 130.f, 52.f - 105.f ), 
								},

								// 16
								{	
									CFloatVector2( 67.f - 70.f, 5.f - 150.f ), 
										CFloatVector2( 112.f - 70.f, 19.f - 150.f ), 
										CFloatVector2( 80.f - 70.f, 169.f - 150.f ), 
										CFloatVector2( 62.f - 70.f, 227.f - 150.f ), 
										CFloatVector2( 39.f - 70.f, 154.f - 150.f ), 
										CFloatVector2( 25.f - 70.f, 20.f - 150.f ), 
								},

								// 17
									{	
										CFloatVector2( 66.f - 70.f, 2.f - 150.f ), 
											CFloatVector2( 113.f - 70.f, 21.f - 150.f ), 
											CFloatVector2( 87.f - 70.f, 149.f - 150.f ), 
											CFloatVector2( 64.f - 70.f, 216.f - 150.f ), 
											CFloatVector2( 40.f - 70.f, 157.f - 150.f ), 
											CFloatVector2( 17.f - 70.f, 26.f - 150.f ), 
									},

									// 18
									{	
										CFloatVector2( 133.f - 134.f, 17.f - 61.f ), 
											CFloatVector2( 213.f - 134.f, 34.f - 61.f ), 
											CFloatVector2( 210.f - 134.f, 81.f - 61.f ), 
											CFloatVector2( 133.f - 134.f, 116.f - 61.f ), 
											CFloatVector2( 46.f - 134.f, 93.f - 61.f ), 
											CFloatVector2( 47.f - 134.f, 54.f - 61.f ), 
									},

									// 19
										{	
											CFloatVector2( 157.f - 152.f, 12.f - 256.f ), 
												CFloatVector2( 217.f - 152.f, 89.f - 256.f ), 
												CFloatVector2( 205.f - 152.f, 291.f - 256.f ), 
												CFloatVector2( 152.f - 152.f, 374.f - 256.f ), 
												CFloatVector2( 98.f - 152.f, 262.f - 256.f ), 
												CFloatVector2( 91.f - 152.f, 86.f - 256.f ), 
										},

										// 20
										{	
											CFloatVector2( 105.f - 108.f, 21.f - 228.f ), 
												CFloatVector2( 145.f - 108.f, 99.f - 228.f ), 
												CFloatVector2( 125.f - 108.f, 260.f - 228.f ), 
												CFloatVector2( 103.f - 108.f, 334.f - 228.f ), 
												CFloatVector2( 84.f - 108.f, 229.f - 228.f ), 
												CFloatVector2( 71.f - 108.f, 79.f - 228.f ), 

										},

										// 21
											{	
												CFloatVector2( 109.f - 124.f, 81.f - 140.f ), 
													CFloatVector2( 216.f - 124.f, 111.f - 140.f ), 
													CFloatVector2( 192.f - 124.f, 189.f - 140.f ), 
													CFloatVector2( 130.f - 124.f, 229.f - 140.f ), 
													CFloatVector2( 51.f - 124.f, 187.f - 140.f ), 
													CFloatVector2( 42.f - 124.f, 114.f - 140.f ), 
											},

											// 22
											{	
												CFloatVector2( 62.f - 65.f, 8.f - 68.f ), 
													CFloatVector2( 113.f - 65.f, 53.f - 68.f ), 
													CFloatVector2( 116.f - 65.f, 100.f - 68.f ), 
													CFloatVector2( 60.f - 65.f, 123.f - 68.f ), 
													CFloatVector2( 17.f - 65.f, 84.f - 68.f ), 
													CFloatVector2( 25.f - 65.f, 17.f - 68.f ), 
											},

											// 23
												{	
													CFloatVector2( 62.f - 64.f, 10.f - 64.f ), 
														CFloatVector2( 111.f - 64.f, 30.f - 64.f ), 
														CFloatVector2( 102.f - 64.f, 92.f - 64.f ), 
														CFloatVector2( 62.f - 64.f, 108.f - 64.f ), 
														CFloatVector2( 22.f - 64.f, 90.f - 64.f ), 
														CFloatVector2( 22.f - 64.f, 33.f - 64.f ), 
												},
};


const CFloatVector2 MonsterRect[FishType_Max] = 
{
	// 1 
	CFloatVector2( 12.f, 35.f ), 

	// 2
	CFloatVector2( 12.f, 27.f ), 
	// 3
	CFloatVector2( 24.f, 39.f ), 

	// 4
	CFloatVector2( 32.f, 50.f ), 

	// 5
	CFloatVector2( 22.f, 62.f ), 

	// 6
	CFloatVector2( 36.f, 52.f ), 

	// 7
	CFloatVector2( 28.f, 60.f ), 

	// 8
	CFloatVector2( 37.f, 66.f ), 

	// 9
	CFloatVector2( 35.f, 80.f ),  

	// 10
	CFloatVector2( 72.f, 67.f ), 

	// 11
	CFloatVector2( 42.f, 95.f ), 

	// 12
	CFloatVector2( 82.f, 79.f ), 

	// 13
	CFloatVector2( 44.f, 110.f ), 

	// 14
	CFloatVector2( 48.f, 142.f ), 

	// 15
	CFloatVector2( 130.f, 105.f ), 

	// 16
	CFloatVector2( 70.f, 150.f ), 

	// 17
	CFloatVector2( 70.f, 150.f ), 

	// 18
	CFloatVector2( 134.f, 61.f ), 

	// 19
	CFloatVector2( 152.f, 256.f ), 

	// 20
	CFloatVector2( 108.f, 228.f ), 

	// 21
	CFloatVector2( 124.f, 140.f ), 

	// 22
	CFloatVector2( 65.f, 68.f ), 

	// 23
	CFloatVector2( 64.f, 64.f ), 
};