/*
 *  mkSWFTypes.h
 *  MonkSWF
 *
 *  Created by Micah Pearlman on 3/19/09.
 *  Copyright 2009 MP Engineering. All rights reserved.
 *
 */

#ifndef __mkSWFTypes_h__
#define __mkSWFTypes_h__

namespace MonkSWF {

	typedef     signed long     fixed_t;
	typedef     signed long     coord_t;
	
	#define coord_t_MAX 0x7fffffff
	#define coord_t_MIN -0x80000000
	
    const int SWF_TWIPS	= 20;
	const float SWF_INV_TWIPS = 1.f / SWF_TWIPS;
	const float SWF_INV_CXFORM = 1.f / (1<<8);
	const float SWF_INV_MATRIX = 1.f / (1<<16);

	// Basic Structures
	
	struct POINT
	{ 
		coord_t        x;
		coord_t        y;
	};
	
	struct RGBA
	{ 
		uint8_t    a;
		uint8_t    r;
		uint8_t    g;
		uint8_t    b;
	};

   	struct COLOR4f
    {
        float r,g,b,a;
    };
    	
	struct YUV
	{
		uint8_t	y,u,v;
	};
	
	struct RECT
	{ 
		float        xmin;
		float        ymin;
		float        xmax;
		float        ymax;
	};
	
	struct MATRIX { 
		float        sx, r1, tx;
		float        r0, sy, ty;
	};

    struct MATRIX3f
    {
        float* operator [] ( const int Row )
    	{
		    return &f[Row*3];
	    }
	    float f[9];	/*!< Array of float */
    };

	typedef struct _CXFORM
    {
        COLOR4f mult;
        COLOR4f add;
	} CXFORM;
	
	struct GRADIENT
	{
		int num;
		uint8_t* ratios;
		RGBA* rgba;
	};
//	
//	typedef struct _FILTER
//		{
//			uint8_t type;
//		} FILTER;
//	
//	typedef struct _FILTERLIST
//		{
//			int num;
//			FILTER*filter[8];
//		} FILTERLIST;
//	
//	typedef struct _TAG             // NEVER access a Tag-Struct directly !
//		{ U16           id;
//			uint8_t *          data;
//			U32           memsize;        // to minimize realloc() calls
//			
//			U32         len;            // for Set-Access
//			U32         pos;            // for Get-Access
//			
//			struct _TAG * next;
//			struct _TAG * prev;
//			
//			uint8_t            readBit;        // for Bit-Manipulating Functions [read]
//			uint8_t            writeBit;       // [write]
//			
//		} TAG;
//	
//#define swf_ResetReadBits(tag)   if (tag->readBit)  { tag->pos++; tag->readBit = 0; }
//#define swf_ResetWriteBits(tag)  if (tag->writeBit) { tag->writeBit = 0; }
//	
//	typedef struct _SOUNDINFO 
//		{
//			uint8_t stop;
//			uint8_t nomultiple; //continue playing if already started
//			
//			U32 inpoint;
//			U32 outpoint;
//			
//			U16 loops;
//			uint8_t envelopes;
//			
//			//envelope:
//			U32* pos;
//			U32* left;
//			U32* right;
//		} SOUNDINFO;
	
}
#endif	// __mkSWFTypes_h__