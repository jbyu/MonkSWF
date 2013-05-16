/*
 *  reader.cpp
 *  MonkSWF
 *
 *  Created by Micah Pearlman on 3/18/09.
 *  Copyright 2009 MP Engineering. All rights reserved.
 *
 */

#include "mkReader.h"
#define UNUSED(x) (void) (x)

namespace MonkSWF {

void Reader::getFilterList() {
	// reads FILTERLIST
	int filters = get<uint8_t>();
	for (int i = 0; i < filters; i++) {
		int filter_id = get<uint8_t>();
		switch (filter_id) {
		// DropShadowFilter
		case 0:	{
			RGBA drop_shadow_color;
			getRGBA(drop_shadow_color);
			float blur_x = getFIXED();	// Horizontal blur amount
			float blur_y = getFIXED();	// Vertical blur amount
			float angle = getFIXED();	// Radian angle of the drop shadow
			float distance = getFIXED();	// Distance of the drop shadow
			float strength = get<int16_t>();	// hack, must be FIXED8 Strength of the drop shadow
			uint8_t flag = get<uint8_t>();
			int inner_shadow = flag & 0x80;
			int knockout = flag & 0x40;
			int composite_source = flag & 0x20;
			int passes = flag & 0x1f;	// Number of blur passes
			UNUSED(blur_x);
			UNUSED(blur_y);
			UNUSED(angle);
			UNUSED(distance);
			UNUSED(strength);
			UNUSED(inner_shadow);
			UNUSED(knockout);
			UNUSED(composite_source);
			UNUSED(passes);
			}
			break;

		// BlurFilter
		case 1: {
			float blur_x = getFIXED(); // Horizontal blur amount
			float blur_y = getFIXED(); // Vertical blur amount
			uint8_t flag = get<uint8_t>();
			int passes = flag & 0x1f;	// Number of blur passes
			UNUSED(blur_x);
			UNUSED(blur_y);
			UNUSED(passes);
			}
			break;

		// GlowFilter
		case 2: {
			RGBA glow_color;
			getRGBA(glow_color);
			float blur_x = getFIXED();	// Horizontal blur amount
			float blur_y = getFIXED();	// Vertical blur amount
			float strength = get<int16_t>();	// hack, must be FIXED8 Strength of the drop shadow
			uint8_t flag = get<uint8_t>();
			int inner_glow = flag & 0x80;
			int knockout = flag & 0x40;
			int composite_source = flag & 0x20;
			int passes = flag & 0x1f;	// Number of blur passes
			UNUSED(blur_x);
			UNUSED(blur_y);
			UNUSED(strength);
			UNUSED(inner_glow);
			UNUSED(knockout);
			UNUSED(composite_source);
			UNUSED(passes);
			}
			break;

		// BevelFilter
		case 3: {
			RGBA shadow_color;
			getRGBA(shadow_color);
			RGBA highlight_color;
			getRGBA(highlight_color);
			float blur_x = getFIXED();	// Horizontal blur amount
			float blur_y = getFIXED();	// Vertical blur amount
			float angle = getFIXED();	// Radian angle of the drop shadow
			float distance = getFIXED();	// Distance of the drop shadow
			float strength = get<int16_t>();	// hack, must be FIXED8 Strength of the drop shadow
			uint8_t flag = get<uint8_t>();
			int inner_shadow = flag & 0x80;
			int knockout = flag & 0x40;
			int composite_source = flag & 0x20;
			int on_top = flag & 0x10;
			int passes = flag & 0x0f;	// Number of blur passes
			UNUSED(blur_x);
			UNUSED(blur_y);
			UNUSED(angle);
			UNUSED(distance);
			UNUSED(strength);
			UNUSED(inner_shadow);
			UNUSED(knockout);
			UNUSED(composite_source);
			UNUSED(on_top);
			UNUSED(passes);
			}
			break;

		// GradientGlowFilter
		case 4: {
			int num_colors = get<uint8_t>();		// Number of colors in the gradient
			for (int i = 0; i < num_colors; i++) {
				RGBA gradient_colors;
				getRGBA(gradient_colors);
			}
			for (int i = 0; i < num_colors; i++)
			{
				int gradient_ratio = get<uint8_t>();	// UI8[NumColors] Gradient ratios
				UNUSED(gradient_ratio);
			}
			float blur_x = getFIXED();	// Horizontal blur amount
			float blur_y = getFIXED();	// Vertical blur amount
			float angle = getFIXED();	// Radian angle of the drop shadow
			float distance = getFIXED();	// Distance of the drop shadow
			float strength = get<int16_t>();	// hack, must be FIXED8 Strength of the drop shadow
			uint8_t flag = get<uint8_t>();
			int inner_shadow = flag & 0x80;
			int knockout = flag & 0x40;
			int composite_source = flag & 0x20;
			int on_top = flag & 0x10;
			int passes = flag & 0x0f;	// Number of blur passes
			UNUSED(blur_x);
			UNUSED(blur_y);
			UNUSED(angle);
			UNUSED(distance);
			UNUSED(strength);
			UNUSED(inner_shadow);
			UNUSED(knockout);
			UNUSED(composite_source);
			UNUSED(on_top);
			UNUSED(passes);
			}
			break;

		// ConvolutionFilter
		case 5: {
			int matrix_x = get<uint8_t>();	// Horizontal matrix size
			int matrix_y = get<uint8_t>();	// Vertical matrix size
			float divisor = get<float>();	// Divisor applied to the matrix values
			float bias = get<float>();	// Bias applied to the matrix values
			for (int k = 0; k < matrix_x * matrix_y; k++)
			{
				get<float>();	// Matrix values
			}
			RGBA default_color;
			getRGBA(default_color);
			uint8_t flag = get<uint8_t>();
			int clamp = flag & 0x2;
			int preserve_alpha = flag & 0x1;
			UNUSED(divisor);
			UNUSED(bias);
			UNUSED(clamp);
			UNUSED(preserve_alpha);
			}
			break;

		// ColorMatrixFilter
		case 6: 
			// matrix is float[20]
			for (int k = 0; k < 20; k++)
			{
				get<float>();
			}
			break;

		// GradientBevelFilter
		case 7: {
			int num_colors = get<uint8_t>();		// Number of colors in the gradient
			for (int i = 0; i < num_colors; i++) {
				RGBA gradient_colors;
				getRGBA(gradient_colors);
			}
			for (int i = 0; i < num_colors; i++) {
				int gradient_ratio = get<uint8_t>();	// UI8[NumColors] Gradient ratios
				UNUSED(gradient_ratio);
			}
			float blur_x = getFIXED();	// Horizontal blur amount
			float blur_y = getFIXED();	// Vertical blur amount
			float angle = getFIXED();	// Radian angle of the drop shadow
			float distance = getFIXED();	// Distance of the drop shadow
			float strength = get<int16_t>();	// hack, must be FIXED8 Strength of the drop shadow
			uint8_t flag = get<uint8_t>();
			int inner_shadow = flag & 0x80;
			int knockout = flag & 0x40;
			int composite_source = flag & 0x20;
			int on_top = flag & 0x10;
			int passes = flag & 0x0f;	// Number of blur passes
			UNUSED(blur_x);
			UNUSED(blur_y);
			UNUSED(angle);
			UNUSED(distance);
			UNUSED(strength);
			UNUSED(inner_shadow);
			UNUSED(knockout);
			UNUSED(composite_source);
			UNUSED(on_top);
			UNUSED(passes);
			}
			break;

		default:
			assert(0);	// invalid input
		}
	}
}

}
