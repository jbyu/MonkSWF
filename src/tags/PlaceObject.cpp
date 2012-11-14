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
#include "DefineSprite.h"
#include "DefineShape.h"
#include "mkSWF.h"

using namespace std;

#define vgLoadMatrix (void)
#define vgTranslate (void)
#define vgScale (void)
#define vgMultMatrix (void)
#define vgGetMatrix (void)

namespace MonkSWF {

    const MATRIX kMatrixIdentity =
    {
        1,0,0,
        0,1,0
    };
    const CXFORM kCXFormIdentity = 
    {
        {1,1,1,1},
        {0,0,0,0}
    };
    const MATRIX3f kMatrix3fIdentity =
    {{
        1,0,0,
        0,1,0,    
        0,0,1
    }};
    
	bool PlaceObjectTag::read( Reader* reader, SWF* _swf )
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
		
		_depth = reader->get<uint16_t>();

		if ( _has_character )
		{
			_character_id = reader->get<uint16_t>();
            // create movie clip for the sprite
			ITag* tag = _swf->getCharacter( _character_id );
            if ( tag->code() == TAG_DEFINE_SPRITE )
				_pMovieClip = _swf->createMovieClip( *tag );
            else
                _pShape = (DefineShapeTag*)tag;
		}
			
		if ( _has_matrix ) {
			reader->getMatrix( _transform );
		} else {
            _transform = kMatrixIdentity;
        }
		
        _cxform = kCXFormIdentity;
		if ( has_color_transform ) {
            reader->align();
			const uint8_t has_add_terms = reader->getbits( 1 );
			const uint8_t has_mult_terms = reader->getbits( 1 );
			const uint8_t nbits = reader->getbits( 4 );
			if( has_mult_terms ) {
                reader->getColor(_cxform.mult, nbits);
			}
			if( has_add_terms ) {
                reader->getColor(_cxform.add, nbits);
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
            //_texture = Renderer::getRenderer()->getTexture( _name.c_str() );
		}
		if( has_clip_depth ) {
			_clipDepth = reader->get<uint16_t>();
		}
		if( has_clip_actions ) {
			// TOOD: support clip actions 
			assert( 0 );
		}
		
		return true;
	}
		
	//static inline float degrees (float radians) {return radians * (180.0f/3.14159f);}	
    static MATRIX3f goRootMatrix = kMatrix3fIdentity;
    static CXFORM goRootCXForm = kCXFormIdentity;

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

	void PlaceObjectTag::draw( SWF* swf ) {
        //concatenate matrix
        MATRIX3f origMTX = goRootMatrix, mtx;
        MATRIX3fSet(mtx, _transform);
        MATRIX3fMultiply(goRootMatrix, mtx, goRootMatrix);

        CXFORM origCXF = goRootCXForm;
        CXFORMMultiply(goRootCXForm, _cxform, goRootCXForm);

        Renderer::getRenderer()->applyTransform(goRootMatrix);

        // C' = C * Mult + Add
        // in opengl, use blend mode and multi-pass to achieve that
        // 1st pass TexEnv(GL_BLEND) with glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
        //      Cp * (1-Ct) + Cc *Ct 
        // 2nd pass TexEnv(GL_MODULATE) with glBlendFunc(GL_SRC_ALPHA, GL_ONE)
        //      dest + Cp * Ct
        // let Mult as Cc and Add as Cp, then we get the result
		if (_pShape)
            _pShape->draw( swf, goRootCXForm );
		else if ( _pMovieClip )
			_pMovieClip->draw( swf );
#if 0
        swf->GetRenderer()->applyPassMult(goRootCXForm);
        swf->GetRenderer()->applyPassAdd(goRootCXForm);
		if (shape)
            shape->draw( swf );
		else if ( _pMovieClip )
			_pMovieClip->draw( swf );
#endif
		// restore old matrix
        goRootMatrix = origMTX;
        goRootCXForm = origCXF;
	}
	
	void PlaceObjectTag::print() {
		_header.print();
        MK_TRACE("id=%d, depth=%d, clip=%d, move=%d, name=%s\n", _character_id, _depth, _clipDepth, hasMove(), _name.c_str() );
        //MK_TRACE("\tCXFORM-Mult:%1.2f,%1.2f,%1.2f,%1.2f\n", _cxform.mult.r, _cxform.mult.g, _cxform.mult.b, _cxform.mult.a );
        //MK_TRACE("\tCXFORM-Add: %1.2f,%1.2f,%1.2f,%1.2f\n", _cxform.add.r, _cxform.add.g, _cxform.add.b, _cxform.add.a );
	}

    void PlaceObjectTag::setup(MovieClip& movie) 
    {
        DisplayList& _display_list = movie.getDisplayList();
		const uint16_t depth = this->depth();
		PlaceObjectTag* current_obj = _display_list[ depth ];

		if ( this->hasMove() == false && this->hasCharacter() == true )
        {
			// A new character (with ID of CharacterId) is placed on the display list at the specified
			// depth. Other fields set the attributes of this new character.
			// copy over the previous matrix if the new character doesn't have one
			if (current_obj && this->hasMatrix() == false ) {
                current_obj->gotoFrame(0xffffffff);
				this->copyTransform( current_obj );
			}
			this->play(true);
			_display_list[ depth ] = this;
		}
        else if ( this->hasMove() == true && this->hasCharacter() == false )
        {
			// The character at the specified depth is modified. Other fields modify the attributes of this
			// character. Because any given depth can have only one character, no CharacterId is required.
			if ( current_obj ) {
				//current_obj->copyTransform( place_obj );
				//place_obj->setCharacterId( current_obj->characterId() );
				if (this->hasMatrix() == false ) {
					this->copyTransform( current_obj );
				}
				this->copyCharacter( current_obj );
				_display_list[ depth ] = this;
			}
		}
        else if ( this->hasMove() == true && this->hasCharacter() == true )
        {
			// The character at the specified Depth is removed, and a new character (with ID of CharacterId) 
			// is placed at that depth. Other fields set the attributes of this new character.
			if (current_obj && this->hasMatrix() == false ) {
                current_obj->gotoFrame(0xffffffff);
				this->copyTransform( current_obj );
			}
			this->play(true);
			_display_list[ depth ] = this;
		}
    }

//=========================================================================

	MovieClip::MovieClip(const FrameList& frames)
		:_frame_list(frames)
		,_frame_count(frames.size())
		,_frame(0xffffffff)
        ,_play(true)
	{
	}

    void MovieClip::clearDisplayList(void)
    {
		DisplayList::iterator iter = _display_list.begin();
		while ( _display_list.end() != iter )
		{
    		PlaceObjectTag *place_obj = iter->second;
       		//if (place_obj) place_obj->play(false);
			iter->second = NULL;
			++iter;
		}
    }

	void MovieClip::gotoFrame( uint32_t frame )
	{
		if (_frame_count <= frame)
		{
			frame = 0;
            clearDisplayList();
		}
        // build up the display list
		TagList* frame_tags = _frame_list[ frame ];
		setupFrame( *frame_tags );
		_frame = frame;
	}

    void MovieClip::update(void)
    {
        if (_play)
            gotoFrame(_frame + 1);
    	// update the display list
		DisplayList::iterator iter = _display_list.begin();
		while ( _display_list.end() != iter )
		{
			PlaceObjectTag *place_obj = iter->second;
			if (place_obj) 
			{
				place_obj->update();
			}
			++iter;
		}
    }

    void MovieClip::draw( SWF* swf )
	{
        PlaceObjectTag *mask = NULL;
		uint16_t highest_masked_layer = 0;
		// draw the display list
		DisplayList::iterator iter = _display_list.begin();
		while ( _display_list.end() != iter )
		{
			PlaceObjectTag *place_obj = iter->second;
			if (place_obj)
            {
                const uint16_t clip = place_obj->clipDepth();
                const uint16_t depth = place_obj->depth();
			    if (mask && depth > highest_masked_layer)
                {
                    // restore stencil
                    Renderer::getRenderer()->maskOffBegin();
                    mask->draw( swf ); 
                    Renderer::getRenderer()->maskOffEnd();
    			    mask = NULL;
			    }
			    if (0 < clip) {
                    // draw mask
    				Renderer::getRenderer()->maskBegin();
                    place_obj->draw( swf ); 
    				Renderer::getRenderer()->maskEnd();
	    			highest_masked_layer = clip;
				    mask = place_obj;
			    } else {
                    place_obj->draw( swf ); 
                }
            }
			++iter;
		}
		if (mask)
		{
			// If a mask masks the scene all the way up to the highest layer, 
            // it will not be disabled at the end of drawing the display list, so disable it manually.
            Renderer::getRenderer()->maskOffBegin();
            mask->draw( swf ); 
            Renderer::getRenderer()->maskOffEnd();
    	    mask = NULL;
		}
	}

	void MovieClip::setupFrame(const TagList& tags)
	{
        TagList::const_iterator it = tags.begin();
		while( it != tags.end() )
        {
            ITag* tag = *it;
            tag->setup(*this);
            ++it;
		}
	}

}//namespace
