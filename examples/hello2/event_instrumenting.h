#ifndef _EVENT_INSTRUMENTING_H_
#define _EVENT_INSTRUMENTING_H_

#ifdef INSTRUMENT_APP

#include "unikorn.h"
#include <stdlib.h>

enum {
  // Folders
  // Events
  QUICK_SORT_START_ID=1,
  QUICK_SORT_END_ID,
  BUBBLE_SORT_START_ID,
  BUBBLE_SORT_END_ID
};

// IMPORTANT: Call #define DEFINE_FOLDERS_AND_EVENTS, just before #include "event_instrumenting.h", in the file that calls EVENTS_INIT()
#ifdef DEFINE_FOLDERS_AND_EVENTS
//static UkFolderInfo L_folders[] = {
//  { "Folder 1", FOLDER1_ID},
//  { "Folder 2", FOLDER2_ID}
//};
static UkEventInfo L_events[] = {
  { "Quick Sort",  QUICK_SORT_START_ID,  QUICK_SORT_END_ID,  UK_BLUE},
  { "Bubble Sort", BUBBLE_SORT_START_ID, BUBBLE_SORT_END_ID, UK_RED}
};
#endif

// Overall
/*+ move into C++ wrapper */
#define EVENTS_GLOBAL_INSTANCE extern void *G_instance     // Use this in every file that need to record events
#ifdef DEFINE_FOLDERS_AND_EVENTS
  #define EVENTS_INIT(filename, max_events, flush_when_full, is_threaded, record_instance, record_value, record_location) initEventIntrumenting(filename, max_events, flush_when_full, is_threaded, record_instance, record_value, record_location, 0, NULL, sizeof(L_events) / sizeof(UkEventInfo), L_events)
#endif
#define EVENTS_FLUSH() ukFlush(G_instance)
#define EVENTS_FINALIZE() finalizeEventIntrumenting()
// Folders
//#define EVENTS_FOLDER1() ukRecordFolder(G_instance, FOLDER1_ID)
//#define EVENTS_FOLDER2() ukRecordFolder(G_instance, FOLDER2_ID)
//#define EVENTS_CLOSE_FOLDER() ukCloseFolder(G_instance)
// Events
#define EVENTS_START_QUICK_SORT()  ukRecordEvent(G_instance, QUICK_SORT_START_ID,  0.0, __FILE__, __FUNCTION__, __LINE__)
#define EVENTS_END_QUICK_SORT()    ukRecordEvent(G_instance, QUICK_SORT_END_ID,    0.0, __FILE__, __FUNCTION__, __LINE__)
#define EVENTS_START_BUBBLE_SORT() ukRecordEvent(G_instance, BUBBLE_SORT_START_ID, 0.0, __FILE__, __FUNCTION__, __LINE__)
#define EVENTS_END_BUBBLE_SORT()   ukRecordEvent(G_instance, BUBBLE_SORT_END_ID,   0.0, __FILE__, __FUNCTION__, __LINE__)

#ifdef __cplusplus
extern "C"
{
#endif
  extern void initEventIntrumenting(const char *filename, uint32_t max_events, bool flush_when_full, bool is_threaded, bool record_instance, bool record_value, bool record_location,
                                    uint16_t num_folders, UkFolderInfo *folder_info_list, uint16_t num_event_types, UkEventInfo *event_info_list);

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
//#define EVENTS_FOLDER1()
//#define EVENTS_FOLDER2()
//#define EVENTS_CLOSE_FOLDER()
// Events
#define EVENTS_START_QUICK_SORT()
#define EVENTS_END_QUICK_SORT()
#define EVENTS_START_BUBBLE_SORT()
#define EVENTS_END_BUBBLE_SORT()

#endif
#endif
