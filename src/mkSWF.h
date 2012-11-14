/*
 *  mkSWF.h
 *  MonkSWF
 *
 *  Created by Micah Pearlman on 3/20/09.
 *  Copyright 2009 MP Engineering. All rights reserved.
 *
 */

#ifndef __mkSWF_h__
#define __mkSWF_h__

#include "mkReader.h"
#include "mkHeader.h"
#include "mkTag.h"
#include <map> 
#include <vector>
#include <list>

#ifdef USE_BOOST
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#endif

using namespace std;

namespace MonkSWF {

	class MovieClip;

//=========================================================================
    class Renderer
    {
        static Renderer     *spRenderer;

    public:
        virtual ~Renderer() {}
        virtual void applyTexture( unsigned int texture ) = 0;
        virtual void applyTransform( const MATRIX3f& mtx ) = 0;
        //virtual void applyPassMult( const CXFORM& color  ) = 0;
        //virtual void applyPassAdd( const CXFORM& color  ) = 0;
        virtual void drawQuad(const RECT& rect, const CXFORM&) = 0;

        virtual void maskBegin(void) = 0;
        virtual void maskEnd(void) = 0;
        virtual void maskOffBegin(void) = 0;
        virtual void maskOffEnd(void) = 0;

        virtual unsigned int getTexture( const char *filename ) = 0;

        static Renderer* getRenderer(void) { return spRenderer; }
        static void setRenderer(Renderer *r) { spRenderer = r; }
    };
//=========================================================================
   class Speaker
   {
        static Speaker *spSpeaker;

    public:
        virtual ~Speaker() {}

        virtual unsigned int getSound( const char *filename ) = 0;
        virtual void playSound( unsigned int sound, bool stop, bool noMultiple, bool loop ) = 0;

        static Speaker* getSpeaker(void) { return spSpeaker; }
        static void setSpeaker(Speaker *r) { spSpeaker = r; }
   };

//=========================================================================
	class SWF
    {
	public:

#ifdef USE_BOOST
		typedef boost::shared_ptr<SWF> SmartPtr;
		static SWF::SmartPtr create( ) {
			return boost::make_shared<SWF>( );
		}
#endif

		// factory function prototype
		typedef ITag* (*TagFactoryFunc)( TagHeader* );

        typedef uint32_t (*LoadAssetCallback)( const char *name );

		SWF(); 
        ~SWF();
		
		static bool initialize(LoadAssetCallback);
		bool read( Reader *reader );
		void print();
		
		static void addFactory( uint32_t tag_code, TagFactoryFunc factory ) { _tag_factories[ tag_code ] = factory;	}

		static TagFactoryFunc getTagFactory( uint32_t tag_code ) {
			TagFactoryMap::iterator factory = _tag_factories.find( tag_code );
			if (_tag_factories.end() != factory)
				return factory->second;
			return NULL;
		}
		
		void  addCharacter( ITag* tag, uint16_t cid ) { _dictionary[cid] = tag; }
		ITag* getCharacter( uint16_t i ) {
			CharacterDictionary::iterator it = _dictionary.find( i );
			if (it == _dictionary.end() ) {
				return NULL;
			}
			return it->second;
		}
		
		Reader* reader() const { return _reader; }
		
		void draw(void);
        void step(void);
		void play(float delta);

		float getFrameWidth() const     { return _header.getFrameWidth(); }
		float getFrameHeight() const    { return _header.getFrameHeight(); }
		float getFrameRate() const      { return _header.getFrameRate(); }
		int getFrameCount() const       { return _header.getFrameCount(); }
		int getCurrentFrame(void) const;

        COLOR4f& getBackgroundColor(void) { return _bgColor; }

		MovieClip *createMovieClip(const ITag &tag);

        uint32_t getAsset(uint16_t id)
        {
            AssetDictionary::iterator it = moAssets.find(id);
            if (moAssets.end() != it)
                return it->second;
            return 0;
        }
        
        bool addAsset(uint16_t id, const char *name)
        {
            if (_asset_loader)
            {
                moAssets[id] = _asset_loader( name );
                return true;
            }
            return false;
        }

    private:
		typedef std::map< uint32_t, TagFactoryFunc >    TagFactoryMap;
		typedef std::map< uint16_t, ITag* >             CharacterDictionary;
        typedef std::map< uint16_t, uint32_t >          AssetDictionary;
		typedef std::vector< MovieClip* >               MovieList;

		float               _elapsedAccumulator;
    	CharacterDictionary	_dictionary;
		FrameList			_frame_list;
		MovieList			_movie_list;
		Header				_header;

        AssetDictionary     moAssets;
        COLOR4f             _bgColor;
		Reader*				_reader;
		MovieClip			*mpMovieClip;
		static TagFactoryMap        _tag_factories;
        static LoadAssetCallback    _asset_loader;
	};
}
#endif // __mkSWF_h__