/*
 *  mkTag.h
 *  MonkSWF
 *
 *  Created by Micah Pearlman on 3/20/09.
 *  Copyright 2009 MP Engineering. All rights reserved.
 *
 */

#ifndef __mkTag_h__
#define __mkTag_h__

#include "mkReader.h"
#include <vector>
#include <map>
//#include <iostream>

namespace MonkSWF {
	
	class SWF;
	class ITag;
	class IPlaceObjectTag;

	typedef std::map<uint16_t, IPlaceObjectTag*>	DisplayList;
	typedef std::vector<ITag*>						TagList;
	typedef std::vector<TagList*>					FrameList;
	
	class TagHeader {
	public:
		inline uint32_t code() const {
			return _code;
		}
		
		inline uint32_t length() const {
			return _length;
		}
		
		bool read( Reader *reader );
		void print();
		
	private:
		uint32_t _code;	
		uint32_t _length;
	};


	class ITag {
	public:
		virtual bool read( Reader* reader, SWF* _swf ) = 0;
		virtual void print() = 0;
		
		const TagHeader& header() {
			return _header;
		}
		
		uint32_t code() const {
			return _header.code();
		}
		
		int32_t length() const {
			return _header.length();
		}
		
		virtual ~ITag() {
		}

	protected:
		ITag( TagHeader& header ) 
		:	_header( header ) 
		{}
		
		TagHeader	_header;
	};
	
//=========================================================================
#define ENDTAG 0
	class EndTag : public ITag {
	public:
		EndTag( TagHeader& header )
		:	ITag( header )
		{}
		
		virtual ~EndTag() {}
		
		virtual bool read( Reader* reader, SWF* _swf ) { return true; }
		virtual void print() {
			MK_TRACE("END TAG\n");
		}
		
		static ITag* create( TagHeader* header ) {
			return (ITag*)(new EndTag( *header ));
		}				
	};
	
//=========================================================================
#define DEFINESHAPE 2
#define DEFINESHAPE2 22
#define DEFINESHAPE3 32
#define DEFINESHAPE4 83	
	class IDefineShapeTag : public ITag {
	public:
		virtual void draw( SWF* swf ) = 0;
		
		uint16_t shapeId() const {
			return _shape_id;
		}
	
	protected:
		IDefineShapeTag( TagHeader& header ) 
		:	ITag( header )
		,	_shape_id( 0 )
		{}
		
		virtual ~IDefineShapeTag() {}
		
		uint16_t			_shape_id;
	};
	
//=========================================================================
#define DEFINESPRITE 39
	class IDefineSpriteTag : public ITag {
	public:
		uint16_t spriteId() const {
			return _sprite_id;
		}
		
		uint16_t getFrameCount() const {
			return _frame_count;
		}

	protected:
		IDefineSpriteTag( TagHeader& header )
		:	ITag( header )
		,	_sprite_id( 0 )
		{}
		
		virtual ~IDefineSpriteTag()
		{}
	
		uint16_t		_sprite_id;
		uint16_t		_frame_count;
	};

//=========================================================================
#define PLACEOBJECT2	26 
	class IPlaceObjectTag : public ITag 
	{
	public:
    	virtual ~IPlaceObjectTag()
		{
		}

		virtual void play(void) = 0;
		virtual void setFrame(uint32_t frame) = 0;

		virtual void draw( SWF* swf ) = 0;

		virtual void copyCharacter( IPlaceObjectTag* other ) = 0;

		virtual void copyTransform( IPlaceObjectTag* other ) = 0;

		virtual void copyNoTransform( IPlaceObjectTag* other ) { // copies everything except transform 
			_depth = other->_depth;
			_character_id = other->_character_id;
			_has_character = other->_has_character;
			_has_move = other->_has_move;
		}
		
		uint16_t depth() const { return _depth; }
		uint16_t characterId() const { return _character_id; }
		bool hasCharacter() const { return 0!=_has_character; }
		bool hasMatrix() const { return 0!=_has_matrix; }
		bool hasMove() const { return 0!=_has_move; }
	
		int getFrame(void) const { return _frame; }


		static bool compare( IPlaceObjectTag* a, IPlaceObjectTag* b ) {
			return a->depth() < b->depth();
		}

	protected:
		IPlaceObjectTag( TagHeader& header ) 
		:ITag( header )
		,_frame(0)
		,_depth( 0 )
		,_character_id( 0xffff )
		,_has_matrix( 0 )
		,_has_character( 0 )
		,_has_move( 0 )
		{}
		
		int			_frame;
		uint16_t	_depth;
		uint16_t	_character_id;
		uint8_t		_has_matrix;
		uint8_t		_has_character;
		uint8_t		_has_move;
	};
	
//=========================================================================
#define SHOWFRAME 1	
	class IShowFrameTag : public ITag {
	protected:
		IShowFrameTag( TagHeader& header ) 
		:	ITag( header )
		{}
		
		virtual ~IShowFrameTag() {
		}
	};

//=========================================================================
#define REMOVEOBJECT	5
#define REMOVEOBJECT2	28
	class IRemoveObjectTag : public ITag {
	public:	
		uint16_t characterId() const {
			return _character_id;
		}
		uint16_t depth() const {
			return _depth;
		}
		virtual ~IRemoveObjectTag() {
		}

	protected:
		IRemoveObjectTag( TagHeader& header ) 
		:	ITag( header )
		,	_character_id( 0 )
		{}
		
	protected:
		uint16_t	_character_id;
		uint16_t	_depth;
	};
	
}
#endif // __mkTag_h__