#include "util.hpp"

int Wrap(int oldIndex, int change, int bound) {
    int newIndex = oldIndex + change;
    if (newIndex >= bound) {
        return newIndex % bound;
    }
    else if (newIndex < 0) {
        return bound + (newIndex % bound);
    }
    else {
        //A- This returns oldIndex + change, which is what newIndex already is.
        //A- Lmk if there's a reason why it shouldn't be return newIndex;
        return oldIndex + change;
    }
}

const char* MapEventIdToName(unsigned int id) {
    switch (id) {
        case (0x0000): return "SDL_FIRSTEVENT";
        case (0x0100): return "SDL_QUIT";
        case (0x0101): return "SDL_APP_TERMINATING";
        case (0x0102): return "SDL_APP_LOWMEMORY";
        case (0x0103): return "SDL_APP_WILLENTERBACKGROUND";
        case (0x0104): return "SDL_APP_DIDENTERBACKGROUND";
        case (0x0105): return "SDL_APP_WILLENTERFOREGROUND";
        case (0x0106): return "SDL_APP_DIDENTERFOREGROUND";
        case (0x0107): return "SDL_LOCALECHANGED";
        case (0x0150): return "SDL_DISPLAYEVENT";
        case (0x0200): return "SDL_WINDOWEVENT";
        case (0x0201): return "SDL_SYSWMEVENT";
        case (0x0300): return "SDL_KEYDOWN";
        case (0x0301): return "SDL_KEYUP";
        case (0x0302): return "SDL_TEXTEDITING";
        case (0x0303): return "SDL_TEXTINPUT";
        case (0x0304): return "SDL_KEYMAPCHANGED";
        case (0x0305): return "SDL_TEXTEDITING_EXT";
        case (0x0400): return "SDL_MOUSEMOTION";
        case (0x0401): return "SDL_MOUSEBUTTONDOWN";
        case (0x0402): return "SDL_MOUSEBUTTONUP";
        case (0x0403): return "SDL_MOUSEWHEEL";
        case (0x0600): return "SDL_JOYAXISMOTION";
        case (0x0601): return "SDL_JOYBALLMOTION";
        case (0x0602): return "SDL_JOYHATMOTION";
        case (0x0603): return "SDL_JOYBUTTONDOWN";
        case (0x0604): return "SDL_JOYBUTTONUP";
        case (0x0605): return "SDL_JOYDEVICEADDED";
        case (0x0606): return "SDL_JOYDEVICEREMOVED";
        case (0x0607): return "SDL_JOYBATTERYUPDATED";
        case (0x0650): return "SDL_CONTROLLERAXISMOTION";
        case (0x0651): return "SDL_CONTROLLERBUTTONDOWN";
        case (0x0652): return "SDL_CONTROLLERBUTTONUP";
        case (0x0653): return "SDL_CONTROLLERDEVICEADDED";
        case (0x0654): return "SDL_CONTROLLERDEVICEREMOVED";
        case (0x0655): return "SDL_CONTROLLERDEVICEREMAPPED";
        case (0x0656): return "SDL_CONTROLLERTOUCHPADDOWN";
        case (0x0657): return "SDL_CONTROLLERTOUCHPADMOTION";
        case (0x0658): return "SDL_CONTROLLERTOUCHPADUP";
        case (0x0659): return "SDL_CONTROLLERSENSORUPDATE";
        case (0x0700): return "SDL_FINGERDOWN";
        case (0x0701): return "SDL_FINGERUP";
        case (0x0702): return "SDL_FINGERMOTION";
        case (0x0800): return "SDL_DOLLARGESTURE";
        case (0x0801): return "SDL_DOLLARRECORD";
        case (0x0802): return "SDL_MULTIGESTURE";
        case (0x0900): return "SDL_CLIPBOARDUPDATE";
        case (0x1000): return "SDL_DROPFILE";
        case (0x1001): return "SDL_DROPTEXT";
        case (0x1002): return "SDL_DROPBEGIN";
        case (0x1003): return "SDL_DROPCOMPLETE";
        case (0x1100): return "SDL_AUDIODEVICEADDED";
        case (0x1101): return "SDL_AUDIODEVICEREMOVED";
        case (0x1200): return "SDL_SENSORUPDATE";
        case (0x2000): return "SDL_RENDER_TARGETS_RESET";
        case (0x2001): return "SDL_RENDER_DEVICE_RESET";
        case (0x0007): return "SDL_POLLSENTINEL";
        case (0x8000): return "SDL_USEREVENT";
        case (0xFFFF): return "SDL_LASTEVENT";
        default: return "Unknown Event";
    }
}
