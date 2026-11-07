
#ifndef GLUMITY_TYPE_HELPERS_H
#define GLUMITY_TYPE_HELPERS_H
// Combines two tokens together (needed for unique padding names)
#define COMBINE_INNER(a, b) a##b
#define COMBINE(a, b) COMBINE_INNER(a, b)

// Creates a padding array of a specific size
#define PAD(size) uint8_t COMBINE(pad_, __LINE__)[size]

// Creates padding from current position to a target offset
#define PAD_TO(current, target) uint8_t COMBINE(pad_, __LINE__)[target - current]
#endif