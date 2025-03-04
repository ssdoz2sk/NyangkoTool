#ifndef _SETTINGS__H_
#define _SETTINGS__H_

#include <Base.h>
#include <Uefi.h>
#include "ScreenManager.h"

#define DEFAULT_DELAY  100

typedef struct {
    SCREEN_MODE     DisplayMode;
} SETTING;

EFI_STATUS LoadSetting();
EFI_STATUS SaveAndFreeSetting();

extern SETTING *gSetting;

#endif