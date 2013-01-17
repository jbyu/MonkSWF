/*
 *  PlaceObject.h
 *  MonkSWF
 *
 *  Created by Micah Pearlman on 3/26/09.
 *  Copyright 2009 MP Engineering. All rights reserved.
 *
 */

#ifndef __PlaceObject_h__
#define __PlaceObject_h__

#include "mkTag.h"
#include "mkMovieClip.h"
#include <string>

namespace MonkSWF {

    class DefineShapeTag;

	class PlaceObjectTag : public ITag
    {
	public:
		PlaceObjectTag( TagHeader& h ) 
			:ITag( h )
		    ,_character_id( 0xffff )
		    ,_depth( 0 )
            ,_clipDepth(0)
		    ,_has_matrix( 0 )
		    ,_has_character( 0 )
		    ,_has_move( 0 )
            ,_pShape(NULL)
            ,_pMovie(NULL)
			,_name( "none" )
		{
		}
		
		virtual ~PlaceObjectTag()
		{
		}

		virtual bool read( Reader& reader, SWF&, MovieFrames& );

		virtual void print();

        virtual void setup(MovieClip&);

		void draw(void);

		void copyCharacter( PlaceObjectTag* other )
        {
            _character_id = other->_character_id;
            _depth = other->_depth;
            _clipDepth = other->_clipDepth;
		    _pMovie = other->_pMovie;
            _pShape = other->_pShape;
        }

		void copyTransform( PlaceObjectTag* other )
        {
            _transform = other->_transform;
        }

		void update(void)
		{
			if (_pMovie)
				_pMovie->update();
		}

        void play(bool enable) 
        {
			if (_pMovie)
				_pMovie->play(enable);
		}

		void gotoFrame(uint32_t frame)
		{
			if (_pMovie)
				_pMovie->gotoFrame( frame );
		}

		uint16_t depth()        const { return _depth; }
        uint16_t clipDepth()    const { return _clipDepth; }
		uint16_t characterId()  const { return _character_id; }
		bool hasCharacter()     const { return 0!=_has_character; }
		bool hasMatrix()        const { return 0!=_has_matrix; }
		bool hasMove()          const { return 0!=_has_move; }

		static ITag* create( TagHeader& header ) {
			return (ITag*)(new PlaceObjectTag( header ));
		}

	private:
		uint16_t	_character_id;
		uint16_t	_depth;
		uint16_t	_clipDepth;
		uint8_t		_has_matrix;
		uint8_t		_has_character;
		uint8_t		_has_move;

        DefineShapeTag  *_pShape;
        MovieClip       *_pMovie;
        MATRIX          _transform;
        CXFORM          _cxform;
		std::string     _name;
	};
}
#endif // __PlaceObject_h__