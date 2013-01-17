/*
 *  reader.h
 *  MonkSWF
 *
 *  Created by Micah Pearlman on 3/18/09.
 *  Copyright 2009 MP Engineering. All rights reserved.
 *
 */

#ifndef __reader_h__
#define __reader_h__

#include "mkCommon.h"
#include "mkTypes.h"
#include <string>

namespace MonkSWF {
	
	class Reader {
	public:
	
		Reader( char* data, int32_t sz )
			:	_data( data )
			,	_sz( sz )
			,	_cur( 0 )
			,	_bitpos( 0 )
		{}
		
		template< class T> T get() {
			if( _bitpos > 0 && _bitpos < 8)
				_cur++;
			_bitpos = 0;
			T *ret = (T*)(_data + _cur);
			_cur += sizeof( T );
			return *ret;
		}
		
		inline void skip( int32_t bytes ) {
			_cur += bytes;
			_bitpos = 0;
		}
		
		inline void align() {
			if( _bitpos > 0 && _bitpos < 8)
				_cur++;
			_bitpos = 0;
		}
		
		inline uint32_t getbit() {

			uint32_t v = (_data[_cur] >> (7 - _bitpos++)) & 1;
			if( _bitpos == 8 )
				get<uint8_t>();
			
			return v;
		}
		
		inline uint32_t getbits( uint8_t nbits ) {
			uint32_t val = 0;
			for( int t = 0; t < nbits; t++ )
			{
				val <<= 1;
				val |= getbit();
			}
			return val;
		}
		
		inline int32_t getsignedbits( uint8_t nbits ) {
			uint32_t res = getbits( nbits );
			if ( res & ( 1 << ( nbits - 1 ) ) ) 
				res |= (0xffffffff << nbits);
				  
			return (int32_t)res;			
		}
		
		inline void getRectangle( RECT& rect ) {
			const int32_t nbits = (int32_t) getbits( 5 );
			rect.xmin = getsignedbits( nbits ) * SWF_INV_TWIPS;
			rect.xmax = getsignedbits( nbits ) * SWF_INV_TWIPS;
			rect.ymin = getsignedbits( nbits ) * SWF_INV_TWIPS;
			rect.ymax = getsignedbits( nbits ) * SWF_INV_TWIPS;
		}
		
		inline void getMatrix( MATRIX& m ) {
			int32_t nbits;
			if ( getbits( 1 ) )	{	// has scale
				nbits = getbits( 5 );
				m.sx = (float)getsignedbits( nbits )*SWF_INV_MATRIX;
				m.sy = (float)getsignedbits( nbits )*SWF_INV_MATRIX;
			} else {
				m.sx = m.sy = 1.0f;
			}

			if ( getbits( 1 ) ) { // has rotation
				nbits = getbits( 5 );
				m.r0 = (float)getsignedbits( nbits )*SWF_INV_MATRIX;
				m.r1 = (float)getsignedbits( nbits )*SWF_INV_MATRIX;
			} else {
				m.r0 = m.r1 = 0.0f;	
			}

			nbits = getbits( 5 );
			m.tx = getsignedbits( nbits ) * SWF_INV_TWIPS;
			m.ty = getsignedbits( nbits ) * SWF_INV_TWIPS;
		}

        inline void getColor( COLOR4f& color, uint8_t nbits ) {
			color.r = getsignedbits( nbits ) * SWF_INV_CXFORM;
			color.g = getsignedbits( nbits ) * SWF_INV_CXFORM;
			color.b = getsignedbits( nbits ) * SWF_INV_CXFORM;
			color.a = getsignedbits( nbits ) * SWF_INV_CXFORM;
        }
		
		inline int32_t getCurrentPos() const {
			return _cur;
		}
		
		inline uint8_t getBitPos() const {
			return _bitpos;
		}
		
		int32_t getBytes( int32_t s, uint8_t* b ) {
			if ( _cur + s > _sz ) {
				s = _sz - _cur;
				if ( s == 0 ) {
					return 0;
				}
			}
			memcpy( b, &_data[_cur], s );
			_cur += s;
			return s;
		}
		
		void setNewData( char* data, int32_t sz ) {
			_data = data;
			_sz = sz;
			_cur = 0;
			_bitpos = 0;
		}

        inline const char* getString() {
			const char *src = (const char *)&_data[_cur];
			size_t len = strlen(src) + 1;
			_cur += len;
            return src;
            /*
			char *dst = new char[len];
			strcpy(dst, src);
			return( dst );
            */
		}

    private:
	
		char*		_data;
		int32_t		_sz;		// total size of the buffer
		int32_t		_cur;		// current ptr/idx into buffer
		uint8_t		_bitpos;	// for reading bits
	};
	
//	template<> char Reader::get<char>();
}
#endif // __reader_h__