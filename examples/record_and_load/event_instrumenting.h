#ifndef _EVENT_INSTRUMENTING_H_
#define _EVENT_INSTRUMENTING_H_

#ifdef INSTRUMENT_EVENTS

#include "unikorn.h"
#include <stdlib.h>

enum {
  // Make sure first ID starts with 1, and +1 for each adtional ID
  // Define folders first (if any)
  FOLDER1_ID = 1,
  FOLDER2_ID,
  // Events
  PRINT_START_ID,
  PRINT_END_ID,
  SQRT_START_ID,
  SQRT_END_ID
};

#ifdef DEFINE_FOLDERS_AND_EVENTS
static UkFolderInfo L_folders[] = {
  { "Folder 1", FOLDER1_ID},
  { "Folder 2", FOLDER2_ID}
};
static UkEventInfo L_events[] = {
  { "Print", PRINT_START_ID, PRINT_END_ID, UK_TEAL},
  { "Sqrt",  SQRT_START_ID,  SQRT_END_ID,  UK_BLACK}
};
#endif

// Overall
#define EVENTS_GLOBAL_INSTANCE extern void *G_instance     // Use this in every file that need to record events
#define EVENTS_INIT(filename, max_events, flush_when_full, is_threaded, record_instance, record_value, record_location) initEventIntrumenting(filename, max_events, flush_when_full, is_threaded, record_instance, record_value, record_location)
#define EVENTS_FLUSH() ukFlush(G_instance)
#define EVENTS_FINALIZE() finalizeEventIntrumenting()
// Folders
#define EVENTS_FOLDER1() ukRecordFolder(G_instance, FOLDER1_ID)
#define EVENTS_FOLDER2() ukRecordFolder(G_instance, FOLDER2_ID)
#define EVENTS_CLOSE_FOLDER() ukCloseFolder(G_instance)
// Events
#define EVENTS_START_PRINT() ukRecordEvent(G_instance, PRINT_START_ID, 0.0, __FILE__, __FUNCTION__, __LINE__)
#define EVENTS_END_PRINT()   ukRecordEvent(G_instance, PRINT_END_ID,   0.0, __FILE__, __FUNCTION__, __LINE__)
#define EVENTS_START_SQRT()  ukRecordEvent(G_instance, SQRT_START_ID,  0.0, __FILE__, __FUNCTION__, __LINE__)
#define EVENTS_END_SQRT()    ukRecordEvent(G_instance, SQRT_END_ID,    0.0, __FILE__, __FUNCTION__, __LINE__)

#ifdef __cplusplus
extern "C"
{
#endif
  extern void initEventIntrumenting(const char *filename, uint32_t max_events, bool flush_when_full, bool is_threaded, bool record_instance, bool record_value, bool record_location);
  extern void finalizeEventIntrumenting();
#ifdef __cplusplus
}
#endif

#else

// Overall
#define EVENTS_GLOBAL_INSTANCE
#define EVENTS_INIT(filename, max_events, flush_when_full, is_threaded, record_instance, record_value, record_location)
#define EVENTS_FLUSH()
#define EVENTS_FINALIZE()
// Folders
#define EVENTS_FOLDER1()
#define EVENTS_FOLDER2()
#define EVENTS_CLOSE_FOLDER()
// Events
#define EVENTS_START_PRINT()
#define EVENTS_END_PRINT()
#define EVENTS_START_SQRT()
#define EVENTS_END_SQRT()

#endif
#endif
