/*
	THIS FILE IS A PART OF RDR 2 SCRIPT HOOK SDK
				http://dev-c.com
			(C) Alexander Blade 2019
*/

#pragma once

#include <windows.h>
#include <math.h>
#include <stdint.h>
#include <stddef.h>

typedef uint64_t Any;
typedef uint64_t Void;
typedef int ScrHandle;

typedef int AnimScene;
typedef int Blip;
typedef int Cam;
typedef ScrHandle Entity;
typedef int FireId;
typedef unsigned int Hash;
typedef int Interior;
typedef ScrHandle ItemSet;
typedef ScrHandle Object;
typedef ScrHandle Ped;
typedef ScrHandle PersChar;
typedef int Pickup;
typedef unsigned int Player;
typedef int PopZone;
typedef int Prompt;
typedef int PropSet;
typedef ScrHandle Vehicle;
typedef ScrHandle Volume;

#if defined(_MSC_VER)
#define ALIGN8 __declspec(align(8))
#else
#define ALIGN8 __attribute__((aligned(8)))
#endif

typedef struct Vector2
{
	ALIGN8 float x;
	ALIGN8 float y;
} Vector2;

/* Vector3 (originally a C++ class by Halen84 (TuffyTown):
   https://github.com/Halen84/Vector3/blob/master/Vector3.h)
   Methods and operators are now free functions: Vector3_Add(a, b), Vector3_Length(v), ... */
#ifdef _MSC_VER
#pragma warning(disable:26495)
#endif
typedef struct Vector3
{
	union
	{
		struct
		{
			ALIGN8 float x;
			ALIGN8 float y;
			ALIGN8 float z;
		};
		struct
		{
			ALIGN8 float X;
			ALIGN8 float Y;
			ALIGN8 float Z;
		};
	};
} Vector3;
#ifdef _MSC_VER
#pragma warning(default:26495)
#endif

static inline Vector3 Vector3_Make(float x, float y, float z) { Vector3 v; v.x = x; v.y = y; v.z = z; return v; }
static inline Vector3 Vector3_FromScalar(float value) { return Vector3_Make(value, value, value); }
static inline Vector3 Vector3_Zero(void) { return Vector3_Make(0.0f, 0.0f, 0.0f); }

static inline int Vector3_NotEqual(Vector3 a, Vector3 b) { return a.x != b.x || a.y != b.y || a.z != b.z; }
static inline int Vector3_Equal(Vector3 a, Vector3 b) { return !Vector3_NotEqual(a, b); }

/* component-wise, vector (op) vector */
static inline Vector3 Vector3_Add(Vector3 a, Vector3 b) { return Vector3_Make(a.x + b.x, a.y + b.y, a.z + b.z); }
static inline Vector3 Vector3_Sub(Vector3 a, Vector3 b) { return Vector3_Make(a.x - b.x, a.y - b.y, a.z - b.z); }
static inline Vector3 Vector3_Mul(Vector3 a, Vector3 b) { return Vector3_Make(a.x * b.x, a.y * b.y, a.z * b.z); }
static inline Vector3 Vector3_Div(Vector3 a, Vector3 b) { return Vector3_Make(a.x / b.x, a.y / b.y, a.z / b.z); }

/* vector (op) scalar */
static inline Vector3 Vector3_AddF(Vector3 a, float s) { return Vector3_Make(a.x + s, a.y + s, a.z + s); }
static inline Vector3 Vector3_SubF(Vector3 a, float s) { return Vector3_Make(a.x - s, a.y - s, a.z - s); }
static inline Vector3 Vector3_MulF(Vector3 a, float s) { return Vector3_Make(a.x * s, a.y * s, a.z * s); }
static inline Vector3 Vector3_DivF(Vector3 a, float s) { return Vector3_Make(a.x / s, a.y / s, a.z / s); }

/* Get the cross product of two vectors */
static inline Vector3 Vector3_Cross(Vector3 a, Vector3 b)
{
	return Vector3_Make(a.y * b.z - a.z * b.y,
	                    a.z * b.x - a.x * b.z,
	                    a.x * b.y - a.y * b.x);
}

/* Get the length of the vector */
static inline float Vector3_Length(Vector3 v) { return sqrtf((v.x * v.x) + (v.y * v.y) + (v.z * v.z)); }
static inline float Vector3_Magnitude(Vector3 v) { return Vector3_Length(v); }

/* Get the dot product of two vectors */
static inline float Vector3_Dot(Vector3 a, Vector3 b) { return (a.x * b.x) + (a.y * b.y) + (a.z * b.z); }

/* Convert the vector to have a length of 1 */
static inline Vector3 Vector3_Normalize(Vector3 v)
{
	float length = Vector3_Length(v);
	float norm;
	if (length == 0.0f) return v;
	norm = 1.0f / length;
	return Vector3_Make(v.x * norm, v.y * norm, v.z * norm);
}

/* Linearly interpolate between two vectors by amount t */
static inline Vector3 Vector3_Lerp(Vector3 from, Vector3 to, float t)
{
	t = fmaxf(fminf(t, 1.0f), 0.0f);
	return Vector3_Make(from.x + (to.x - from.x) * t, from.y + (to.y - from.y) * t, from.z + (to.z - from.z) * t);
}

/* Reflects a vector off the plane defined by a normal */
static inline Vector3 Vector3_Reflect(Vector3 v, Vector3 normal)
{
	float d = Vector3_Dot(v, normal);
	return Vector3_Make(v.x - 2.0f * d * normal.x, v.y - 2.0f * d * normal.y, v.z - 2.0f * d * normal.z);
}

/* Spherically interpolate between two vectors by amount t */
static inline Vector3 Vector3_Slerp(Vector3 from, Vector3 to, float t)
{
	float dot, theta, sinTheta, w1, w2;
	t = fmaxf(fminf(t, 1.0f), 0.0f);
	dot = Vector3_Dot(from, to);
	theta = acosf(dot);
	sinTheta = sinf(theta);

	/* Vectors are parallel */
	if (sinTheta == 0.0f) {
		return Vector3_Lerp(from, to, t);
	}

	w1 = sinf((1.0f - t) * theta) / sinTheta;
	w2 = sinf(t * theta) / sinTheta;

	return Vector3_Make(from.x * w1 + to.x * w2, from.y * w1 + to.y * w2, from.z * w1 + to.z * w2);
}

/* Returns the distance between two vectors */
static inline float Vector3_Distance(Vector3 a, Vector3 b)
{
	float dx = a.x - b.x;
	float dy = a.y - b.y;
	float dz = a.z - b.z;
	return sqrtf(dx * dx + dy * dy + dz * dz);
}

typedef struct Vector4
{
	ALIGN8 float x;
	ALIGN8 float y;
	ALIGN8 float z;
	ALIGN8 float w;
} Vector4;
typedef struct NotificationStruct
{
	ALIGN8 int  Duration;
	ALIGN8 int  f1;
	ALIGN8 int  f2;
	ALIGN8 int  f3;
} NotificationStruct;
typedef struct NotificationStruct2
{
	ALIGN8 int  f0;
	ALIGN8 const char* Title;
	ALIGN8 const char* SubTitle;
	ALIGN8 int  f3;
	ALIGN8 Hash IconDict;
	ALIGN8 Hash Icon;
	ALIGN8 int  f6;
	ALIGN8 int  f7;
} NotificationStruct2;
typedef struct HelpTextArgs1
{
	ALIGN8 int p0;
	ALIGN8 int p1;
	ALIGN8 int p2;
	ALIGN8 int p3;
} HelpTextArgs1;
typedef struct HelpTextArgs2
{
	ALIGN8 int p0;
	ALIGN8 const char* p1;
} HelpTextArgs2;
typedef struct HelpTextArgs3
{
	ALIGN8 int p0;
	ALIGN8  const char* p1;
	ALIGN8  const char* p2;
	ALIGN8  int p3;
} HelpTextArgs3;
typedef struct HelpTextArgs4
{
	ALIGN8  int p0;
	ALIGN8  const char* p1;
	ALIGN8  const char* p2;
	ALIGN8  Hash p3;
	ALIGN8  int p4;
	ALIGN8  Hash p5;
	ALIGN8  int p6;
} HelpTextArgs4;
typedef struct FeedData
{
	ALIGN8 int duration; //how long to display the feed item for (milliseconds)
	ALIGN8 const char* f_1; //Used with UIFEED::0xAFF5BE9BA496CE40, seems to set the background colour, untested.
	ALIGN8 const char* f_2; //Used with UIFEED::0xAFF5BE9BA496CE40, seems to set the background colour, untested.
	ALIGN8 int f_3;
	ALIGN8 int f_4;  //Seems to be a struct for the UIFEED::_0xAFF5BE9BA496CE40 native. 
	ALIGN8 int f_5;
	ALIGN8 const char* secondary_subtitle; //Used with natives like UIFEED::_UI_FEED_POST_SAMPLE_TOAST. Displays another subtitle once the primary once has been displayed (from FeedInfo.subtitle).
	ALIGN8 int f_7;
	ALIGN8 int f_8;
	ALIGN8 int f_9;
	ALIGN8 int f_10;
	ALIGN8 int f_11;
	ALIGN8 int f_12;
} FeedData;
typedef struct FeedInfo
{
	ALIGN8 int f_0;
	ALIGN8 const char* title; //The title of the feed item.
	ALIGN8 const char* subtitle; //the main subtitle of the feed item.
	ALIGN8 const char* secondary_subtitle; //the subtitle that plays after the first subtitle.  Used with natives like UIFEED::_UI_FEED_POST_THREE_TEXT_SHARD
	ALIGN8 int texture_dictionary_hash; //the texture dict hash. Used with natives like UIFEED::_UI_FEED_POST_SAMPLE_TOAST
	ALIGN8 int texture_name_hash; //the texture name hash. Used with natives like UIFEED::_UI_FEED_POST_SAMPLE_TOAST
	ALIGN8 int f_6;
	ALIGN8 int f_7;
} FeedInfo;

//Researched by Halen84 (Tuffy) and Ked
typedef struct SampleToastRightStruct1
{
    ALIGN8 int Duration; //duration to display right toast (milliseconds).
    ALIGN8 const char* SoundSet; // soundset for the sound to play.
    ALIGN8 const char* SoundToPlay; // sound to play from soundset.
    ALIGN8 int f_3; // unk.
} SampleToastRightStruct1;
//Researched by Halen84 (Tuffy) and Ked
typedef struct SampleToastRightStruct2
{
    ALIGN8 int f_0; // unk.
    ALIGN8 const char* Title; // title or text that is displayed.
    ALIGN8 const char* ImageDictionary; //the image dictionary for the image texture that is displayed.
    ALIGN8 Hash ImageHash; // the image texture that is displayed.
    ALIGN8 int BounceAmount; //the amount of bounce affect applied to the toast when displayed.
    ALIGN8 Hash Color; //color of the text that is displayed.
    ALIGN8 int f_6; // unk.
} SampleToastRightStruct2;

typedef struct VolumeSphere
{
	float f_0;
	float f_1;
	float f_2;
	float s_0;
	float s_1;
	float s_2;
	float r_0;
	float r_1;
	float r_2;
} VolumeSphere;
typedef struct VectorH
{
	ALIGN8 float x;
	ALIGN8 float y;
	ALIGN8 float z;
	ALIGN8 float h;
} VectorH;
//struct UI_SCRIPT_EVENT
//{
//	ALIGN8 eUIScriptEventType eventType;
//	ALIGN8 int intParam;
//	ALIGN8 Hash hashParam;
//	ALIGN8 Hash datastoreParam;
//};

typedef struct ScriptedSpeechParams
{
	const char* speechName;
	const char* voiceName;
	ALIGN8 int variation;
	ALIGN8 Hash speechParamHash;
	ALIGN8 Ped listenerPed;
	ALIGN8 BOOL syncOverNetwork;
	ALIGN8 int v7;
} ScriptedSpeechParams;


typedef struct date_time
{
	int year;
	int PADDING1;
	int month;
	int PADDING2;
	int day;
	int PADDING3;
	int hour;
	int PADDING4;
	int minute;
	int PADDING5;
	int second;
	int PADDING6;
} date_time;

typedef struct StatId
{
	ALIGN8 Hash BaseId;
	ALIGN8 Hash PermutationId;
} StatId;
//struct g_lawData
//{
//	int _currentState = *getGlobalPtr(BASE + 4); // eLBS
//	Hash _currentLawRegion = *getGlobalPtr(BASE + 26);
//	float _lawSeeingRange = *getGlobalPtr(BASE + 69); // To be used with native LAW::_SET_LAW_SEEING_RANGE
//	Hash _mostRecentPursuitBountyState = *getGlobalPtr(BASE + 5);
//	Hash _currentCrime = *getGlobalPtr(BASE + 6);
//	int _currentBounty = *getGlobalPtr(BASE + 1381); // Seems to be inaccurate sometimes
//	int _currentRegion = *getGlobalPtr(BASE + 900);
//
//	struct wantedUIData
//	{
//		int eCurrentUIState = *getGlobalPtr(BASE + 78 + 59);
//	}wantedUIData;
//
//	// Global_1934266.f_78.f_62 = g_lawData.wantedUIData.??? (is a SET value for eCurrentUIState)
//	// Global_1934266.f_78.f_60 = g_lawData.wantedUIDate.??? (sub state)
//private:
//	static const int BASE = 1934266;
//};


// _Static_assert(sizeof(Vector2) == 16, "");
// _Static_assert(sizeof(Vector4) == 32, "");
// _Static_assert(sizeof(VectorH) == 32, "");

typedef struct RaycastResult
{
	int Result;
	BOOL DidHit;
	Entity HitEntity;
	Vector3 HitPosition;
	Vector3 SurfaceNormal;
	Hash MaterialHash;
} RaycastResult;

//TuffyTown (Halen84) Inventory Structs https://www.rdr2mods.com/forums/topic/2139-addingremoving-items-to-inventory-using-natives/

typedef struct sGuid
{
	ALIGN8 int data1;
	ALIGN8 int data2;
	ALIGN8 int data3;
	ALIGN8 int data4;
} sGuid;

typedef struct sSlotInfo
{
	ALIGN8 sGuid guid;
	ALIGN8 int f_1;
	ALIGN8 int f_2;
	ALIGN8 int f_3;
	ALIGN8 int slotId;
} sSlotInfo;

typedef struct sItemInfo
{
	ALIGN8 int f_0;
	ALIGN8 int f_1;
	ALIGN8 int f_2;
	ALIGN8 int f_3;
	ALIGN8 int f_4;
	ALIGN8 int f_5;
	ALIGN8 int f_6;
} sItemInfo;
