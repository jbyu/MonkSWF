/*
 *  PlaceObject.cpp
 *  MonkSWF
 *
 *  Created by Micah Pearlman on 3/26/09.
 *  Copyright 2009 MP Engineering. All rights reserved.
 *
 */

#include "PlaceObject.h"
#include "mkSWF.h"

using namespace std;

#define vgLoadMatrix (void)
#define vgTranslate (void)
#define vgScale (void)
#define vgMultMatrix (void)
#define vgGetMatrix (void)

namespace MonkSWF {
    
	bool PlaceObject2Tag::read( Reader* reader, SWF* _swf )
	{
		if( reader->getBitPos() )
			MK_TRACE("UNALIGNED!!\n");
		uint8_t has_clip_actions = reader->getbits( 1 );
		uint8_t has_clip_depth = reader->getbits( 1 );
		uint8_t has_name = reader->getbits( 1 );
		uint8_t has_ratio = reader->getbits( 1 );
		uint8_t has_color_transform = reader->getbits( 1 );
		_has_matrix = reader->getbits( 1 );
		_has_character = reader->getbits( 1 );
		_do_move = reader->getbits( 1 );
		
		if( has_clip_actions || has_clip_depth )
			assert( 0 );
			
		_depth = reader->get<uint16_t>();

		if( _has_character )
			_character_id = reader->get<uint16_t>();
			
		if( _has_matrix ) {
			//MATRIX m;
			reader->getMatrix( _transfrom );
			/*
			//	OpenVG:
			//	sx	shx	tx
			//	shy	sy	ty
			//	0	0	1
			
			// also see MATRIX in swf file format spec
			
			// translation
			_transform[0][2] = m.tx/20.0f;
			_transform[1][2] = m.ty/20.0f;
			
			// scale
			_transform[0][0] = m.sx/65536.0f;
			_transform[1][1] = m.sy/65536.0f;
			
			// rotate and skew
			// note the order of r1 and r0!
			_transform[0][1] = m.r1/65536.0f;
			_transform[1][0] = m.r0/65536.0f;
			*/
		}
		
		if ( has_color_transform ) {
		
			const uint8_t has_add_terms = reader->getbits( 1 );
			const uint8_t has_mult_terms = reader->getbits( 1 );
			const uint8_t nbits = reader->getbits( 4 );
			if( has_mult_terms ) {
				coord_t red = reader->getsignedbits( nbits );
				coord_t green = reader->getsignedbits( nbits );
				coord_t blue = reader->getsignedbits( nbits );
				coord_t alpha = reader->getsignedbits( nbits );

			}
			
			if( has_add_terms ) {
				coord_t red = reader->getsignedbits( nbits );
				coord_t green = reader->getsignedbits( nbits );
				coord_t blue = reader->getsignedbits( nbits );
				coord_t alpha = reader->getsignedbits( nbits );
			}
		}
		
		if ( has_ratio ) {
			// dummy read
			// TODO
			reader->get<uint16_t>();
		}
		if( has_name ) {
			//char* n = reader->getString();
			//_name = std::string( n );
			//delete [] n;
            reader->align();
            _name = std::string( reader->getString() );
            _texture = _swf->GetRenderer()->getTexture( _name.c_str() );
		}
		if( has_clip_depth ) {
			reader->get<uint16_t>();
		}
		if( has_clip_actions ) {
			// TOOD: support clip actions 
			assert( 0 );
		}
		
		return true;
	}
	
	void PlaceObject2Tag::copyTransform( IPlaceObjectTag* o ) {
		//copyNoTransform( o );
		PlaceObject2Tag* other = (PlaceObject2Tag*)o;
        _transfrom = other->_transfrom;
        /*
		for ( int i = 0; i < 3; i++ ) {
			for ( int p = 0; p < 3; p++ ) {
				_transform[i][p] = other->_transform[i][p];
			}
		}
        */
	}
	
	static inline float degrees (float radians) {return radians * (180.0f/3.14159f);}	

    static PVRTMATRIX3f goMatrix =
    {{
        1,0,0,
        0,1,0,    
        0,0,1
    }};

    void PVRTMatrixSet(PVRTMATRIX3f &mOut, const MATRIX& mIn)
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

    void PVRTMatrixMultiply(
	    PVRTMATRIX3f		&mOut,
	    const PVRTMATRIX3f	&mA,
	    const PVRTMATRIX3f	&mB)
    {
	    PVRTMATRIX3f mRet;
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
    
	void PlaceObject2Tag::draw( SWF* swf ) {
        PVRTMATRIX3f origMtx = goMatrix, mtx;
        PVRTMatrixSet(mtx, _transfrom);
        PVRTMatrixMultiply(goMatrix, mtx, goMatrix);
        /*
        vgGetMatrix(oldMatrix);
		vgMultMatrix( (VGfloat*)&_transform[0][0] );
		vgScale( _offsetScale, _offsetScale );
		vgTranslate( _offsetTranslate[0], _offsetTranslate[1] );
		*/
        swf->GetRenderer()->applyTransform(goMatrix);
        swf->GetRenderer()->applyTexture(_texture);
		IDefineShapeTag* shape = swf->getShape( _character_id );
		if( shape )
        {
			shape->draw( swf );
        }
		else
        {
			IDefineSpriteTag* sprite = swf->getSprite( _character_id );
			if ( sprite )
                sprite->draw( swf );
		}
		// restore old matrix
		//vgLoadMatrix( oldMatrix );
        goMatrix = origMtx;
	}
	
	void PlaceObject2Tag::print() {
		_header.print();
		MK_TRACE("\tPLACEOBJECT2: id=%d, depth=%d, move=%d\n", _character_id, _depth, doMove() );
	}

}