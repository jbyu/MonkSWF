#ifndef __IMPORT_ASSETS2_H__
#define __IMPORT_ASSETS2_H__

#include "mkTag.h"

namespace MonkSWF
{
	class ImportAssets2Tag : public ITag
    {
        uint16_t count;

	public:
		ImportAssets2Tag( TagHeader& h ) 
		: ITag( h )
		{}
		
		virtual ~ImportAssets2Tag()
        {}

		virtual bool read( Reader& reader, SWF& swf, MovieFrames&) {
            const char *url = reader.getString();
			reader.get<uint8_t>();//reserved
			reader.get<uint8_t>();//reserved
            count = reader.get<uint16_t>();
            MK_TRACE("from %s\n", url);
            for (uint16_t i = 0; i < count; ++i)
            {
			    uint16_t tag = reader.get<uint16_t>();
                const char *name = reader.getString();
                swf.addAsset(tag, name, true );
                MK_TRACE("import[%d] %s\n", tag, name);
            }
			return false; // delete tag
		}

		virtual void print() {
    		_header.print();
			//MK_TRACE("EXPORT_ASSETS:%d\n", count);
		}

		static ITag* create( TagHeader& header ) {
			return (ITag*)(new ImportAssets2Tag( header ));
		}				
    };

}	
#endif//__IMPORT_ASSETS2_H__