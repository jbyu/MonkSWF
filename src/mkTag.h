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

    typedef enum SWF_TagNames {
        TAG_END             = 0,
	    TAG_SHOW_FRAME      = 1,
	    TAG_DEFINE_SHAPE    = 2,
	    TAG_PLACE_OBJECT    = 4,
	    TAG_REMOVE_OBJECT   = 5,
	    TAG_DEFINE_BITS,
	    TAG_DEFINE_BUTTON,
	    TAG_JPEG_TABLES,
	    TAG_SET_BACKGROUND_COLOR = 9,
	    TAG_DEFINE_FONT,
	    TAG_DEFINE_TEXT,
	    TAG_DO_ACTION,
	    TAG_DEFINE_FONT_INFO,
	    TAG_DEFINE_SOUND,
	    TAG_START_SOUND,
	    TAG_DEFINE_BUTTON_SOUND = 17,
	    TAG_SOUND_STREAM_HEAD,
	    TAG_SOUND_STREAM_BLOCK,
	    TAG_DEFINE_BITS_LOSSLESS,
	    TAG_DEFINE_BITS_JPEG2,
	    TAG_DEFINE_SHAPE2       = 22,
	    TAG_DEFINE_BUTTON_CXFORM,
	    TAG_PROTECT,
	    TAG_PLACE_OBJECT2       = 26,
	    TAG_REMOVE_OBJECT2      = 28,
	    TAG_DEFINE_SHAPE3       = 32,
	    TAG_DEFINE_TEXT2,
	    TAG_DEFINE_BUTTON2,
	    TAG_DEFINE_BITS_JPEG3,
	    TAG_DEFINE_BITS_LOSSLESS2,
	    TAG_DEFINE_EDIT_TEXT,
	    TAG_DEFINE_SPRITE       = 39,
	    TAG_FRAME_LABEL         = 43,
	    TAG_SOUND_STREAM_HEAD2  = 45,
	    TAG_DEFINE_MORPH_SHAPE,
	    TAG_DEFINE_FONT2        = 48,
	    TAG_EXPORT_ASSETS       = 56,
	    TAG_IMPORT_ASSETS,
	    TAG_ENABLE_DEBUGGER,
	    TAG_DO_INIT_ACTION,
	    TAG_DEFINE_VIDEO_STREAM,
	    TAG_VIDEO_FRAME,
	    TAG_DEFINE_FONT_INFO2,
	    TAG_ENABLE_DEBUGGER2    = 64,
	    TAG_SCRIPT_LIMITS,
	    TAG_SET_TAB_INDEX,
	    TAG_FILE_ATTRIBUTES     = 69,
	    TAG_PLACE_OBJECT3,
	    TAG_IMPORT_ASSETS2,
	    TAG_DEFINE_FONT_ALIGN_ZONES = 73,
	    TAG_DEFINE_CSM_TEXT_SETTINGS,
	    TAG_DEFINE_FONT3,
	    TAG_SYMBOL_CLASS,
	    TAG_METADATA,
	    TAG_DEFINE_SCALING_GRID,
	    TAG_DO_ABC                              = 82,
	    TAG_DEFINE_SHAPE4                       = 83,
	    TAG_DEFINE_MORPH_SHAPE2,
	    TAG_DEFINE_SCENE_AND_FRAME_LABEL_DATA   = 86,
	    TAG_DEFINE_BINARY_DATA,
	    TAG_DEFINE_FONT_NAME        = 88,
	    TAG_DEFINE_START_SOUND2     = 89,
	    TAG_DEFINE_BITS_JPEG4       = 90,
	    TAG_DEFINE_FONT4            = 91,
    } SWF_TAG_NAMES;

	class SWF;
	class ITag;
	class IPlaceObjectTag;

	typedef std::map<uint16_t, IPlaceObjectTag*>	DisplayList;
	typedef std::vector<ITag*>						TagList;
	typedef std::vector<TagList*>					FrameList;
	
//=========================================================================
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

//=========================================================================
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
	class IDefineShapeTag : public ITag {
	public:
		virtual void draw( SWF* swf, const CXFORM& ) = 0;
		
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
	class IPlaceObjectTag : public ITag 
	{
	public:
    	virtual ~IPlaceObjectTag()
		{
		}

		virtual void update(void) = 0;
        virtual void play(bool enable) = 0;
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

        static bool compare( IPlaceObjectTag* a, IPlaceObjectTag* b ) {
			return a->depth() < b->depth();
		}

	protected:
		IPlaceObjectTag( TagHeader& header ) 
		:ITag( header )
		,_depth( 0 )
		,_character_id( 0xffff )
		,_has_matrix( 0 )
		,_has_character( 0 )
		,_has_move( 0 )
		{}
		
		uint16_t	_depth;
		uint16_t	_character_id;
		uint8_t		_has_matrix;
		uint8_t		_has_character;
		uint8_t		_has_move;
	};
	
//=========================================================================
	class IShowFrameTag : public ITag {
	protected:
		IShowFrameTag( TagHeader& header ) 
		:	ITag( header )
		{}
		
		virtual ~IShowFrameTag() {
		}
	};

//=========================================================================
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