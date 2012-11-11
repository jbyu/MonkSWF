/*
 *  PlaceObject.cpp
 *  MonkSWF
 *
 *  Created by Micah Pearlman on 3/26/09.
 *  Copyright 2009 MP Engineering. All rights reserved.
 *
 */

#include "PlaceObject.h"
#include "DefineSprite.h"
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
		_has_move = reader->getbits( 1 );
		
		if( has_clip_actions || has_clip_depth )
			assert( 0 );
			
		_depth = reader->get<uint16_t>();

		if ( _has_character )
		{
			_character_id = reader->get<uint16_t>();
			IDefineSpriteTag* sprite = _swf->getSprite( _character_id );
			if ( sprite )
			{
				_pMovieClip = _swf->createMovieClip( *sprite );
			}
		}
			
		if ( _has_matrix )
		{
			reader->getMatrix( _transfrom );
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
	
	void PlaceObject2Tag::copyCharacter( IPlaceObjectTag* o )
	{
		PlaceObject2Tag* other = (PlaceObject2Tag*)o;
        _character_id = other->_character_id;
		_pMovieClip = other->_pMovieClip;
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
		else if ( _pMovieClip )
		{
			_pMovieClip->draw( swf );
		}
		// restore old matrix
		//vgLoadMatrix( oldMatrix );
        goMatrix = origMtx;
	}
	
	void PlaceObject2Tag::print() {
		_header.print();
		MK_TRACE("\tPLACEOBJECT2: id=%d, depth=%d, move=%d\n", _character_id, _depth, hasMove() );
	}

//=========================================================================

	MovieClip::MovieClip(const FrameList& frames)
		:_frame_list(frames)
		,_frame_count(frames.size())
		,_frame(0)
	{
	}

	void MovieClip::setFrame( uint32_t frame )
	{
		if (_frame_count <= frame)
		{
			frame = 0;
			// clear display list
			DisplayList::iterator iter = _display_list.begin();
			while ( _display_list.end() != iter )
			{
				iter->second = NULL;
				++iter;
			}
		}
        // build up the display list
		TagList* frame_tags = _frame_list[ frame ];
		setup_frame(_display_list, *frame_tags );
		_frame = frame;
	}

	void MovieClip::draw( SWF* swf )
	{
		// draw the display list
		DisplayList::iterator iter = _display_list.begin();
		while ( _display_list.end() != iter )
		{
			IPlaceObjectTag *place_obj = iter->second;
			if (place_obj) 
				place_obj->draw( swf ); 
			++iter;
		}
	}

	void setup_frame(DisplayList& display, const TagList& tags)
	{
        TagList::const_iterator it = tags.begin();
		while( it != tags.end() ) {
            ITag* tag = *it;
			switch ( tag->code() ) {
				case PLACEOBJECT2: {
					IPlaceObjectTag* place_obj = (IPlaceObjectTag*)tag;
					uint16_t depth = place_obj->depth();

					if ( place_obj->hasMove() == false && place_obj->hasCharacter() == true ) {
						// A new character (with ID of CharacterId) is placed on the display list at the specified
						// depth. Other fields set the attributes of this new character.
						IPlaceObjectTag* current_obj = display[ depth ];
						// copy over the previous matrix if the new character doesn't have one
						if ( current_obj && place_obj->hasMatrix() == false ) {
							place_obj->copyTransform( current_obj );
						}
						place_obj->setFrame(0xffffffff);
						display[ depth ] = place_obj;
						
					} else if ( place_obj->hasMove() == true && place_obj->hasCharacter() == false ) {
						// The character at the specified depth is modified. Other fields modify the attributes of this
						// character. Because any given depth can have only one character, no CharacterId is required.
						IPlaceObjectTag* current_obj = display[ depth ];
						if ( current_obj ) {
							//current_obj->copyTransform( place_obj );
							//place_obj->setCharacterId( current_obj->characterId() );
							place_obj->copyCharacter( current_obj );
							display[ depth ] = place_obj;
						}
					} else if ( place_obj->hasMove() == true && place_obj->hasCharacter() == true ) {
						// The character at the specified Depth is removed, and a new character (with ID of CharacterId) 
						// is placed at that depth. Other fields set the attributes of this new character.
						place_obj->setFrame(0xffffffff);
						display[ depth ] = place_obj;
					}
				} break;

				case REMOVEOBJECT:
				case REMOVEOBJECT2:	{
					IRemoveObjectTag* remove_object = (IRemoveObjectTag*)tag;
					display[ remove_object->depth() ] = NULL;
					//_display_list.erase( remove_object->depth() );
				} break;
	
				default:
					break;
			}
            ++it;
		}
		// update the display list
		DisplayList::iterator iter = display.begin();
		while ( display.end() != iter )
		{
			IPlaceObjectTag *place_obj = iter->second;
			if (place_obj) 
			{
				place_obj->play();
			}
			++iter;
		}
	}

}//namespace
