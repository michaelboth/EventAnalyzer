#ifndef _EVENT_INSTRUMENTING_H_
#define _EVENT_INSTRUMENTING_H_

#include "unikorn.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern void *G_instance;

extern void initEventIntrumenting(const char *filename, uint32_t max_events, bool flush_when_full, bool is_threaded, bool record_instance, bool record_value, bool record_location,
				  uint16_t num_folders, UkFolderInfo *folder_info_list, uint16_t num_event_types, UkEventInfo *event_info_list);
extern void finalizeEventIntrumenting();

#ifdef __cplusplus
}
#endif

#endif
