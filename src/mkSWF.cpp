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
#include "tags/ImportAssets2.h"
#include "tags/DefineSound.h"
#include "tags/DoAction.h"
#include "tags/FrameLabel.h"

namespace MonkSWF {
	
    Speaker *Speaker::spSpeaker = NULL;
    Renderer *Renderer::spRenderer = NULL;
    SWF::LoadAssetCallback SWF::_asset_loader = NULL;
	SWF::TagFactoryMap SWF::_tag_factories;

	SWF::SWF()
    	:MovieClip(_swf_data)
        ,_elapsedAccumulator(0.0f)
        ,_elapsedAccumulatorDuplicate(0.0f)
        ,_getURL(0)
	{
        _owner = this;
	}

	SWF::~SWF()
    {
        MovieClip::destroyFrames(_swf_data);
		// foreach movieclip in sprite
		MovieList::iterator mit = _movie_list.begin();
		while (_movie_list.end() != mit)
        {
            delete *mit;
            ++mit;
        }
    }

	MovieClip *SWF::createMovieClip(const ITag &tag)
	{
		const DefineSpriteTag& spriteImpl = (const DefineSpriteTag&) tag;
        MovieClip *movie = new MovieClip( this, spriteImpl.getMovieFrames() );
		_movie_list.push_back(movie);
		return movie;
	}

    //static function
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
        addFactory( TAG_IMPORT_ASSETS2, ImportAssets2Tag::create );

        addFactory( TAG_DEFINE_SOUND,   DefineSoundTag::create );
        addFactory( TAG_START_SOUND,    StartSoundTag::create );

        addFactory( TAG_DO_ACTION,      DoActionTag::create );
        addFactory( TAG_FRAME_LABEL,    FrameLabelTag::create );

		addFactory( TAG_SET_BACKGROUND_COLOR, SetBackgroundColorTag::create );
		return true;
	}
	
	bool SWF::read( Reader& reader ) {
		bool ret = _header.read( reader );
        if (false == ret)
            return false;

		createFrames(reader, *this, _swf_data);

        MK_ASSERT(getFrameCount() == _header.getFrameCount());
		this->gotoFrame( 0 );
		return true;
	}
	
	void SWF::print()
	{
		_header.print();
	}

    bool SWF::addAsset(uint16_t id, const char *name, bool import)
    {
        if (_asset_loader)
        {
            uint32_t handle = _asset_loader( name, import );
            if (0 != handle) {
                Asset asset = { import, handle };
                moAssets[id] = asset;
            } else {
                _library[name] = id;
            }
            return true;
        }
        return false;
    }

    MovieClip *SWF::duplicate(const char *name)
    {
        MovieClip *movie = NULL;
        SymbolDictionary::const_iterator it = _library.find(name);
        if (_library.end() != it)
        {
            ITag *tag = _dictionary[it->second];
            if ( tag->code() == TAG_DEFINE_SPRITE ) {
                // create a new instance for playback
				movie = createMovieClip( *tag );
                movie->createTransform();
                _duplicates.push_back(movie);
            }
        }
        return movie;
    }

 	void SWF::updateDuplicate( float delta )
	{
        _elapsedAccumulatorDuplicate += delta;
        const float secondPerFrame = getFrameRate();
        while (secondPerFrame <= _elapsedAccumulatorDuplicate)
        {
            _elapsedAccumulatorDuplicate -= secondPerFrame;
		    MovieList::iterator it = _duplicates.begin();
		    while (_duplicates.end() != it)
            {
                (*it)->update();
                ++it;
            }
        }
    }

    extern MATRIX3f goRootMatrix;
    extern CXFORM goRootCXForm;

	void SWF::drawDuplicate(void)
    {
        Renderer::getRenderer()->drawBegin();
	    MovieList::iterator it = _duplicates.begin();
	    while (_duplicates.end() != it)
        {
            MATRIX *xform = (*it)->getTransform();
            MK_ASSERT(xform);
            MATRIX3f origMTX = goRootMatrix, mtx;
            MATRIX3fSet(mtx, *xform); // convert matrix format
            MATRIX3fMultiply(goRootMatrix, mtx, goRootMatrix);

            (*it)->draw();
            ++it;

    		// restore old matrix
            goRootMatrix = origMTX;
        }
        Renderer::getRenderer()->drawEnd();
	}

	void SWF::drawMovieClip(MovieClip *movie, float alpha)
	{
		if (movie == NULL)
			return;

        MATRIX *xform = movie->getTransform();
        MK_ASSERT(xform);
        MATRIX3f origMTX = goRootMatrix, mtx;
        MATRIX3fSet(mtx, *xform); // convert matrix format
        MATRIX3fMultiply(goRootMatrix, mtx, goRootMatrix);
        float origAlpha = goRootCXForm.mult.a;
        goRootCXForm.mult.a = alpha;

		Renderer::getRenderer()->drawBegin();
        movie->draw();
        Renderer::getRenderer()->drawEnd();

    	// restore old matrix
        goRootMatrix = origMTX;
        goRootCXForm.mult.a = origAlpha;
	}

	void SWF::update( float delta )
	{
        _elapsedAccumulator += delta;
        const float secondPerFrame = getFrameRate();
        while (secondPerFrame <= _elapsedAccumulator)
        {
            _elapsedAccumulator -= secondPerFrame;
			MovieClip::update();
        }
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
        Renderer::getRenderer()->drawBegin();
		MovieClip::draw();
        Renderer::getRenderer()->drawEnd();
#ifdef USE_OPENVG
		// restore old matrix
		vgLoadMatrix( oldMatrix );
#endif
	}
}