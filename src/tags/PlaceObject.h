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
#include <string>

namespace MonkSWF {

	struct ButtonRecord;
	class MovieClip;

	class PlaceObjectTag : public ITag
    {
		enum Mode {
			INVALID,
			PLACE,
			MOVE,
			REPLACE
		};

	public:
		PlaceObjectTag( TagHeader& h ) 
			:ITag( h )
		    ,_character_id( 0xffff )
		    ,_depth( 0 )
            ,_clipDepth(0)
		    ,_has_matrix( 0 )
		    //,_has_character( 0 )
		    //,_has_move( 0 )
			,_placeMode( INVALID )
            ,_character(NULL)
			,_name( "none" )
		{
		}

		PlaceObjectTag( const ButtonRecord& h );

		virtual ~PlaceObjectTag()
		{
		}

		virtual bool read( Reader& reader, SWF&, MovieFrames& );

		virtual void print();

		virtual void setup(MovieClip&, bool skipAction);

		void draw(void);

		ICharacter* getCharacter(void) { return _character; }

		const MATRIX& getTransform(void) const { return _transform; }

		void copyCharacter( PlaceObjectTag* other )
        {
            _character_id = other->_character_id;
            _depth = other->_depth;
            _clipDepth = other->_clipDepth;
		    _character = other->_character;
        }

		void copyTransform( PlaceObjectTag* other )
        {
            _transform = other->_transform;
        }

		void update(void)
		{
			if (_character)
				_character->update();
		}

        void play(bool enable) 
        {
			if (_character)
				_character->play(enable);
		}

		void gotoFrame(uint32_t frame, bool skipAction)
		{
			if (_character)
				_character->gotoFrame( frame,skipAction );
		}

		uint16_t depth()        const { return _depth; }
        uint16_t clipDepth()    const { return _clipDepth; }
		uint16_t characterID()  const { return _character_id; }
		//bool hasCharacter()     const { return 0!=_has_character; }
		bool hasMatrix()        const { return 0!=_has_matrix; }
		//bool hasMove()          const { return 0!=_has_move; }

		static ITag* create( TagHeader& header ) {
			return new PlaceObjectTag( header );
		}

	private:
		uint16_t	_character_id;
		uint16_t	_depth;
		uint16_t	_clipDepth;
		uint16_t	_has_matrix;
		//uint8_t		_has_character;
		//uint8_t		_has_move;
		Mode			_placeMode;

		ICharacter      *_character;
		std::string     _name;
        MATRIX          _transform;
        CXFORM          _cxform;
	};
}
#endif // __PlaceObject_h__