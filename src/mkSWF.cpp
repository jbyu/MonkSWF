/*
 *  mkSWF.cpp
 *  MonkSWF
 *
 *  Created by Micah Pearlman on 3/20/09.
 *  Copyright 2009 MP Engineering. All rights reserved.
 *
 */

#include "mkSWF.h"
#include "tags/DefineShape.h"
#include "tags/PlaceObject.h"
#include "tags/ShowFrame.h"
#include "tags/RemoveObject.h"
#include "tags/DefineSprite.h"
#include "tags/SetBackgroundColor.h"
#include "tags/ExportAssets.h"
#include "tags/DefineSound.h"
#include "tags/DoAction.h"

namespace MonkSWF {
	
    Speaker *Speaker::spSpeaker = NULL;
    Renderer *Renderer::spRenderer = NULL;
    SWF::LoadAssetCallback SWF::_asset_loader = NULL;
	SWF::TagFactoryMap SWF::_tag_factories;

	SWF::SWF()
	:_elapsedAccumulator(0.0f)
	,mpMovieClip(NULL)
	{
	}

	SWF::~SWF()
    {
		// foreach frames in sprite
		FrameList::iterator it = _frame_list.begin();
		while (_frame_list.end() != it)
        {
			// foreach tags in frame
			TagList *_tag_list = *it;
            TagList::iterator tag_it = _tag_list->begin();
            while(_tag_list->end() != tag_it)
            {
                delete *tag_it;
                ++tag_it;
            }
			// delete container
            delete *it;
            ++it;
        }
		// foreach movieclip in sprite
		MovieList::iterator mit = _movie_list.begin();
		while (_movie_list.end() != mit)
        {
            delete *mit;
            ++mit;
        }
		// swf movie
		delete mpMovieClip;
		mpMovieClip = NULL;
    }

	MovieClip *SWF::createMovieClip(const ITag &tag)
	{
		const DefineSpriteTag& spriteImpl = (const DefineSpriteTag&) tag;
		MovieClip *movie = new MovieClip( spriteImpl.getFrameList() );
		_movie_list.push_back(movie);
		return movie;
	}

	bool SWF::initialize(LoadAssetCallback func) {
        _asset_loader = func;
		// setup the factories
		addFactory( TAG_END,			EndTag::create );
		
		addFactory( TAG_DEFINE_SHAPE,	DefineShapeTag::create );		// DefineShape
		addFactory( TAG_DEFINE_SHAPE2,	DefineShapeTag::create );		// DefineShape2
		addFactory( TAG_DEFINE_SHAPE3,	DefineShapeTag::create );		// DefineShape3
		addFactory( TAG_DEFINE_SHAPE4,	DefineShapeTag::create );		// DefineShape4

		addFactory( TAG_DEFINE_SPRITE,	DefineSpriteTag::create );
		addFactory( TAG_PLACE_OBJECT2,	PlaceObjectTag::create );
		addFactory( TAG_REMOVE_OBJECT2,	RemoveObjectTag::create );
		addFactory( TAG_SHOW_FRAME,		ShowFrameTag::create );
		
        addFactory( TAG_EXPORT_ASSETS,  ExportAssetsTag::create );
        addFactory( TAG_DEFINE_SOUND,   DefineSoundTag::create );
        addFactory( TAG_START_SOUND,    StartSoundTag::create );

        addFactory( TAG_DO_ACTION,      DoActionTag::create );

		addFactory( TAG_SET_BACKGROUND_COLOR, SetBackgroundColorTag::create );
		return true;
	}
	
	bool SWF::read( Reader* reader ) {
		_reader = reader;
		bool ret = _header.read( reader );
        if (false == ret)
            return false;
		
		// get the first tag
        TagHeader oHeader;
		TagHeader *tag_header = &oHeader;//new TagHeader();
		tag_header->read( reader );
				
		// get all tags and build frame lists
		TagList* frame_tags = new TagList;
		while( tag_header->code() != 0 ) { // while not the end tag 
			ITag* tag = NULL;
            const uint32_t code = tag_header->code();
			TagFactoryFunc factory = getTagFactory( code );
			if ( factory ) {
				tag = factory( tag_header );

				int32_t end_pos = reader->getCurrentPos() + tag->length();
				tag->read( reader, this );
				tag->print();
				reader->align();

				int32_t dif = end_pos - reader->getCurrentPos();
				if( dif != 0 ) {
					MK_TRACE("WARNING: tag not read correctly. trying to skip.\n");
					reader->skip( dif );
				}
                // process tag
                bool keepTag = tag->process(this);
                if (keepTag) {
                    frame_tags->push_back( tag );
                } else {
                    delete tag;
                }
                // create a new frame
                if ( TAG_SHOW_FRAME == code )
                {
					_frame_list.push_back( frame_tags );
					frame_tags = new TagList;
                }
			} else { // no registered factory so skip this tag
				MK_TRACE("*** SKIP *** ");
				tag_header->print();
				reader->skip( tag_header->length() );
				//delete tag_header;
			}
			
			//tag_header = new TagHeader();
			tag_header->read( reader );
		}

		delete frame_tags;
		MK_ASSERT(_frame_list.size() == getFrameCount());
		mpMovieClip = new MovieClip( _frame_list );
		mpMovieClip->gotoFrame( 0 );
		return true;
	}
	
	void SWF::print()
	{
		_header.print();
	}
	
	void SWF::play( float delta )
	{
        _elapsedAccumulator += delta;
        const float secondPerFrame = getFrameRate();
        while (secondPerFrame <= _elapsedAccumulator)
        {
            _elapsedAccumulator -= secondPerFrame;
			mpMovieClip->update();
		}
    }

	void SWF::step( void )
	{
		mpMovieClip->gotoFrame( mpMovieClip->getFrame() + 1 );
    }

	int SWF::getCurrentFrame( void ) const
	{
        return mpMovieClip->getFrame();
    }


	void SWF::draw(void) {
#ifdef USE_OPENVG
		// make sure we use even odd fill rule
		vgSeti( VG_FILL_RULE, VG_EVEN_ODD );
		// clear the screen
		VGfloat clearColor[] = {1,1,1,1};
		vgSetfv(VG_CLEAR_COLOR, 4, clearColor);
		vgClear(0,0,_header.getFrameWidth(), _header.getFrameHeight() );
		
		vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
		VGfloat oldMatrix[9];
		vgGetMatrix( oldMatrix );
		vgMultMatrix( _rootTransform );
#endif

		mpMovieClip->draw( this );

#ifdef USE_OPENVG
		// restore old matrix
		vgLoadMatrix( oldMatrix );
#endif
	}
}