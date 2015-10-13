/*
 * Author Tim Diels <timdiels.m@gmail.com>
 *
 * From: http://www.codeproject.com/Tips/884269/Generating-Unique-Contrasting-Colors-in-VB-NET
 * By Gregory Morse
 *
 * Translated to C++
 *
 * TODO CPOL license
 */

#pragma once


namespace DEEP_BLUE_GENOME {
namespace UTIL {

struct RGBColor {
	;
};

/**
 * Get k most visually distant colors
 *
 * Avoids difficult colors for the color-blind.
 *
 * Uses CMC i:c quasimetric.
 */
void get_k_most_distant_colors();



}} // end namespace
