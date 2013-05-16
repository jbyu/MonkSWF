/*
 *  PlaceObject.cpp
 *  MonkSWF
 *
 *  Created by Micah Pearlman on 3/26/09.
 *  Copyright 2009 MP Engineering. All rights reserved.
 *
 */
//#include <Windows.h>
//#include <GL/gl.h>

#include "PlaceObject.h"
#include "mkSWF.h"
#include "DefineButton.h"

using namespace std;

#define vgLoadMatrix (void)
#define vgTranslate (void)
#define vgScale (void)
#define vgMultMatrix (void)
#define vgGetMatrix (void)

namespace MonkSWF {

PlaceObjectTag::PlaceObjectTag( const ButtonRecord& rec )
	:ITag( DoActionTag::scButtonHeader )
	,_character_id( rec._character_id )
	,_depth( rec._depth )
    ,_clipDepth( 0 )
	,_has_matrix( 1 )
	//,_has_character( 0 )
	//,_has_move( 0 )
	,_placeMode( PLACE )
    ,_character( NULL )
	,_name( "none" )
	,_transform( rec._matrix )
	,_cxform( rec._cxform )
{
}

bool PlaceObjectTag::read( Reader& reader, SWF& swf, MovieFrames& data )
{
	if ( reader.getBitPos() )
		MK_TRACE("UNALIGNED!!\n");

	uint8_t has_clip_actions = reader.getbits( 1 );
	uint8_t has_clip_depth = reader.getbits( 1 );
	uint8_t has_name = reader.getbits( 1 );
	uint8_t has_ratio = reader.getbits( 1 );
	uint8_t has_color_transform = reader.getbits( 1 );
	_has_matrix = reader.getbits( 1 );
	uint8_t _has_character = reader.getbits( 1 );
	uint8_t _has_move = reader.getbits( 1 );
		
	_depth = reader.get<uint16_t>();

	if ( _has_character )
	{
		_character_id = reader.get<uint16_t>();
	}
			
	if ( _has_matrix ) {
		reader.getMatrix( _transform );
	} else {
        _transform = kMatrixIdentity;
    }
		
    _cxform = kCXFormIdentity;
	if ( has_color_transform ) {
        reader.align();
		reader.getCXForm( _cxform );
    }
		
	if ( has_ratio ) {
		// dummy read
		// TODO
		reader.get<uint16_t>();
	}
	if ( has_name ) {
        reader.align();
        _name.assign( reader.getString() );
	}
	if ( has_clip_depth ) {
		_clipDepth = reader.get<uint16_t>();
	}
	if ( has_clip_actions ) {
		// TOOD: support clip actions 
		assert( 0 );
	}

	if ( _has_move == 0 && _has_character == 1 ) {
		_placeMode = PLACE;
	} else if ( _has_move == 1 && _has_character == 0 ) {
		_placeMode = MOVE;
	} else if ( _has_move == 1 && _has_character == 1 ) {
		_placeMode = REPLACE;
	}

	if (! MovieClip::sbCalculateRectangle) 
		return true;

	if ( _has_character ) {
		RECT rect = swf.calculateRectangle(_character_id, 0<_has_matrix? &_transform:NULL );
		MergeRectangle(data._rectangle, rect);
	}

	return true; // keep tag
}

//static inline float degrees (float radians) {return radians * (180.0f/3.14159f);}	
inline void COLOR4fSet(
	COLOR4f		&mOut,
	const COLOR4f	&mIn)
{
    mOut.r = mIn.r;
    mOut.g = mIn.g;
    mOut.b = mIn.b;
    mOut.a = mIn.a;
}
inline void COLOR4fMultiply(
	COLOR4f		&mOut,
	const COLOR4f	&mA,
	const COLOR4f	&mB)
{
    mOut.r = mA.r * mB.r;
    mOut.g = mA.g * mB.g;
    mOut.b = mA.b * mB.b;
    mOut.a = mA.a * mB.a;
}
inline void COLOR4fAdd(
	COLOR4f		&mOut,
	const COLOR4f	&mA,
	const COLOR4f	&mB)
{
    mOut.r = mA.r + mB.r;
    mOut.g = mA.g + mB.g;
    mOut.b = mA.b + mB.b;
    mOut.a = mA.a + mB.a;
}

inline void CXFORMMultiply(
	CXFORM		&mOut,
	const CXFORM	&child,
	const CXFORM	&parent)
{
    COLOR4f add;
    COLOR4fMultiply(add, parent.mult, child.add);
    COLOR4fAdd(mOut.add, parent.add, add);
    COLOR4fMultiply(mOut.mult, child.mult, parent.mult);
}

void PlaceObjectTag::draw() {
    //concatenate matrix
	MATRIX3f& currMTX = SWF::getCurrentMatrix();
    MATRIX3f origMTX = currMTX, mtx;
    MATRIX3fSet(mtx, _transform); // convert matrix format
    MATRIX3fMultiply(currMTX, mtx, currMTX);

	CXFORM& currCXF = SWF::getCurrentCXForm();
    CXFORM origCXF = currCXF;
    CXFORMMultiply(currCXF, _cxform, currCXF);

    Renderer::getRenderer()->applyTransform(currMTX);

    // C' = C * Mult + Add
    // in opengl, use blend mode and multi-pass to achieve that
    // 1st pass TexEnv(GL_BLEND) with glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
    //      Cp * (1-Ct) + Cc *Ct 
    // 2nd pass TexEnv(GL_MODULATE) with glBlendFunc(GL_SRC_ALPHA, GL_ONE)
    //      dest + Cp * Ct
    // let Mult as Cc and Add as Cp, then we get the result
	if (_character)
		_character->draw();

	// restore old matrix
    currMTX = origMTX;
    currCXF = origCXF;
}
	
void PlaceObjectTag::print() {
	_header.print();
#ifdef SWF_DEBUG
	const char *mode[] = {"N/A","PLACE","MOVE","REPLACE"};
	MK_TRACE("character=%d, depth=%d, clip=%d, mode=%s, name=%s\n", _character_id, _depth, _clipDepth, mode[_placeMode], _name.c_str() );
#endif
    //MK_TRACE("\tCXFORM-Mult:%1.2f,%1.2f,%1.2f,%1.2f\n", _cxform.mult.r, _cxform.mult.g, _cxform.mult.b, _cxform.mult.a );
    //MK_TRACE("\tCXFORM-Add: %1.2f,%1.2f,%1.2f,%1.2f\n", _cxform.add.r, _cxform.add.g, _cxform.add.b, _cxform.add.a );
}

void PlaceObjectTag::setup(MovieClip& movie) 
{
    DisplayList& _display_list = movie.getDisplayList();
	const uint16_t depth = this->depth();
	PlaceObjectTag* current_obj = _display_list[ depth ];

	switch (_placeMode) {
	default:
		MK_ASSERT(false);
		break;

	case PLACE:
		// A new character (with ID of CharacterId) is placed on the display list at the specified
		// depth. Other fields set the attributes of this new character.
		// copy over the previous matrix if the new character doesn't have one
		if ( current_obj && this->hasMatrix() == false ) {
            current_obj->gotoFrame(ICharacter::kFRAME_MAXIMUM);
			this->copyTransform( current_obj );
		}
		_character = movie.getInstance(this);
		//this->play(true);
		_display_list[ depth ] = this;
		break;

	case MOVE:
		// The character at the specified depth is modified. Other fields modify the attributes of this
		// character. Because any given depth can have only one character, no CharacterId is required.
		MK_ASSERT(current_obj);
		//current_obj->copyTransform( place_obj );
		//place_obj->setCharacterId( current_obj->characterId() );
		if ( this->hasMatrix() == false ) {
			this->copyTransform( current_obj );
		}
		this->copyCharacter( current_obj );
		_display_list[ depth ] = this;
		break;

	case REPLACE:
		// The character at the specified Depth is removed, and a new character (with ID of CharacterId) 
		// is placed at that depth. Other fields set the attributes of this new character.
		if ( current_obj && this->hasMatrix() == false ) {
            current_obj->gotoFrame(ICharacter::kFRAME_MAXIMUM);
			this->copyTransform( current_obj );
		}
		_character = movie.getInstance(this);
		//this->play(true);
		_display_list[ depth ] = this;
	}
}

}//namespace
