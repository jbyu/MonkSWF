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

	void setup_frame(DisplayList& display, const TagList& tagse);

	class MovieClip {
	public:
		MovieClip( const FrameList& frames );

		uint32_t getFrame( void ) { return _frame; }
		void setFrame( uint32_t frame );
        //void reset( void ) { _frame = 0xffffffff; }
		void draw( SWF* swf );
        void play( bool enable )
        {
            if (enable ^ _play)
                _frame = 0xffffffff; 
            _play = enable;
        }
		void step( void )
		{
            if (_play)
                setFrame(_frame + 1);
		}

	private:
		const FrameList &_frame_list;
		const uint32_t	_frame_count;
		uint32_t		_frame;
        bool            _play;
		DisplayList		_display_list;
	};

	class PlaceObject2Tag : public IPlaceObjectTag {
	public:
		PlaceObject2Tag( TagHeader& h ) 
			:IPlaceObjectTag( h )
			,_pMovieClip(NULL)
			,_frame(0)
            ,_texture(0)
			,_name( "__none__" )
		{
		}
		
		virtual ~PlaceObject2Tag()
		{
		}

		virtual bool read( Reader* reader, SWF* );

		virtual void print();

		virtual void draw( SWF* swf );

		virtual void copyCharacter( IPlaceObjectTag* other );

		virtual void copyTransform( IPlaceObjectTag* other );

		void update(void)
		{
			if (_pMovieClip)
				_pMovieClip->step();
		}

        void play(bool enable) 
        {
			if (_pMovieClip)
				_pMovieClip->play(enable);
		}

		void setFrame(uint32_t frame)
		{
			if (_pMovieClip)
				_pMovieClip->setFrame( frame );
		}

		static ITag* create( TagHeader* header ) {
			return (ITag*)(new PlaceObject2Tag( *header ));
		}

	private:
		MovieClip		*_pMovieClip;
		int				_frame;
        unsigned int    _texture;
		std::string		_name;
        MATRIX          _transform;
        CXFORM          _cxform;
	};
}
#endif // __PlaceObject_h__