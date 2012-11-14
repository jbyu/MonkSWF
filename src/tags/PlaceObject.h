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

	class MovieClip {
	public:
		MovieClip( const FrameList& frames );

        void clearDisplayList(void);

		uint32_t getFrame( void ) { return _frame; }
		void gotoFrame( uint32_t frame );
        void setupFrame(const TagList& tags);

        void play( bool enable ) { _play = enable; }
        void update(void);
		void draw( SWF* swf );

        DisplayList& getDisplayList(void) { return _display_list; }

	private:
		const FrameList &_frame_list;
		const uint32_t	_frame_count;
		uint32_t		_frame;
        bool            _play;
		DisplayList		_display_list;
	};

//=========================================================================
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
			,_pMovieClip(NULL)
			,_name( "none" )
		{
		}
		
		virtual ~PlaceObjectTag()
		{
		}

        virtual bool process(SWF* swf ) { return true; }

        virtual void setup(MovieClip&);

		virtual bool read( Reader* reader, SWF* );

		virtual void print();

		virtual void draw( SWF* swf );

		void copyCharacter( PlaceObjectTag* other )
        {
            _character_id = other->_character_id;
            _depth = other->_depth;
            _clipDepth = other->_clipDepth;
		    _pShape = other->_pShape;
		    _pMovieClip = other->_pMovieClip;
        }

		void copyTransform( PlaceObjectTag* other )
        {
            _transform = other->_transform;
        }

		void update(void)
		{
			if (_pMovieClip)
				_pMovieClip->update();
		}

        void play(bool enable) 
        {
			if (_pMovieClip)
				_pMovieClip->play(enable);
		}

		void gotoFrame(uint32_t frame)
		{
			if (_pMovieClip)
				_pMovieClip->gotoFrame( frame );
		}

		uint16_t depth() const { return _depth; }
        uint16_t clipDepth() const { return _clipDepth; }
		uint16_t characterId() const { return _character_id; }
		bool hasCharacter() const { return 0!=_has_character; }
		bool hasMatrix() const { return 0!=_has_matrix; }
		bool hasMove() const { return 0!=_has_move; }

		static ITag* create( TagHeader* header ) {
			return (ITag*)(new PlaceObjectTag( *header ));
		}

	private:
		uint16_t	_character_id;
		uint16_t	_depth;
		uint16_t	_clipDepth;
		uint8_t		_has_matrix;
		uint8_t		_has_character;
		uint8_t		_has_move;

        DefineShapeTag  *_pShape;
		MovieClip		*_pMovieClip;
		std::string		_name;
        MATRIX          _transform;
        CXFORM          _cxform;
	};
}
#endif // __PlaceObject_h__