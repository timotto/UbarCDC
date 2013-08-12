#ifndef CDC_H
#define CDC_H

#include "settings.h"

typedef struct _s_cdc_status {
  bool selected;
  bool playing;
  int currentDisc;
  int currentTrack;
  int trackPosition;
} t_cdc_status;
  
void cdc_setup();
void cdc_loop();
void cdc_selected(bool selected);
void cdc_onSelectCd(int cd);
void cdc_onPlay();
void cdc_onPause();
void cdc_onNext();
void cdc_onPrev();
void cdc_onFwd(bool startStop);
void cdc_onRwd(bool startStop);

extern t_cdc_status cdc_status;

#endif // CDC_H

