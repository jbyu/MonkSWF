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
#include <string>

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
	    TAG_DO_ACTION           = 12,
	    TAG_DEFINE_FONT_INFO,
	    TAG_DEFINE_SOUND        = 14,
	    TAG_START_SOUND         = 15,
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

	// forward declaration
	class SWF;
	class ITag;
	class PlaceObjectTag;
    class MovieClip;

	typedef std::map<uint16_t, PlaceObjectTag*>	DisplayList;
    typedef std::map<std::string, uint32_t>     LabelList;
	typedef std::vector<ITag*>					TagList;
	typedef std::vector<TagList*>				FrameList;

	// movieclip information
    struct MovieFrames
    {
        FrameList _frames;
        LabelList _labels;
        RECT      _rectangle;
    };

	// import asset
    struct Asset
    {
        bool        import;
        uint32_t    handle;
		int			typeId;
    };
    const Asset kNULL_ASSET = {false, 0};


	struct Event {
		enum Code {
			INVALID = 0,

			// These are for buttons & sprites.
			PRESS,
			RELEASE,
			RELEASE_OUTSIDE,
			ROLL_OVER,
			ROLL_OUT,
			DRAG_OVER,
			DRAG_OUT,
			KEY_PRESS,

			// These are for sprites only.
			INITIALIZE,
			LOAD,
			UNLOAD,
			ENTER_FRAME,
			MOUSE_DOWN,
			MOUSE_UP,
			MOUSE_MOVE,
			KEY_DOWN,
			KEY_UP,
			DATA,

			CONSTRUCT,
			SETFOCUS,
			KILLFOCUS,			

			// MovieClipLoader events
			ONLOAD_COMPLETE,
			ONLOAD_ERROR,
			ONLOAD_INIT,
			ONLOAD_PROGRESS,
			ONLOAD_START,

			// sound
			ON_SOUND_COMPLETE,

			EVENT_COUNT
		};
		unsigned char	m_id;
		unsigned char	m_key_code;
	};

	// character interface for display list
	class ICharacter {
	public:
		const static uint32_t kFRAME_MAXIMUM = 0xffffffff;

        virtual const RECT& getRectangle(void) const = 0;
		virtual void draw(void) = 0;
		virtual void update(void) = 0;
        virtual void play( bool enable ) = 0;
		virtual void gotoFrame( uint32_t frame ) = 0;
		virtual ICharacter* getTopMost(float localX, float localY) = 0;
		virtual void onEvent(Event::Code) {}
	};

//=========================================================================

	class TagHeader {
	public:
		inline uint32_t code() const { return _code; }
		inline uint32_t length() const { return _length; }
		
		bool read( Reader& reader );
		void print();

#ifdef SWF_DEBUG
        const char* name(void) const { return _tagNames[_code]; }
        static int initialize(void);
        static std::map<uint32_t, const char*> _tagNames;
#else
        const char* name(void) const { return "unknow"; }
#endif

	private:
		uint32_t _code;	
		uint32_t _length;
	};

//=========================================================================

	class ITag {
	public:
        // return false to delete this tag
        // return true to keep this tag
		virtual bool read( Reader& reader, SWF& swf, MovieFrames& data ) = 0;
		virtual void print() = 0;
        virtual void setup( MovieClip& ) {}

		inline uint32_t code() const { return _header.code(); }
		inline int32_t length() const { return _header.length(); }
		const TagHeader& header() { return _header; }

        virtual ~ITag() {}

	protected:
		ITag( TagHeader& header ) 
		    :_header( header ) 
		{}
		
		TagHeader	_header;
	};

//=========================================================================

	class EndTag : public ITag {
	public:
		EndTag( TagHeader& header )
            :ITag( header )
		{}
		
		virtual ~EndTag() {}
		
		virtual bool read( Reader& reader, SWF& swf, MovieFrames& data ) { return false; }
		virtual void print() { MK_TRACE("END TAG\n"); }
		
		static ITag* create( TagHeader& header ) {
			return (ITag*)(new EndTag(header));
		}				
	};

//=========================================================================

}
#endif // __mkTag_h__