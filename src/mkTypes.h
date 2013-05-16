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
	const float SWF_INV_TWIPS   = 1.f / SWF_TWIPS;
	const float SWF_INV_CXFORM  = 1.f / (1<<8);
	const float SWF_INV_MATRIX  = 1.f / (1<<16);
	const float SWF_INV_COLOR   = 1.f / 255.f;

	// Basic Structures
	
	struct POINT
	{ 
		coord_t        x;
		coord_t        y;
	};

	struct POINTf
	{ 
		float        x;
		float        y;
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

	inline void MergeRectangle(RECT &rect, const RECT other) {
		if (other.xmin < rect.xmin)
			rect.xmin = other.xmin;
		if (other.xmax > rect.xmax)
			rect.xmax = other.xmax;
		if (other.ymin < rect.ymin)
			rect.ymin = other.ymin;
		if (other.ymax > rect.ymax)
			rect.ymax = other.ymax;
	}

    inline bool IsWithinRectangle(const RECT& rect, float x, float y) {
		if (x < rect.xmin)
			return false;
		if (x > rect.xmax)
			return false;
		if (y < rect.ymin)
			return false;
		if (y > rect.ymax)
			return false;
		return true;
	}
		
	union MATRIX { 
		struct {
			float  sx, r1, tx;
			float  r0, sy, ty;
		};
		float m_[2][3];

		void setIdentity() {
			sx = 1; r1 = tx = 0;
			sy = 1; r0 = ty = 0;
		}
		void setInverse(const MATRIX& m);
		void transform(POINTf& result, const POINTf& p) const;
		void transform(RECT& result, const RECT& p) const;
	};

    const MATRIX kMatrixIdentity = {
        1,0,0,
        0,1,0
    };

	// Transform point 'p' by our matrix.  Put the result in result.
	inline void MATRIX::transform(POINTf& result, const POINTf& p) const {
		result.x = m_[0][0] * p.x + m_[0][1] * p.y + m_[0][2];
		result.y = m_[1][0] * p.x + m_[1][1] * p.y + m_[1][2];
	}

	// Set this matrix to the inverse of the given matrix.
	inline void MATRIX::setInverse(const MATRIX& m)
	{
		// Invert the rotation part.
		float det = m.m_[0][0] * m.m_[1][1] - m.m_[0][1] * m.m_[1][0];
		if (det == 0.0f) {
			// Arbitrary fallback.
			setIdentity();
			m_[0][2] = -m.m_[0][2];
			m_[1][2] = -m.m_[1][2];
		} else {
			float	inv_det = 1.0f / det;
			m_[0][0] = m.m_[1][1] * inv_det;
			m_[1][1] = m.m_[0][0] * inv_det;
			m_[0][1] = -m.m_[0][1] * inv_det;
			m_[1][0] = -m.m_[1][0] * inv_det;

			m_[0][2] = -(m_[0][0] * m.m_[0][2] + m_[0][1] * m.m_[1][2]);
			m_[1][2] = -(m_[1][0] * m.m_[0][2] + m_[1][1] * m.m_[1][2]);
		}
	}

    struct MATRIX3f
    {
        float* operator [] ( const int Row )
    	{
		    return &f[Row*3];
	    }
	    float f[9];	/*!< Array of float */
    };

    inline void MATRIX3fSet(MATRIX3f &mOut, const MATRIX& mIn)
    {
        mOut.f[0] = mIn.sx;
        mOut.f[4] = mIn.sy;
        mOut.f[1] = mIn.r0;
        mOut.f[3] = mIn.r1;
        mOut.f[6] = mIn.tx;
        mOut.f[7] = mIn.ty;
        mOut.f[2] = 0;
        mOut.f[5] = 0;
        mOut.f[8] = 1;
    }

    inline void MATRIX3fMultiply(
	    MATRIX3f		&mOut,
	    const MATRIX3f	&mA,
	    const MATRIX3f	&mB)
    {
	    MATRIX3f mRet;
	    mRet.f[0] = mA.f[0]*mB.f[0] + mA.f[1]*mB.f[3] + mA.f[2]*mB.f[6];
	    mRet.f[1] = mA.f[0]*mB.f[1] + mA.f[1]*mB.f[4] + mA.f[2]*mB.f[7];
	    mRet.f[2] = mA.f[0]*mB.f[2] + mA.f[1]*mB.f[5] + mA.f[2]*mB.f[8];

	    mRet.f[3] = mA.f[3]*mB.f[0] + mA.f[4]*mB.f[3] + mA.f[5]*mB.f[6];
	    mRet.f[4] = mA.f[3]*mB.f[1] + mA.f[4]*mB.f[4] + mA.f[5]*mB.f[7];
	    mRet.f[5] = mA.f[3]*mB.f[2] + mA.f[4]*mB.f[5] + mA.f[5]*mB.f[8];

	    mRet.f[6] = mA.f[6]*mB.f[0] + mA.f[7]*mB.f[3] + mA.f[8]*mB.f[6];
	    mRet.f[7] = mA.f[6]*mB.f[1] + mA.f[7]*mB.f[4] + mA.f[8]*mB.f[7];
	    mRet.f[8] = mA.f[6]*mB.f[2] + mA.f[7]*mB.f[5] + mA.f[8]*mB.f[8];
	    mOut = mRet;
    }

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

    const CXFORM kCXFormIdentity = {
        {1,1,1,1},
        {0,0,0,0}
    };
    const MATRIX3f kMatrix3fIdentity = {
		{1,0,0,
		 0,1,0,
		 0,0,1}
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