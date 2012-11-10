/*
 *  mkHeader.cpp
 *  MonkSWF
 *
 *  Created by Micah Pearlman on 3/19/09.
 *  Copyright 2009 MP Engineering. All rights reserved.
 *
 */

#include "mkHeader.h"

#ifdef USE_ZLIB
#include <zlib.h>
#define MAX_BUFFER 1000000
#endif


using namespace std;

short int endianSwap16(short int val)
{
	long int i = 1;
	const char *p = (const char *) &i;
	if (p[0] == 1)  // Lowest address contains the least significant byte
	{
		return (((val & 0xff00) >> 8) | ((val & 0x00ff) << 8));
	}
	return val;
}


namespace MonkSWF {
	
	bool Header::read( Reader* reader ) {
	
		_signature[0] = reader->get<uint8_t>();
		_signature[1] = reader->get<uint8_t>();
		_signature[2] = reader->get<uint8_t>();
		
		_version = reader->get<uint8_t>();
		
		_file_length = reader->get<uint32_t>();
		
		if ( _signature[0] == 'C' ) {
#ifdef USE_ZLIB
			// compressed file
			_outputBuffer = (char*)malloc( _file_length );
			
			z_stream stream;
			static unsigned char inputBuffer[ MAX_BUFFER ];
			int status;
			
			stream.avail_in = 0;
			stream.next_in = inputBuffer;
			stream.next_out = (Bytef*) _outputBuffer;
			stream.zalloc = (alloc_func) NULL;
			stream.zfree = (free_func) NULL;
			stream.opaque = (voidpf) 0;
			stream.avail_out = _file_length;
			
			status = inflateInit( &stream );
			if( status != Z_OK ) {
				fprintf( stderr, "Error decompressing SWF: %s\n", stream.msg );
				return false;
			}
			
			do {
				if( stream.avail_in == 0 ) {
					stream.next_in = inputBuffer;
					stream.avail_in = reader->getBytes( MAX_BUFFER, inputBuffer );
				}
				
				if( stream.avail_in == 0 ) break;
				
				status = inflate( &stream, Z_SYNC_FLUSH );
			} while( status == Z_OK );
			
			if( status != Z_STREAM_END && status != Z_OK ) {
				fprintf( stderr, "Error decompressing SWF: %s\n", stream.msg );
				return false;
			}
			
			reader->setNewData( _outputBuffer, _file_length );
#else
            MK_ASSERT(0);
#endif
		}
		
		
		// get the bound rectangle
        reader->getRectangle(_frame_size);
		uint16_t fr = reader->get<uint16_t>();
		_frame_rate = 1.f/(fr>>8);
		_frame_count = reader->get<uint16_t>(); 
				
		return true;
	}
	
	void Header::print() {
		MK_TRACE("signature:\t%c%c%c\n",	_signature[0], _signature[1], _signature[2]);
		MK_TRACE("version:\t\t%d\n",		 _version); 
		MK_TRACE("file length:\t%d\n",		_file_length);
		MK_TRACE("frame width:\t%f\n",		getFrameWidth());
		MK_TRACE("frame height:\t%f\n",		getFrameHeight());
		MK_TRACE("frame rate:\t%f\n",		_frame_rate);
		MK_TRACE("frame count:\t%d\n",		_frame_count);
	}
	
}