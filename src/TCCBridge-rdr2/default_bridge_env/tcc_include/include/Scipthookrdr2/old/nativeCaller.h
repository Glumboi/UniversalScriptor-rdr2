/*
	THIS FILE IS A PART OF RDR 2 SCRIPT HOOK SDK
				http://dev-c.com
			(C) Alexander Blade 2019

	C port: the C++ templates (nativePush<T>, pushArgs, invoke<R>) are replaced by
	type-specific push functions. natives.h generates the right calls for every native.
*/

#pragma once

#include <stdint.h>
#include <string.h>
#include "main.h"

/* Every argument occupies one 64-bit slot. 32-bit values are zero-extended and
   floats are stored in the low 32 bits, exactly like the C++ nativePush<T>. */

static inline void nativePushU64(UINT64 val) { nativePush64(val); }

static inline void nativePushU32(uint32_t val)
{
	nativePush64((UINT64)val);
}

static inline void nativePushFloat(float val)
{
	UINT64 val64 = 0;
	memcpy(&val64, &val, sizeof(float));
	nativePush64(val64);
}

static inline void nativePushPtr(const void* val)
{
	nativePush64((UINT64)(uintptr_t)val);
}

/* Convenience for hand-written calls: nativePush(x) picks the right function.
   Plain int/unsigned values are pushed as 32-bit; use nativePushU64 for 64-bit ones. */
#define nativePush(x) _Generic((x),                          \
	float:               nativePushFloat,                    \
	double:              nativePushFloat,                    \
	int:                 nativePushU32,                      \
	unsigned int:        nativePushU32,                      \
	long:                nativePushU32,                      \
	unsigned long:       nativePushU32,                      \
	long long:           nativePushU64,                      \
	unsigned long long:  nativePushU64,                      \
	default:             nativePushPtr)(x)

/* Read the result of nativeCall() as type T:  float f = NATIVE_RESULT(float); */
#define NATIVE_RESULT(T) (*(T*)nativeCall())

/* Float bit pattern as a 64-bit value, for the variadic natives in natives.h */
static inline UINT64 nativeArgFloat(float f)
{
	UINT64 v = 0;
	memcpy(&v, &f, sizeof(float));
	return v;
}
