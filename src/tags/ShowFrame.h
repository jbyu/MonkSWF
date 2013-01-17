/*
 *  ShowFrame.h
 *  MonkSWF
 *
 *  Created by Micah Pearlman on 5/6/09.
 *  Copyright 2009 MP Engineering. All rights reserved.
 *
 */

#ifndef __ShowFrame_h__
#define __ShowFrame_h__

#include "mkTag.h"

namespace MonkSWF {
	
	class ShowFrameTag : public ITag {
	public:
		ShowFrameTag( TagHeader& h ) 
		: ITag( h )
		{}
		
		virtual ~ShowFrameTag()
        {}
		
        virtual bool read( Reader& reader, SWF&, MovieFrames& ) { return false; } //delete tag

		virtual void print() { MK_TRACE("SHOW_FRAME\n"); }

		static ITag* create( TagHeader& header ) {
			return (ITag*)(new ShowFrameTag( header ));
		}				
	};
}

#endif	// __ShowFrame_h__