#pragma once

#include <DB/Core/Types.h>


/** Хэш функции, которые лучше чем тривиальная функция std::hash.
  * (при агрегации по идентификатору посетителя, прирост производительности более чем в 5 раз)
  */

/** Взято из MurmurHash.
  * Быстрее, чем intHash32 при вставке в хэш-таблицу UInt64 -> UInt64, где ключ - идентификатор посетителя.
  */
inline DB::UInt64 intHash64(DB::UInt64 x)
{
	x ^= x >> 33;
	x *= 0xff51afd7ed558ccdULL;
	x ^= x >> 33;
	x *= 0xc4ceb9fe1a85ec53ULL;
	x ^= x >> 33;

	return x;
}

/** CRC32C является не очень качественной в роли хэш функции,
  *  согласно avalanche и bit independence тестам, а также малым количеством бит,
  *  но может вести себя хорошо при использовании в хэш-таблицах,
  *  за счёт высокой скорости (latency 3 + 1 такт, througput 1 такт).
  * Работает только при поддержке SSE 4.2.
  * Используется asm вместо интринсика, чтобы не обязательно было собирать весь проект с -msse4.
  */
inline DB::UInt64 intHashCRC32(DB::UInt64 x)
{
	DB::UInt64 crc = -1ULL;
	asm("crc32q %[x], %[crc]\n" : [crc] "+r" (crc) : [x] "rm" (x));
	return crc;
}


template <typename T> struct DefaultHash;

template <typename T>
inline size_t DefaultHash64(T key)
{
	union
	{
		T in;
		DB::UInt64 out;
	} u;
	u.out = 0;
	u.in = key;
	return intHash64(u.out);
}

#define DEFINE_HASH(T) \
template <> struct DefaultHash<T>\
{\
	size_t operator() (T key) const\
	{\
		return DefaultHash64<T>(key);\
	}\
};

DEFINE_HASH(DB::UInt8)
DEFINE_HASH(DB::UInt16)
DEFINE_HASH(DB::UInt32)
DEFINE_HASH(DB::UInt64)
DEFINE_HASH(DB::Int8)
DEFINE_HASH(DB::Int16)
DEFINE_HASH(DB::Int32)
DEFINE_HASH(DB::Int64)
DEFINE_HASH(DB::Float32)
DEFINE_HASH(DB::Float64)

#undef DEFINE_HASH


template <typename T> struct HashCRC32;

template <typename T>
inline size_t hashCRC32(T key)
{
	union
	{
		T in;
		DB::UInt64 out;
	} u;
	u.out = 0;
	u.in = key;
	return intHashCRC32(u.out);
}

#define DEFINE_HASH(T) \
template <> struct HashCRC32<T>\
{\
	size_t operator() (T key) const\
	{\
		return hashCRC32<T>(key);\
	}\
};

DEFINE_HASH(DB::UInt8)
DEFINE_HASH(DB::UInt16)
DEFINE_HASH(DB::UInt32)
DEFINE_HASH(DB::UInt64)
DEFINE_HASH(DB::Int8)
DEFINE_HASH(DB::Int16)
DEFINE_HASH(DB::Int32)
DEFINE_HASH(DB::Int64)
DEFINE_HASH(DB::Float32)
DEFINE_HASH(DB::Float64)

#undef DEFINE_HASH
