//====== Copyright 1996-2012, Valve Corporation, All rights reserved. ======//
//
// Endianness handling.
//
//==========================================================================//

#ifndef MINBASE_ENDIAN_H
#define MINBASE_ENDIAN_H
#pragma once

#include "miniutl.h"

//-----------------------------------------------------------------------------
// Purpose: Standard functions for handling endian-ness
//-----------------------------------------------------------------------------

//-------------------------------------
// Basic swaps
//-------------------------------------

template <typename T>
inline T WordSwapC( T w )
{
	COMPILE_TIME_ASSERT( sizeof(T) == sizeof(uint16) );
	uint16 temp;
#if defined( __ICC )
	temp = _byteswap_ushort( *(uint16*)&w );
#else
	// This translates into a single rotate on x86/x64
	temp = *(uint16 *)&w;
	temp = (temp << 8) | (temp >> 8);
#endif
	return *((T*)&temp);
}

template <typename T>
inline T DWordSwapC( T dw )
{
	COMPILE_TIME_ASSERT( sizeof( T ) == sizeof(uint32) );
	uint32 temp;
#if defined( __ICC )
	temp = _byteswap_ulong( *(uint32*)&dw );
#elif defined( __clang__ ) || GCC_VERSION >= 40300
	temp = __builtin_bswap32( *(uint32*)&dw );
#else
	temp =    *((uint32 *)&dw) >> 24;
	temp |= ((*((uint32 *)&dw) & 0x00FF0000) >> 8);
	temp |= ((*((uint32 *)&dw) & 0x0000FF00) << 8);
	temp |=   *((uint32 *)&dw) << 24;
#endif
   return *((T*)&temp);
}

template <typename T>
inline T QWordSwapC( T dw )
{
	COMPILE_TIME_ASSERT( sizeof( dw ) == sizeof(uint64) );
	uint64 temp;
#if defined( __ICC )
	temp = _byteswap_uint64( *(uint64*)&dw );
#elif defined( __clang__ ) || GCC_VERSION >= 40300
	temp = __builtin_bswap64( *(uint64*)&dw );
#else
	temp = (uint64)DWordSwapC( (uint32)( ( *(uint64*)&dw ) >> 32 ) );
	temp |= (uint64)DWordSwapC( (uint32)( *(uint64*)&dw ) ) << 32;
#endif
	return *((T*)&temp);
}

#define WordSwap  WordSwapC
#define DWordSwap DWordSwapC
#define QWordSwap QWordSwapC

#if defined _WIN32 || defined __DOS__
#define __LITTLE_ENDIAN 4321
#define __BIG_ENDIAN 1234
#define __BYTE_ORDER __LITTLE_ENDIAN //!!!
#endif

#if !defined(__BYTE_ORDER) && !defined(__LITTLE_ENDIAN) && !defined(__BIG_ENDIAN)
#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && defined(__ORDER_LITTLE_ENDIAN__) // some compilers define this
#define __BYTE_ORDER __BYTE_ORDER__
#define __LITTLE_ENDIAN __ORDER_LITTLE_ENDIAN__
#define __BIG_ENDIAN __ORDER_BIG_ENDIAN__
#else
#error
#endif
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN

	#define BigInt16( val )		WordSwap( val )
	#define BigWord( val )		WordSwap( val )
	#define BigInt32( val )		DWordSwap( val )
	#define BigDWord( val )		DWordSwap( val )
	#define BigQWord( val )		QWordSwap( val ) 
	#define BigFloat( val )		DWordSwap( val )
	#define LittleInt16( val )	( val )
	#define LittleWord( val )	( val )
	#define LittleInt32( val )	( val )
	#define LittleDWord( val )	( val )
	#define LittleQWord( val )	( val )

	#define LittleFloat( val )	( val )

#elif __BYTE_ORDER == __BIG_ENDIAN

	#define BigInt16( val )		( val )
	#define BigWord( val )		( val )
	#define BigInt32( val )		( val )
	#define BigDWord( val )		( val )
	#define BigQWord( val )		( val )
	#define BigFloat( val )		( val )
	#define LittleInt16( val )	WordSwap( val )
	#define LittleWord( val )	WordSwap( val )
	#define LittleInt32( val )	DWordSwap( val )
	#define LittleDWord( val )	DWordSwap( val )
	#define LittleQWord( val )	QWordSwap( val )
	#define LittleFloat( val )	DWordSwap( val )

#else

	// @Note (toml 05-02-02): this technique expects the compiler to 
	// optimize the expression and eliminate the other path. On any new 
	// platform/compiler this should be tested.
	inline short BigInt16( int16 val )		{ int test = 1; return ( *(char *)&test == 1 ) ? WordSwap( val )  : val; }
	inline uint16 BigWord( uint16 val )		{ int test = 1; return ( *(char *)&test == 1 ) ? WordSwap( val )  : val; }
	inline int32 BigInt32( int32 val )		{ int test = 1; return ( *(char *)&test == 1 ) ? DWordSwap( val ) : val; }
	inline uint32 BigDWord( uint32 val )	{ int test = 1; return ( *(char *)&test == 1 ) ? DWordSwap( val ) : val; }
	inline uint64 BigQWord( uint64 val )	{ int test = 1; return ( *(char *)&test == 1 ) ? QWordSwap( val ) : val; }
	inline float BigFloat( float val )		{ int test = 1; return ( *(char *)&test == 1 ) ? DWordSwap( val ) : val; }
	inline short LittleInt16( int16 val )	{ int test = 1; return ( *(char *)&test == 1 ) ? val : WordSwap( val ); }
	inline uint16 LittleWord( uint16 val )	{ int test = 1; return ( *(char *)&test == 1 ) ? val : WordSwap( val ); }
    inline long LittleInt32( int32 val )		{ int test = 1; return ( *(char *)&test == 1 ) ? val : DWordSwap( val ); }
	inline uint32 LittleDWord( uint32 val )	{ int test = 1; return ( *(char *)&test == 1 ) ? val : DWordSwap( val ); }
	inline uint64 LittleQWord( uint64 val )	{ int test = 1; return ( *(char *)&test == 1 ) ? val : QWordSwap( val ); }
	inline float LittleFloat( float val )	{ int test = 1; return ( *(char *)&test == 1 ) ? val : DWordSwap( val ); }

#endif

#if __BYTE_ORDER == __BIG_ENDIAN
	#if defined ( _PS3 ) || defined ( __ppc64__ )
		inline uint32 LoadLittleDWord( uint32 *base, unsigned int dwordIndex )
		{
			return __lwbrx( base + dwordIndex );
		}

		inline void StoreLittleDWord( uint32 *base, unsigned int dwordIndex, uint32 dword )
		{
			__stwbrx( base + dwordIndex, dword );
		}
		inline uint64 LoadLittleInt64( uint64 *base, unsigned int nWordIndex )
		{
			return __ldbrx( base + nWordIndex );
		}

		inline void StoreLittleInt64( uint64 *base, unsigned int nWordIndex, uint64 nWord )
		{
			__stdbrx( base + nWordIndex, nWord );
		}
	#elif defined ( __ppc__ ) || defined ( __PPC__ )
		inline uint32 LoadLittleDWord( uint32 *base, unsigned int dwordIndex )
		{
			uint32 v;
			__asm__ ("lwbrx   %0, %y1" : "=r"(v) : "Z"(*(const uint32*)( base + dwordIndex )));
			return v;
		}

		inline void StoreLittleDWord( uint32 *base, unsigned int dwordIndex, uint32 dword )
		{
			__asm__ ("stwbrx  %1, %y0" : "=Z"(*(uint32*)( base + dwordIndex )) : "r"(dword));
		}
		
		inline uint64 LoadLittleInt64( uint64 *base, unsigned int nWordIndex )
  		{
 			union { uint64 v; uint32 hl[2]; } v;
			__asm__ ("lwbrx   %0, %y2  \n\t"
					 "lwbrx   %1, %y3  \n\t"
					 : "=&r"(v.hl[1]), "=r"(v.hl[0])
					 : "Z"(*(const uint32*)( base + nWordIndex )), "Z"(*((const uint32*)( base + nWordIndex + 1 ))));
			return v.v;
		}

		inline void StoreLittleInt64( uint64 *base, unsigned int nWordIndex, uint64 nWord )
		{
			union { uint64 v; uint32 hl[2]; } vv = { nWord };
			__asm__ ("stwbrx  %2, %y0  \n\t"
					 "stwbrx  %3, %y1  \n\t"
					 : "=Z"(*(uint32*)( base + nWordIndex )), "=Z"(*((uint32*)( base + nWordIndex + 1 )))
					 : "r"(vv.hl[1]), "r"(vv.hl[0]));
		}
	#else
		inline uint32 LoadLittleDWord( uint32 *base, unsigned int dwordIndex )
		{
			return __loadwordbytereverse( dwordIndex<<2, base );
		}

		inline void StoreLittleDWord( uint32 *base, unsigned int dwordIndex, uint32 dword )
		{
			__storewordbytereverse( dword, dwordIndex<<2, base );
		}
		inline uint64 LoadLittleInt64( uint64 *base, unsigned int nWordIndex )
		{
			return __loaddoublewordbytereverse( nWordIndex<<2, base );
		}

		inline void StoreLittleInt64( uint64 *base, unsigned int nWordIndex, uint64 nWord )
		{
			__storedoublewordbytereverse( nWord, nWordIndex<<2, base );
		}
	#endif
#else
	inline uint32 LoadLittleDWord( uint32 *base, unsigned int dwordIndex )
	{
		return LittleDWord( base[dwordIndex] );
	}

	inline void StoreLittleDWord( uint32 *base, unsigned int dwordIndex, uint32 dword )
	{
		base[dwordIndex] = LittleDWord(dword);
	}
#endif

#endif // #ifndef MINBASE_ENDIAN_H
