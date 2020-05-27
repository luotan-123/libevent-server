#include "PolygonShape.h"

const CFloatVector2		b2Vec2_zero(0.0f, 0.0f);

static CFloatVector2 ComputeCentroid(const CFloatVector2* vs, int count)
{
	_Assert(count >= 3);

	CFloatVector2 c; c.Set(0.0f, 0.0f);
	float area = 0.0f;

	CFloatVector2 pRef(0.0f, 0.0f);
#if 0
	for (int i = 0; i < count; ++i)
	{
		pRef += vs[i];
	}
	pRef *= 1.0f / count;
#endif

	const float inv3 = 1.0f / 3.0f;

	for (int i = 0; i < count; ++i)
	{
		CFloatVector2 p1 = pRef;
		CFloatVector2 p2 = vs[i];
		CFloatVector2 p3 = i + 1 < count ? vs[i+1] : vs[0];

		CFloatVector2 e1 = p2 - p1;
		CFloatVector2 e2 = p3 - p1;

		float D = b2Cross(e1, e2);

		float triangleArea = 0.5f * D;
		area += triangleArea;

		c += triangleArea * inv3 * (p1 + p2 + p3);
	}
	_Assert(area > _Epsilon);
	c *= 1.0f / area;
	return c;
}


CPolygonShape::CPolygonShape()
{
	m_vertexCount = 0;
	m_centroid.SetZero();
}

CPolygonShape::CPolygonShape( const CPolygonShape& right )
{
	m_centroid = right.m_centroid;
	m_vertexCount = right.m_vertexCount;
	for( int i = 0; i < _MaxPolygonVertices; ++i )
	{
		m_vertices[i] = right.m_vertices[i];
		m_normals[i] = right.m_normals[i];
	}
}

void CPolygonShape::Set(const CFloatVector2* vertices, int count)
{
	_Assert(3 <= count && count <= _MaxPolygonVertices);
	m_vertexCount = count;

	for (int i = 0; i < m_vertexCount; ++i)
	{
		m_vertices[i] = vertices[i];
	}

	for (int i = 0; i < m_vertexCount; ++i)
	{
		int i1 = i;
		int i2 = i + 1 < m_vertexCount ? i + 1 : 0;
		CFloatVector2 edge = m_vertices[i2] - m_vertices[i1];
		_Assert(edge.LengthSquared() > _Epsilon * _Epsilon);
		m_normals[i] = b2Cross(edge, 1.0f);
		m_normals[i].Normalize();
	}

#ifdef _DEBUG
	for (int i = 0; i < m_vertexCount; ++i)
	{
		int i1 = i;
		int i2 = i + 1 < m_vertexCount ? i + 1 : 0;
		CFloatVector2 edge = m_vertices[i2] - m_vertices[i1];

		for (int j = 0; j < m_vertexCount; ++j)
		{
			if (j == i1 || j == i2)
			{
				continue;
			}

			CFloatVector2 r = m_vertices[j] - m_vertices[i1];
			float s = b2Cross(edge, r);
			_Assert(s > 0.0f && "ERROR: Please ensure your polygon is convex and has a CCW winding order");
		}
	}
#endif

	m_centroid = ComputeCentroid(m_vertices, m_vertexCount);
}


void CPolygonShape::SetAsBox(float hx, float hy)
{
	m_vertexCount = 4;
	m_vertices[0].Set(-hx, -hy);
	m_vertices[1].Set( hx, -hy);
	m_vertices[2].Set( hx,  hy);
	m_vertices[3].Set(-hx,  hy);
	m_normals[0].Set(0.0f, -1.0f);
	m_normals[1].Set(1.0f, 0.0f);
	m_normals[2].Set(0.0f, 1.0f);
	m_normals[3].Set(-1.0f, 0.0f);
	m_centroid.SetZero();
}

void CPolygonShape::SetAsBox(float hx, float hy, const CFloatVector2& center, float angle)
{
	m_vertexCount = 4;
	m_vertices[0].Set(-hx, -hy);
	m_vertices[1].Set( hx, -hy);
	m_vertices[2].Set( hx,  hy);
	m_vertices[3].Set(-hx,  hy);
	m_normals[0].Set(0.0f, -1.0f);
	m_normals[1].Set(1.0f, 0.0f);
	m_normals[2].Set(0.0f, 1.0f);
	m_normals[3].Set(-1.0f, 0.0f);
	m_centroid = center;

	CFloatTransform xf;
	xf.p = center;
	xf.q.Set(angle);

	for (int i = 0; i < m_vertexCount; ++i)
	{
		m_vertices[i] = b2Mul(xf, m_vertices[i]);
		m_normals[i] = b2Mul(xf.q, m_normals[i]);
	}
}

bool CPolygonShape::PointInShape(const CFloatTransform& xf, const CFloatVector2& p) const
{
	CFloatVector2 pLocal = b2MulT(xf.q, p - xf.p);

	for (int i = 0; i < m_vertexCount; ++i)
	{
		float dot = b2Dot(m_normals[i], pLocal - m_vertices[i]);
		if (dot > 0.0f)
		{
			return false;
		}
	}

	return true;
}

//----------------------------------------------------------------------------------

struct b2DistanceProxy
{
	b2DistanceProxy() : m_vertices(NULL), m_count(0) {}

	void Set(const CPolygonShape* shape )
	{
		const CPolygonShape* polygon = (CPolygonShape*)shape;
		m_vertices = polygon->m_vertices;
		m_count = polygon->m_vertexCount;
	}


	int GetSupport(const CFloatVector2& d) const
	{
		int bestIndex = 0;
		float bestValue = b2Dot(m_vertices[0], d);
		for (int i = 1; i < m_count; ++i)
		{
			float value = b2Dot(m_vertices[i], d);
			if (value > bestValue)
			{
				bestIndex = i;
				bestValue = value;
			}
		}

		return bestIndex;
	}

	CFloatVector2 m_buffer[2];
	const CFloatVector2* m_vertices;
	int m_count;
};

struct b2SimplexCache
{
	float metric;		
	unsigned short count;
	unsigned char indexA[3];	
	unsigned char indexB[3];	
};

struct b2DistanceInput
{
	b2DistanceProxy proxyA;
	b2DistanceProxy proxyB;
	CFloatTransform transformA;
	CFloatTransform transformB;
	bool useRadii;
};

struct b2DistanceOutput
{
	CFloatVector2 pointA;		
	CFloatVector2 pointB;		
	float distance;
	int iterations;
};

struct b2SimplexVertex
{
	CFloatVector2 wA;		
	CFloatVector2 wB;		
	CFloatVector2 w;		
	float a;		
	int indexA;	
	int indexB;
};

struct b2Simplex
{
	void ReadCache(	const b2SimplexCache* cache,
		const b2DistanceProxy* proxyA, const CFloatTransform& transformA,
		const b2DistanceProxy* proxyB, const CFloatTransform& transformB)
	{
		_Assert(cache->count <= 3);

		m_count = cache->count;
		b2SimplexVertex* vertices = &m_v1;
		for (int i = 0; i < m_count; ++i)
		{
			b2SimplexVertex* v = vertices + i;
			v->indexA = cache->indexA[i];
			v->indexB = cache->indexB[i];
			CFloatVector2 wALocal = proxyA->m_vertices[v->indexA];
			CFloatVector2 wBLocal = proxyB->m_vertices[v->indexB];
			v->wA = b2Mul(transformA, wALocal);
			v->wB = b2Mul(transformB, wBLocal);
			v->w = v->wB - v->wA;
			v->a = 0.0f;
		}

		if (m_count > 1)
		{
			float metric1 = cache->metric;
			float metric2 = GetMetric();
			if (metric2 < 0.5f * metric1 || 2.0f * metric1 < metric2 || metric2 < _Epsilon)
			{
				m_count = 0;
			}
		}

		if (m_count == 0)
		{
			b2SimplexVertex* v = vertices + 0;
			v->indexA = 0;
			v->indexB = 0;
			CFloatVector2 wALocal = proxyA->m_vertices[0];
			CFloatVector2 wBLocal = proxyB->m_vertices[0];
			v->wA = b2Mul(transformA, wALocal);
			v->wB = b2Mul(transformB, wBLocal);
			v->w = v->wB - v->wA;
			m_count = 1;
		}
	}

	void WriteCache(b2SimplexCache* cache) const
	{
		cache->metric = GetMetric();
		cache->count = (unsigned short)m_count;
		const b2SimplexVertex* vertices = &m_v1;
		for (int i = 0; i < m_count; ++i)
		{
			cache->indexA[i] = (unsigned char)vertices[i].indexA;
			cache->indexB[i] = (unsigned char)vertices[i].indexB;
		}
	}

	CFloatVector2 GetSearchDirection() const
	{
		switch (m_count)
		{
		case 1:
			return -m_v1.w;

		case 2:
			{
				CFloatVector2 e12 = m_v2.w - m_v1.w;
				float sgn = b2Cross(e12, -m_v1.w);
				if (sgn > 0.0f)
				{
					return b2Cross(1.0f, e12);
				}
				else
				{
					return b2Cross(e12, 1.0f);
				}
			}

		default:
			_Assert(false);
			return b2Vec2_zero;
		}
	}

	CFloatVector2 GetClosestPoint() const
	{
		switch (m_count)
		{
		case 0:
			_Assert(false);
			return b2Vec2_zero;

		case 1:
			return m_v1.w;

		case 2:
			return m_v1.a * m_v1.w + m_v2.a * m_v2.w;

		case 3:
			return b2Vec2_zero;

		default:
			_Assert(false);
			return b2Vec2_zero;
		}
	}

	void GetWitnessPoints(CFloatVector2* pA, CFloatVector2* pB) const
	{
		switch (m_count)
		{
		case 0:
			_Assert(false);
			break;

		case 1:
			*pA = m_v1.wA;
			*pB = m_v1.wB;
			break;

		case 2:
			*pA = m_v1.a * m_v1.wA + m_v2.a * m_v2.wA;
			*pB = m_v1.a * m_v1.wB + m_v2.a * m_v2.wB;
			break;

		case 3:
			*pA = m_v1.a * m_v1.wA + m_v2.a * m_v2.wA + m_v3.a * m_v3.wA;
			*pB = *pA;
			break;

		default:
			_Assert(false);
			break;
		}
	}

	float GetMetric() const
	{
		switch (m_count)
		{
		case 0:
			_Assert(false);
			return 0.0;

		case 1:
			return 0.0f;

		case 2:
			return b2Distance(m_v1.w, m_v2.w);

		case 3:
			return b2Cross(m_v2.w - m_v1.w, m_v3.w - m_v1.w);

		default:
			_Assert(false);
			return 0.0f;
		}
	}

	void Solve2();
	void Solve3();

	b2SimplexVertex m_v1, m_v2, m_v3;
	int m_count;
};

void b2Simplex::Solve2()
{
	CFloatVector2 w1 = m_v1.w;
	CFloatVector2 w2 = m_v2.w;
	CFloatVector2 e12 = w2 - w1;

	float d12_2 = -b2Dot(w1, e12);
	if (d12_2 <= 0.0f)
	{
		m_v1.a = 1.0f;
		m_count = 1;
		return;
	}

	float d12_1 = b2Dot(w2, e12);
	if (d12_1 <= 0.0f)
	{
		m_v2.a = 1.0f;
		m_count = 1;
		m_v1 = m_v2;
		return;
	}

	float inv_d12 = 1.0f / (d12_1 + d12_2);
	m_v1.a = d12_1 * inv_d12;
	m_v2.a = d12_2 * inv_d12;
	m_count = 2;
}

void b2Simplex::Solve3()
{
	CFloatVector2 w1 = m_v1.w;
	CFloatVector2 w2 = m_v2.w;
	CFloatVector2 w3 = m_v3.w;

	CFloatVector2 e12 = w2 - w1;
	float w1e12 = b2Dot(w1, e12);
	float w2e12 = b2Dot(w2, e12);
	float d12_1 = w2e12;
	float d12_2 = -w1e12;

	CFloatVector2 e13 = w3 - w1;
	float w1e13 = b2Dot(w1, e13);
	float w3e13 = b2Dot(w3, e13);
	float d13_1 = w3e13;
	float d13_2 = -w1e13;

	CFloatVector2 e23 = w3 - w2;
	float w2e23 = b2Dot(w2, e23);
	float w3e23 = b2Dot(w3, e23);
	float d23_1 = w3e23;
	float d23_2 = -w2e23;

	float n123 = b2Cross(e12, e13);

	float d123_1 = n123 * b2Cross(w2, w3);
	float d123_2 = n123 * b2Cross(w3, w1);
	float d123_3 = n123 * b2Cross(w1, w2);

	if (d12_2 <= 0.0f && d13_2 <= 0.0f)
	{
		m_v1.a = 1.0f;
		m_count = 1;
		return;
	}

	if (d12_1 > 0.0f && d12_2 > 0.0f && d123_3 <= 0.0f)
	{
		float inv_d12 = 1.0f / (d12_1 + d12_2);
		m_v1.a = d12_1 * inv_d12;
		m_v2.a = d12_2 * inv_d12;
		m_count = 2;
		return;
	}

	if (d13_1 > 0.0f && d13_2 > 0.0f && d123_2 <= 0.0f)
	{
		float inv_d13 = 1.0f / (d13_1 + d13_2);
		m_v1.a = d13_1 * inv_d13;
		m_v3.a = d13_2 * inv_d13;
		m_count = 2;
		m_v2 = m_v3;
		return;
	}

	if (d12_1 <= 0.0f && d23_2 <= 0.0f)
	{
		m_v2.a = 1.0f;
		m_count = 1;
		m_v1 = m_v2;
		return;
	}

	if (d13_1 <= 0.0f && d23_1 <= 0.0f)
	{
		m_v3.a = 1.0f;
		m_count = 1;
		m_v1 = m_v3;
		return;
	}

	if (d23_1 > 0.0f && d23_2 > 0.0f && d123_1 <= 0.0f)
	{
		float inv_d23 = 1.0f / (d23_1 + d23_2);
		m_v2.a = d23_1 * inv_d23;
		m_v3.a = d23_2 * inv_d23;
		m_count = 2;
		m_v1 = m_v3;
		return;
	}

	float inv_d123 = 1.0f / (d123_1 + d123_2 + d123_3);
	m_v1.a = d123_1 * inv_d123;
	m_v2.a = d123_2 * inv_d123;
	m_v3.a = d123_3 * inv_d123;
	m_count = 3;
}

void b2Distance(b2DistanceOutput* output,
				b2SimplexCache* cache,
				const b2DistanceInput* input)
{
	const b2DistanceProxy* proxyA = &input->proxyA;
	const b2DistanceProxy* proxyB = &input->proxyB;

	CFloatTransform transformA = input->transformA;
	CFloatTransform transformB = input->transformB;

	b2Simplex simplex;
	simplex.ReadCache(cache, proxyA, transformA, proxyB, transformB);

	b2SimplexVertex* vertices = &simplex.m_v1;
	const int k_maxIters = 20;

	int saveA[3], saveB[3];
	int saveCount = 0;

	CFloatVector2 closestPoint = simplex.GetClosestPoint();
	float distanceSqr1 = closestPoint.LengthSquared();
	float distanceSqr2 = distanceSqr1;

	int iter = 0;
	while (iter < k_maxIters)
	{
		saveCount = simplex.m_count;
		for (int i = 0; i < saveCount; ++i)
		{
			saveA[i] = vertices[i].indexA;
			saveB[i] = vertices[i].indexB;
		}

		switch (simplex.m_count)
		{
		case 1:
			break;

		case 2:
			simplex.Solve2();
			break;

		case 3:
			simplex.Solve3();
			break;

		default:
			_Assert(false);
		}

		if (simplex.m_count == 3)
		{
			break;
		}

		CFloatVector2 p = simplex.GetClosestPoint();
		distanceSqr2 = p.LengthSquared();

		if (distanceSqr2 >= distanceSqr1)
		{
			//break;
		}
		distanceSqr1 = distanceSqr2;

		CFloatVector2 d = simplex.GetSearchDirection();

		if (d.LengthSquared() < _Epsilon * _Epsilon)
		{
			break;
		}

		b2SimplexVertex* vertex = vertices + simplex.m_count;
		vertex->indexA = proxyA->GetSupport(b2MulT(transformA.q, -d));
		vertex->wA = b2Mul(transformA, proxyA->m_vertices[vertex->indexA]);
		CFloatVector2 wBLocal;
		vertex->indexB = proxyB->GetSupport(b2MulT(transformB.q, d));
		vertex->wB = b2Mul(transformB, proxyB->m_vertices[vertex->indexB]);
		vertex->w = vertex->wB - vertex->wA;

		++iter;

		bool duplicate = false;
		for (int i = 0; i < saveCount; ++i)
		{
			if (vertex->indexA == saveA[i] && vertex->indexB == saveB[i])
			{
				duplicate = true;
				break;
			}
		}

		if (duplicate)
		{
			break;
		}

		++simplex.m_count;
	}

	simplex.GetWitnessPoints(&output->pointA, &output->pointB);
	output->distance = b2Distance(output->pointA, output->pointB);
	output->iterations = iter;

	simplex.WriteCache(cache);

	if (input->useRadii)
	{
		float rA = 0.f;
		float rB = 0.f;

		if (output->distance > rA + rB && output->distance > _Epsilon)
		{
			output->distance -= rA + rB;
			CFloatVector2 normal = output->pointB - output->pointA;
			normal.Normalize();
			output->pointA += rA * normal;
			output->pointB -= rB * normal;
		}
		else
		{
			CFloatVector2 p = 0.5f * (output->pointA + output->pointB);
			output->pointA = p;
			output->pointB = p;
			output->distance = 0.0f;
		}
	}
}


bool ShapeOverlap( const CPolygonShape* shapeA, const CPolygonShape* shapeB, const CFloatTransform& xfA, const CFloatTransform& xfB )
{
	b2DistanceInput input;
	input.proxyA.Set(shapeA);
	input.proxyB.Set(shapeB);
	input.transformA = xfA;
	input.transformB = xfB;
	input.useRadii = true;

	b2SimplexCache cache;
	cache.count = 0;

	b2DistanceOutput output;

	b2Distance(&output, &cache, &input);

	return output.distance < 10.0f * _Epsilon;
}