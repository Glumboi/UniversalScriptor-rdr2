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

#define NATIVE_RESULT(T) (*(T*)nativeCall())

static inline UINT64 nativeArgFloat(float f)
{
	UINT64 v = 0;
	memcpy(&v, &f, sizeof(float));
	return v;
}
