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

    class Renderer
    {
    public:
        virtual ~Renderer() {}
        virtual void applyTexture( unsigned int texture ) = 0;
        virtual void applyTransform( const PVRTMATRIX3f& mtx ) = 0;
        virtual void drawQuad( const RECT& rect ) = 0;
        virtual unsigned int getTexture( const char *filename ) = 0;
    };
	
	class SWF {
	public:

#ifdef USE_BOOST
		typedef boost::shared_ptr<SWF> SmartPtr;
		static SWF::SmartPtr create( ) {
			return boost::make_shared<SWF>( );
		}
#endif

		// factory function prototype
		typedef ITag* (*TagFactoryFunc)( TagHeader* );
	
		SWF(); 
        ~SWF();
		
        Renderer *GetRenderer(void) { return mpRenderer; }
		bool initialize(Renderer*renderer);
		bool read( Reader *reader );
		void print();
		
		void addFactory( uint32_t tag_code, TagFactoryFunc factory ) {
			_tag_factories[ tag_code ] = factory;
		}
		
		TagFactoryFunc getTagFactory( uint32_t tag_code ) {
			TagFactoryMap::iterator factory = _tag_factories.find( tag_code );

			if (_tag_factories.end() != factory)
				return factory->second;

			return NULL;
		}
		
		IDefineShapeTag* getShape( uint16_t i ) {		// todo: change to map instead of vector
			ShapeDictionary::iterator it = _shape_dictionary.find( i );
			if (it == _shape_dictionary.end() ) {
				return NULL;
			}
			return it->second;
		}
		
		int32_t numShapes() const {
			return _shape_dictionary.size();
		}

		void addShape( IDefineShapeTag* shape, uint16_t cid ) {
			_shape_dictionary[cid] = shape;
		}

#if 0		
		IDefineShapeTag* shapeAt( int32_t idx ) {
			return _shape_dictionary[idx];
		}
		IDefineSpriteTag* spriteAt( uint32_t idx ) {
			if ( _sprite_dictionary.size() == 0 || idx >= _sprite_dictionary.size() ) {
				return 0;
			}
			SpriteDictionary::iterator i = _sprite_dictionary.begin();
			advance(i, idx);
			return i->second;
		}
#endif

		IDefineSpriteTag* getSprite( uint16_t i ) {
			SpriteDictionary::iterator it = _sprite_dictionary.find( i );
			if (it == _sprite_dictionary.end() ) {
				return NULL;
			}
			return it->second;
		}

		int32_t numSprites() const {
			return _sprite_dictionary.size();
		}
		
		void addSprite( IDefineSpriteTag* sprite, uint16_t cid ) {
			_sprite_dictionary[cid] = sprite;
		}

		/*
		int32_t numFrames() const {
			return _frame_list.size();
		}
		*/
		Reader* reader() const {
			return _reader;
		}
		
		void drawFrame( int32_t frame_idx );
		void draw(void) {drawFrame(_frame);} 
		void play(float delta);

		float getFrameWidth() const {
			return _header.getFrameWidth();
		}
		float getFrameHeight() const {
			return _header.getFrameHeight();
		}
		float getFrameRate() const {
			return _header.getFrameRate();
		}
		int getFrameCount() const {
			return _header.getFrameCount();
		}
		
		void setOffsetTranslate( const float t[2] ) {
			_offsetTranslate[0] = t[0];
			_offsetTranslate[1] = t[1];
		}

		void getOffsetTranslate( float t[2] ) const  {
			t[0] = _offsetTranslate[0];
			t[1] = _offsetTranslate[1];
		}

		void setOffsetScale( float s ) {
			_offsetScale = s;
		}

		float getOffsetScale() const {
			return _offsetScale;
		}
		
		void setRootTransform( const float t[9] ) {
			for( int i = 0; i < 9; i++ )
				_rootTransform[i] = t[i];
		}
		
		void getRootTransform( float t[9] ) {
			for( int i = 0; i < 9; i++ )
				t[i] = _rootTransform[i];
		}
		
	private:
		typedef std::map< uint32_t, SWF::TagFactoryFunc >	TagFactoryMap;
		typedef std::map< uint16_t, IDefineShapeTag* >		ShapeDictionary;
		typedef std::map< uint16_t, IDefineSpriteTag* >		SpriteDictionary;
		//typedef std::vector< DisplayList* >					FrameList;
		
		ShapeDictionary		_shape_dictionary;
		SpriteDictionary	_sprite_dictionary;
		FrameList			_frame_list;
		DisplayList			_display_list;


		Header				_header;
		TagFactoryMap		_tag_factories;
		Reader*				_reader;
		int32_t				_frame;
		
		// offset
		float				_offsetTranslate[2];
		float				_offsetScale;
		float				_rootTransform[9];
		float               _accumulator;

        Renderer            *mpRenderer;
	};
}
#endif // __mkSWF_h__