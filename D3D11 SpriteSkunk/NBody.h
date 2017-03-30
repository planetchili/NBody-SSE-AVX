#pragma once
#include "Vec2.h"
#include "Sprite.h"
#include <vector>
#include <random>
#include <immintrin.h>

class NBody
{
public:
	NBody( int nBodies )
	{
		GenerateSystem( nBodies / 2,{ 350.0f,350.0f },80.0f,{ 0.0f,2.5f },3.0f );
		GenerateSystem( nBodies / 2,{ 850.0f,350.0f },80.0f,{ 0.0f,-2.5f },3.0f );
		mass = 5.0f;
		constGrav = 1.0f;
		maxAccel = 0.006f;
	}
	void Step()
	{
		const float g = constGrav * mass;
		for( auto i = bodies.begin(),end = bodies.end(); i < end; i++ )
		{
			for( auto j = std::next( i ); j < end; j++ )
			{
				i->CalculateAcceleration( *j,g );
			}
			i->Update();
		}
	}
	void Draw( const Sprite& s,DirectX::SpriteBatch& sb ) const
	{
		for( auto& b : bodies )
		{
			s.Draw( sb,b.GetPos() );
		}
	}
	static void SetMass( float mass )
	{
		NBody::mass = mass;
	}
	static void SetGrav( float grav )
	{
		constGrav = grav;
	}
	static void SetMaxAccel( float accel )
	{
		maxAccel = accel;
	}
	void GenerateSystem( int count,Vec2 center,float posDev,Vec2 avgVel,float velDev )
	{
		std::random_device rd;
		std::mt19937 rng( rd() );
		std::uniform_real_distribution<float> theta( 0.0f,3.14159f );
		std::normal_distribution<float> r( 0.0f,posDev );
		std::normal_distribution<float> vx( avgVel.x,velDev );
		std::normal_distribution<float> vy( avgVel.y,velDev );

		for( int i = 0; i < count; i++ )
		{
			const float rr = r( rng );
			const float t = theta( rng );
			const Vec2 pos = { rr * cos( t ),rr * sin( t ) };
			const Vec2 vel = { vx( rng ),vy( rng ) };
			bodies.emplace_back( pos + center,vel );
		}
	}
private:
	class Body
	{
	public:
		Body( Vec2 pos,Vec2 vel = { 0.0f,0.0f } )
			:
			pos( pos ),
			vel( vel ),
			tmpAccel( 0.0f,0.0f )
		{}
		inline void CalculateAcceleration( Body& other,float g )
		{
			const Vec2 delta = other.pos - pos;
			const float rSq = delta.LenSq();
			const Vec2 normal = delta / sqrt( rSq );
			const float force = min( g / rSq,maxAccel );
			Vec2 accel = normal * force;
			tmpAccel += accel;
			other.tmpAccel -= accel;
		}
		inline void Update()
		{
			vel += tmpAccel;
			pos += vel;
			tmpAccel = { 0.0f,0.0f };
		}
		inline Vec2 GetPos() const
		{
			return pos;
		}
	private:
		Vec2 pos;
		Vec2 vel;
		Vec2 tmpAccel;
	};
private:
	std::vector<Body> bodies;
	static float mass;
	static float constGrav;
	static float maxAccel;
};

class NBodySSE
{
public:
	NBodySSE( int nBodies )
		:
		nBodies( nBodies ),
		xPos( nullptr ),
		yPos( nullptr ),
		xVel( nullptr ),
		yVel( nullptr ),
		xAccel( nullptr ),
		yAccel( nullptr )
	{
		assert( nBodies % 4 == 0 && nBodies > 0 );
		xPos = reinterpret_cast<float*>( _aligned_malloc( nBodies*sizeof( float ),16 ) );
		yPos = reinterpret_cast<float*>( _aligned_malloc( nBodies*sizeof( float ),16 ) );
		xVel = reinterpret_cast<float*>( _aligned_malloc( nBodies*sizeof( float ),16 ) );
		yVel = reinterpret_cast<float*>( _aligned_malloc( nBodies*sizeof( float ),16 ) );
		xAccel = reinterpret_cast<float*>( _aligned_malloc( nBodies*sizeof( float ),16 ) );
		yAccel = reinterpret_cast<float*>( _aligned_malloc( nBodies*sizeof( float ),16 ) );
				
		CreateBodiesBinarySystem();

		mass = 5.0f;
		constGrav = 1.0f;
		maxAccel = 0.0055f;
	}
	~NBodySSE()
	{
		if( xPos )
		{
			_aligned_free( xPos );
			xPos = nullptr;
		}
		if( yPos )
		{
			_aligned_free( yPos );
			yPos = nullptr;
		}
		if( xVel )
		{
			_aligned_free( xVel );
			xVel = nullptr;
		}
		if( yVel )
		{
			_aligned_free( yVel );
			yVel = nullptr;
		}
		if( xAccel )
		{
			_aligned_free( xAccel );
			xAccel = nullptr;
		}
		if( yAccel )
		{
			_aligned_free( yAccel );
			yAccel = nullptr;
		}
	}
	void Draw( const Sprite& s,DirectX::SpriteBatch& sb ) const
	{
		for( int i = 0; i < nBodies; i++ )
		{
			s.Draw( sb,{ xPos[i],yPos[i] } );
		}
	}
	static void SetMass( float m )
	{
		mass = m;
	}
	static void SetGrav( float grav )
	{
		constGrav = grav;
	}
	static void SetMaxAccel( float accel )
	{
		maxAccel = accel;
	}
	void CreateBodiesDiffuse()
	{
		std::random_device rd;
		std::mt19937 rng( rd() );
		std::uniform_real_distribution<float> dx( 0.0f,1280.0f );
		std::uniform_real_distribution<float> dy( 0.0f,720.0f );

		for( int i = 0; i < nBodies; i++ )
		{
			xPos[i] = dx( rng );
			yPos[i] = dy( rng );
			xVel[i] = 0.0f;
			yVel[i] = 0.0f;
			xAccel[i] = 0.0f;
			yAccel[i] = 0.0f;
		}
	}
	void CreateBodiesBinarySystem()
	{
		int index = 0;
		std::random_device rd;
		GenerateSystem( index,nBodies/2,{ 350.0f,350.0f },80.0f,{ 0.0f,2.6f },3.0f,rd );
		GenerateSystem( index,nBodies/2,{ 850.0f,350.0f },80.0f,{ 0.0f,-2.6f },3.0f,rd );
	}
	void GenerateSystem( int& index,int count,Vec2 center,float posDev,Vec2 avgVel,float velDev,std::random_device& rd )
	{
		assert( index >= 0 );
		assert( count > 0 );
		assert( index + count <= nBodies );
		std::mt19937 rng( rd() );
		std::uniform_real_distribution<float> theta( 0.0f,3.14159f );
		std::normal_distribution<float> r( 0.0f,posDev );
		std::normal_distribution<float> vx( avgVel.x,velDev );
		std::normal_distribution<float> vy( avgVel.y,velDev );

		for( int end = index + count; index < end; index++ )
		{
			const float rr = r( rng );
			const float t = theta( rng );
			const Vec2 pos = { rr * cos( t ),rr * sin( t ) };
			const Vec2 vel = { vx( rng ),vy( rng ) };
			xPos[index] = pos.x + center.x;
			yPos[index] = pos.y + center.y;
			xVel[index] = vel.x;
			yVel[index] = vel.y;
			xAccel[index] = 0.0f;
			yAccel[index] = 0.0f;
		}
	}
	#pragma region intrablockSSEroutine
#define SSEIntrablockIteration()\
	{\
		const __m128 xDelta = _mm_sub_ps( jxPos,ixPos );\
		const __m128 yDelta = _mm_sub_ps( jyPos,iyPos );\
		const __m128 rSq = _mm_add_ps( _mm_mul_ps( xDelta,xDelta ),_mm_mul_ps( yDelta,yDelta ) );\
		const __m128 rRcp = _mm_rsqrt_ps( rSq );\
		const __m128 xNorm = _mm_mul_ps( xDelta,rRcp );\
		const __m128 yNorm = _mm_mul_ps( yDelta,rRcp );\
		const __m128 rSqRcp = _mm_rcp_ps( rSq );\
		const __m128 accelMag = _mm_min_ps( _mm_mul_ps( g,rSqRcp ),maxAccel );\
		const __m128 xAccel = _mm_mul_ps( xNorm,accelMag );\
		const __m128 yAccel = _mm_mul_ps( yNorm,accelMag );\
		ixAccel = _mm_add_ps( ixAccel,xAccel );\
		iyAccel = _mm_add_ps( iyAccel,yAccel );\
		jxAccel = _mm_sub_ps( jxAccel,xAccel );\
		jyAccel = _mm_sub_ps( jyAccel,yAccel );\
		jxPos = _mm_shuffle_ps( jxPos,jxPos,_MM_SHUFFLE( 0,3,2,1 ) );\
		jyPos = _mm_shuffle_ps( jyPos,jyPos,_MM_SHUFFLE( 0,3,2,1 ) );\
		jxAccel = _mm_shuffle_ps( jxAccel,jxAccel,_MM_SHUFFLE( 0,3,2,1 ) );\
		jyAccel = _mm_shuffle_ps( jyAccel,jyAccel,_MM_SHUFFLE( 0,3,2,1 ) );\
	}
	#pragma endregion
	#pragma region intrablockSSEroutine_selfblockfirst
#define SSESelfIntrablockFirstIteration()\
		{\
		jxPos = _mm_shuffle_ps( jxPos,jxPos,_MM_SHUFFLE( 0,3,2,1 ) );\
		jyPos = _mm_shuffle_ps( jyPos,jyPos,_MM_SHUFFLE( 0,3,2,1 ) );\
		jxAccel = _mm_shuffle_ps( jxAccel,jxAccel,_MM_SHUFFLE( 0,3,2,1 ) );\
		jyAccel = _mm_shuffle_ps( jyAccel,jyAccel,_MM_SHUFFLE( 0,3,2,1 ) );\
		const __m128 xDelta = _mm_sub_ps( jxPos,ixPos );\
		const __m128 yDelta = _mm_sub_ps( jyPos,iyPos );\
		const __m128 rSq = _mm_add_ps( _mm_mul_ps( xDelta,xDelta ),_mm_mul_ps( yDelta,yDelta ) );\
		const __m128 rRcp = _mm_rsqrt_ps( rSq );\
		const __m128 xNorm = _mm_mul_ps( xDelta,rRcp );\
		const __m128 yNorm = _mm_mul_ps( yDelta,rRcp );\
		const __m128 rSqRcp = _mm_rcp_ps( rSq );\
		const __m128 accelMag = _mm_min_ps( _mm_mul_ps( g,rSqRcp ),maxAccel );\
		const __m128 xAccel = _mm_mul_ps( xNorm,accelMag );\
		const __m128 yAccel = _mm_mul_ps( yNorm,accelMag );\
		ixAccel = _mm_add_ps( ixAccel,xAccel );\
		iyAccel = _mm_add_ps( iyAccel,yAccel );\
		jxAccel = _mm_sub_ps( jxAccel,xAccel );\
		jyAccel = _mm_sub_ps( jyAccel,yAccel );\
		jxPos = _mm_shuffle_ps( jxPos,jxPos,_MM_SHUFFLE( 0,3,2,1 ) );\
		jyPos = _mm_shuffle_ps( jyPos,jyPos,_MM_SHUFFLE( 0,3,2,1 ) );\
		jxAccel = _mm_shuffle_ps( jxAccel,jxAccel,_MM_SHUFFLE( 0,3,2,1 ) );\
		jyAccel = _mm_shuffle_ps( jyAccel,jyAccel,_MM_SHUFFLE( 0,3,2,1 ) );\
		}
	#pragma endregion
	inline void Step()
	{
		const float* const pixPosEnd = &xPos[nBodies];
		float* pixPos = xPos;
		float* piyPos = yPos;
		float* pixVel = xVel;
		float* piyVel = yVel;
		float* pixAccel = xAccel;
		float* piyAccel = yAccel;

		const __m128 g = _mm_set1_ps( NBodySSE::constGrav * NBodySSE::mass );
		const __m128 maxAccel = _mm_set1_ps( NBodySSE::maxAccel );

		for( ; pixPos < pixPosEnd; 
			pixPos += 4,piyPos += 4,pixVel += 4,piyVel += 4,pixAccel += 4,piyAccel += 4 )
		{
			__m128 ixPos = _mm_load_ps( pixPos );
			__m128 iyPos = _mm_load_ps( piyPos );
			__m128 ixAccel = _mm_load_ps( pixAccel );
			__m128 iyAccel = _mm_load_ps( piyAccel );
			_mm_store_ps( pixAccel,_mm_setzero_ps() );
			_mm_store_ps( piyAccel,_mm_setzero_ps() );

			const float* pjxPos = pixPos;
			const float* pjyPos = piyPos;
			float* pjxAccel = pixAccel;
			float* pjyAccel = piyAccel;

			{
				__m128 jxPos = _mm_load_ps( pjxPos );
				__m128 jyPos = _mm_load_ps( pjyPos );
				__m128 jxAccel = _mm_load_ps( pjxAccel );
				__m128 jyAccel = _mm_load_ps( pjyAccel );

				SSESelfIntrablockFirstIteration();
				SSEIntrablockIteration();
				SSEIntrablockIteration();

				_mm_store_ps( pjxAccel,jxAccel );
				_mm_store_ps( pjyAccel,jyAccel );

				pjxPos += 4,pjyPos += 4,pjxAccel += 4,pjyAccel += 4;
			}

			for( ; pjxPos < pixPosEnd; 
				pjxPos += 4,pjyPos += 4,pjxAccel += 4,pjyAccel += 4 )
			{
				__m128 jxPos = _mm_load_ps( pjxPos );
				__m128 jyPos = _mm_load_ps( pjyPos );
				__m128 jxAccel = _mm_load_ps( pjxAccel );
				__m128 jyAccel = _mm_load_ps( pjyAccel );

				SSEIntrablockIteration();
				SSEIntrablockIteration();
				SSEIntrablockIteration();
				SSEIntrablockIteration();

				_mm_store_ps( pjxAccel,jxAccel );
				_mm_store_ps( pjyAccel,jyAccel );
			}

			__m128 ixVel = _mm_load_ps( pixVel );
			__m128 iyVel = _mm_load_ps( piyVel );
			ixVel = _mm_add_ps( ixVel,ixAccel );
			iyVel = _mm_add_ps( iyVel,iyAccel );
			ixPos = _mm_add_ps( ixPos,ixVel );
			iyPos = _mm_add_ps( iyPos,iyVel );
			_mm_store_ps( pixVel,ixVel );
			_mm_store_ps( piyVel,iyVel );
			_mm_store_ps( pixPos,ixPos );
			_mm_store_ps( piyPos,iyPos );
		}
	}
private:
	const int nBodies;
	float* xPos;
	float* yPos;
	float* xVel;
	float* yVel;
	float* xAccel;
	float* yAccel;
	static float mass;
	static float constGrav;
	static float maxAccel;
};

class NBodyAVX
{
public:
	NBodyAVX( int nBodies )
		:
		nBodies( nBodies ),
		xPos( nullptr ),
		yPos( nullptr ),
		xVel( nullptr ),
		yVel( nullptr ),
		xAccel( nullptr ),
		yAccel( nullptr )
	{
		assert( nBodies % 8 == 0 && nBodies > 0 );
		xPos = reinterpret_cast<float*>( _aligned_malloc( nBodies*sizeof( float ),32 ) );
		yPos = reinterpret_cast<float*>( _aligned_malloc( nBodies*sizeof( float ),32 ) );
		xVel = reinterpret_cast<float*>( _aligned_malloc( nBodies*sizeof( float ),32 ) );
		yVel = reinterpret_cast<float*>( _aligned_malloc( nBodies*sizeof( float ),32 ) );
		xAccel = reinterpret_cast<float*>( _aligned_malloc( nBodies*sizeof( float ),32 ) );
		yAccel = reinterpret_cast<float*>( _aligned_malloc( nBodies*sizeof( float ),32 ) );

		CreateBodiesBinarySystem();

		mass = 5.0f;
		constGrav = 1.0f;
		maxAccel = 0.0055f;
	}
	~NBodyAVX()
	{
		if( xPos )
		{
			_aligned_free( xPos );
			xPos = nullptr;
		}
		if( yPos )
		{
			_aligned_free( yPos );
			yPos = nullptr;
		}
		if( xVel )
		{
			_aligned_free( xVel );
			xVel = nullptr;
		}
		if( yVel )
		{
			_aligned_free( yVel );
			yVel = nullptr;
		}
		if( xAccel )
		{
			_aligned_free( xAccel );
			xAccel = nullptr;
		}
		if( yAccel )
		{
			_aligned_free( yAccel );
			yAccel = nullptr;
		}
	}
	void Draw( const Sprite& s,DirectX::SpriteBatch& sb ) const
	{
		for( int i = 0; i < nBodies; i++ )
		{
			s.Draw( sb,{ xPos[i],yPos[i] } );
		}
	}
	static void SetMass( float m )
	{
		mass = m;
	}
	static void SetGrav( float grav )
	{
		constGrav = grav;
	}
	static void SetMaxAccel( float accel )
	{
		maxAccel = accel;
	}
	void CreateBodiesDiffuse()
	{
		std::random_device rd;
		std::mt19937 rng( rd() );
		std::uniform_real_distribution<float> dx( 0.0f,1280.0f );
		std::uniform_real_distribution<float> dy( 0.0f,720.0f );

		for( int i = 0; i < nBodies; i++ )
		{
			xPos[i] = dx( rng );
			yPos[i] = dy( rng );
			xVel[i] = 0.0f;
			yVel[i] = 0.0f;
			xAccel[i] = 0.0f;
			yAccel[i] = 0.0f;
		}
	}
	void CreateBodiesBinarySystem()
	{
		int index = 0;
		std::random_device rd;
		GenerateSystem( index,nBodies / 2,{ 350.0f,350.0f },80.0f,{ 0.0f,2.5f },2.1f,rd );
		GenerateSystem( index,nBodies / 2,{ 850.0f,350.0f },80.0f,{ 0.0f,-2.5f },2.1f,rd );
	}
	void CreateBodiesBinarySystemAsymmetric()
	{
		int index = 0;
		std::random_device rd;
		GenerateSystem( index,nBodies - 40,{ 640.0f,360.0f },60.0f,{ 0.0f,0.0f },4.1f,rd );
		GenerateSystem( index,40,{ 640.0f,20.0f },10.0f,{ 5.5f,0.0f },1.1f,rd );
	}
	void GenerateSystem( int& index,int count,Vec2 center,float posDev,Vec2 avgVel,float velDev,std::random_device& rd )
	{
		assert( index >= 0 );
		assert( count > 0 );
		assert( index + count <= nBodies );
		std::mt19937 rng( rd() );
		std::uniform_real_distribution<float> theta( 0.0f,3.14159f );
		std::normal_distribution<float> r( 0.0f,posDev );
		std::normal_distribution<float> vx( avgVel.x,velDev );
		std::normal_distribution<float> vy( avgVel.y,velDev );

		for( int end = index + count; index < end; index++ )
		{
			const float rr = r( rng );
			const float t = theta( rng );
			const Vec2 pos = { rr * cos( t ),rr * sin( t ) };
			const Vec2 vel = { vx( rng ),vy( rng ) };
			xPos[index] = pos.x + center.x;
			yPos[index] = pos.y + center.y;
			xVel[index] = vel.x;
			yVel[index] = vel.y;
			xAccel[index] = 0.0f;
			yAccel[index] = 0.0f;
		}
	}
	#pragma region intrablockAVXroutine
#define AVXIntrablockIteration()\
	{\
		const __m256 xDelta = _mm256_sub_ps( jxPos,ixPos );\
		const __m256 yDelta = _mm256_sub_ps( jyPos,iyPos );\
		const __m256 rSq = _mm256_add_ps( _mm256_mul_ps( xDelta,xDelta ),_mm256_mul_ps( yDelta,yDelta ) );\
		const __m256 rRcp = _mm256_rsqrt_ps( rSq );\
		const __m256 xNorm = _mm256_mul_ps( xDelta,rRcp );\
		const __m256 yNorm = _mm256_mul_ps( yDelta,rRcp );\
		const __m256 rSqRcp = _mm256_rcp_ps( rSq );\
		const __m256 accelMag = _mm256_min_ps( _mm256_mul_ps( g,rSqRcp ),maxAccel );\
		const __m256 xAccel = _mm256_mul_ps( xNorm,accelMag );\
		const __m256 yAccel = _mm256_mul_ps( yNorm,accelMag );\
		ixAccel = _mm256_add_ps( ixAccel,xAccel );\
		iyAccel = _mm256_add_ps( iyAccel,yAccel );\
		jxAccel = _mm256_sub_ps( jxAccel,xAccel );\
		jyAccel = _mm256_sub_ps( jyAccel,yAccel );\
		jxPos = _mm256_permutevar8x32_ps( jxPos,shuffleMask );\
		jyPos = _mm256_permutevar8x32_ps( jyPos,shuffleMask );\
		jxAccel = _mm256_permutevar8x32_ps( jxAccel,shuffleMask );\
		jyAccel = _mm256_permutevar8x32_ps( jyAccel,shuffleMask );\
	}
#pragma endregion
	#pragma region intrablockAVXroutine_selfblockfirst
#define AVXSelfIntrablockFirstIteration()\
	{\
		jxPos = _mm256_permutevar8x32_ps( jxPos,shuffleMask );\
		jyPos = _mm256_permutevar8x32_ps( jyPos,shuffleMask );\
		jxAccel = _mm256_permutevar8x32_ps( jxAccel,shuffleMask );\
		jyAccel = _mm256_permutevar8x32_ps( jyAccel,shuffleMask );\
		const __m256 xDelta = _mm256_sub_ps( jxPos,ixPos );\
		const __m256 yDelta = _mm256_sub_ps( jyPos,iyPos );\
		const __m256 rSq = _mm256_add_ps( _mm256_mul_ps( xDelta,xDelta ),_mm256_mul_ps( yDelta,yDelta ) );\
		const __m256 rRcp = _mm256_rsqrt_ps( rSq );\
		const __m256 xNorm = _mm256_mul_ps( xDelta,rRcp );\
		const __m256 yNorm = _mm256_mul_ps( yDelta,rRcp );\
		const __m256 rSqRcp = _mm256_rcp_ps( rSq );\
		const __m256 accelMag = _mm256_min_ps( _mm256_mul_ps( g,rSqRcp ),maxAccel );\
		const __m256 xAccel = _mm256_mul_ps( xNorm,accelMag );\
		const __m256 yAccel = _mm256_mul_ps( yNorm,accelMag );\
		ixAccel = _mm256_add_ps( ixAccel,xAccel );\
		iyAccel = _mm256_add_ps( iyAccel,yAccel );\
		jxAccel = _mm256_sub_ps( jxAccel,xAccel );\
		jyAccel = _mm256_sub_ps( jyAccel,yAccel );\
		jxPos = _mm256_permutevar8x32_ps( jxPos,shuffleMask );\
		jyPos = _mm256_permutevar8x32_ps( jyPos,shuffleMask );\
		jxAccel = _mm256_permutevar8x32_ps( jxAccel,shuffleMask );\
		jyAccel = _mm256_permutevar8x32_ps( jyAccel,shuffleMask );\
	}
#pragma endregion
	void Step()
	{
		const float* const pixPosEnd = &xPos[nBodies];
		float* pixPos = xPos;
		float* piyPos = yPos;
		float* pixVel = xVel;
		float* piyVel = yVel;
		float* pixAccel = xAccel;
		float* piyAccel = yAccel;

		const __m256 g = _mm256_set1_ps( NBodyAVX::constGrav * NBodyAVX::mass );
		const __m256 maxAccel = _mm256_set1_ps( NBodyAVX::maxAccel );
		const __m256i shuffleMask = _mm256_set_epi32( 6,5,4,3,2,1,0,7 );

		for( ; pixPos < pixPosEnd;
			pixPos += 8,piyPos += 8,pixVel += 8,piyVel += 8,pixAccel += 8,piyAccel += 8 )
		{
			__m256 ixPos = _mm256_load_ps( pixPos );
			__m256 iyPos = _mm256_load_ps( piyPos );
			__m256 ixAccel = _mm256_load_ps( pixAccel );
			__m256 iyAccel = _mm256_load_ps( piyAccel );
			_mm256_store_ps( pixAccel,_mm256_setzero_ps() );
			_mm256_store_ps( piyAccel,_mm256_setzero_ps() );

			const float* pjxPos = pixPos;
			const float* pjyPos = piyPos;
			float* pjxAccel = pixAccel;
			float* pjyAccel = piyAccel;

			{
				__m256 jxPos = _mm256_load_ps( pjxPos );
				__m256 jyPos = _mm256_load_ps( pjyPos );
				__m256 jxAccel = _mm256_load_ps( pjxAccel );
				__m256 jyAccel = _mm256_load_ps( pjyAccel );

				AVXSelfIntrablockFirstIteration();
				AVXIntrablockIteration();
				AVXIntrablockIteration();
				AVXIntrablockIteration();
				AVXIntrablockIteration();
				AVXIntrablockIteration();
				AVXIntrablockIteration();

				_mm256_store_ps( pjxAccel,jxAccel );
				_mm256_store_ps( pjyAccel,jyAccel );

				pjxPos += 8,pjyPos += 8,pjxAccel += 8,pjyAccel += 8;
			}

			for( ; pjxPos < pixPosEnd;
				pjxPos += 8,pjyPos += 8,pjxAccel += 8,pjyAccel += 8 )
			{
				__m256 jxPos = _mm256_load_ps( pjxPos );
				__m256 jyPos = _mm256_load_ps( pjyPos );
				__m256 jxAccel = _mm256_load_ps( pjxAccel );
				__m256 jyAccel = _mm256_load_ps( pjyAccel );

				AVXIntrablockIteration();
				AVXIntrablockIteration();
				AVXIntrablockIteration();
				AVXIntrablockIteration();
				AVXIntrablockIteration();
				AVXIntrablockIteration();
				AVXIntrablockIteration();
				AVXIntrablockIteration();

				_mm256_store_ps( pjxAccel,jxAccel );
				_mm256_store_ps( pjyAccel,jyAccel );
			}

			__m256 ixVel = _mm256_load_ps( pixVel );
			__m256 iyVel = _mm256_load_ps( piyVel );
			ixVel = _mm256_add_ps( ixVel,ixAccel );
			iyVel = _mm256_add_ps( iyVel,iyAccel );
			ixPos = _mm256_add_ps( ixPos,ixVel );
			iyPos = _mm256_add_ps( iyPos,iyVel );
			_mm256_store_ps( pixVel,ixVel );
			_mm256_store_ps( piyVel,iyVel );
			_mm256_store_ps( pixPos,ixPos );
			_mm256_store_ps( piyPos,iyPos );
		}
	}
private:
	const int nBodies;
	float* xPos;
	float* yPos;
	float* xVel;
	float* yVel;
	float* xAccel;
	float* yAccel;
	static float mass;
	static float constGrav;
	static float maxAccel;
};