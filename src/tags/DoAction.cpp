#include "DoAction.h"
#include "PlaceObject.h"

namespace MonkSWF
{
    enum ACTION_CODE
    {
        //SWF 3 action model
        ACTION_NEXT_FRAME       = 0x04,
        ACTION_PREV_FRAME       = 0x05,
        ACTION_PLAY             = 0x06,
        ACTION_STOP             = 0x07,
        ACTION_TOGGLE_QUALITY   = 0x08,
        ACTION_STOP_SOUNDS      = 0x09,
        ACTION_GOTO_FRAME       = 0x81,
        ACTION_GET_URL          = 0x83,
        ACTION_WAIT_FOR_FRAME   = 0x8A,
        ACTION_SET_TARGET       = 0x8B,
        ACTION_GOTO_LABEL       = 0x8C,
    };

    bool DoActionTag::read( Reader* reader, SWF* swf) {
        uint8_t code;
        do {
			code = reader->get<uint8_t>();
    		if (code & 0x80)
			{
				// Action contains extra data.
				uint16_t length = reader->get<uint16_t>();
                reader->skip(length);
			}
            moActions.push_back(code);
        } while(0 != code);
		return true;
	}

    void DoActionTag::setup(MovieClip& movie)
    {
        std::vector<uint8_t>::iterator it = moActions.begin();
        while(moActions.end()!=it)
        {
            uint8_t code = *it;
            switch(code)
            {
            case ACTION_PLAY:
                movie.play(true);
                break;
            case ACTION_STOP:
                movie.play(false);
                break;
            default:
                break;
            }
            ++it;
        }
    }

}//namespace
