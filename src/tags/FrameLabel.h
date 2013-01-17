#ifndef __FRAME_LABEL_H__
#define __FRAME_LABEL_H__

#include "mkTag.h"

namespace MonkSWF
{
	class FrameLabelTag : public ITag {
        std::string _label;

	public:
		FrameLabelTag( TagHeader& h ) 
		: ITag( h )
		{}
		
		virtual ~FrameLabelTag()
        {}
		
		virtual bool read( Reader& reader, SWF&, MovieFrames& data )
        {
            const char* label = reader.getString();
            //reader.skip( length() - _label.size() );
            _label.assign( label );
            data._labels[label] = data._frames.size();
			return false;//delete tag
		}
		
		virtual void print() {
    		_header.print();
            MK_TRACE("label:%s\n", _label.c_str());
		}
		
		static ITag* create( TagHeader& header ) {
			return (ITag*)(new FrameLabelTag( header ));
		}				
	};
}	
#endif	//__FRAME_LABEL_H__