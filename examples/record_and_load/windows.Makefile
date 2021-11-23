INSTRUMENT_CFLAGS       =
INSTRUMENT_C_OBJS       =
INSTRUMENT_PRJ_LIBS     =
CLOCK_CFLAGS            =

!IF "$(INSTRUMENT_APP)" == "Yes"
INSTRUMENT_CFLAGS       = -DINSTRUMENT_APP -DPRINT_LOAD_INFO
INSTRUMENT_C_OBJS       = event_clocks.obj event_file_flush.obj event_instrumenting.obj events_loader.obj
INSTRUMENT_PRJ_LIBS     = ../../lib/unikorn.lib -nodefaultlib:MSVCRTD.LIB
# Define a clock
CLOCK_CFLAGS = unset
!  IF "$(CLOCK)" == "QueryPerformanceCounter"
CLOCK_CFLAGS = -DUSE_QueryPerformanceCounter_CLOCK
!  ENDIF
!  IF "$(CLOCK)" == "ftime"
CLOCK_CFLAGS = -DUSE_ftime_CLOCK
!  ENDIF
!  IF "$(CLOCK_CFLAGS)" == "unset"
!  ERROR 'The variable CLOCK is not defined!'
!  ENDIF
!ENDIF

# Check if threading is enabled
THREAD_SAFE = Yes
!IF "$(THREAD_SAFE)" == "Yes"
THREAD_CFLAGS = -Ic:/pthreads4w/install/include
THREAD_LIBS   = c:/pthreads4w/install/lib/libpthreadVC3.lib -nodefaultlib:LIBCMT.LIB
#THREAD_LIBS   = c:/pthreads4w/install/lib/libpthreadVC3d.lib -nodefaultlib:LIBCMT.LIB
INSTRUMENT_PRJ_LIBS = ../../lib/unikornMT.lib -nodefaultlib:MSVCRTD.LIB
!ELSE
THREAD_CFLAGS =
THREAD_LIBS   =
!ENDIF

OPTIMIZATION_CFLAGS  = -O2 -MD  # Release: -MT means static linking, and -MD means dynamic linking.
#OPTIMIZATION_CFLAGS  = -Zi -MDd # Debug: -MTd or -MDd

CFLAGS  = $(OPTIMIZATION_CFLAGS) -nologo -WX -W3 -I. -I../../inc -I../../ref $(INSTRUMENT_CFLAGS) $(CLOCK_CFLAGS) $(THREAD_CFLAGS)
C_OBJS  = record_and_load.obj $(INSTRUMENT_C_OBJS)
LDFLAGS = -nologo -incremental:no -manifest:embed -subsystem:console
LIBS    = $(INSTRUMENT_PRJ_LIBS) $(THREAD_LIBS)
TARGET  = record_and_load.exe

.SUFFIXES: .c

all: $(TARGET)

{.\}.c{}.obj::
	cl -c $(CFLAGS) -Fo $<

{..\..\ref}.c{}.obj::
	cl -c $(CFLAGS) -Fo $<

$(TARGET): $(C_OBJS)
	link $(LDFLAGS) $(C_OBJS) $(LIBS) -out:$(TARGET)

clean:
	-del $(TARGET)
	-del *.obj
	-del *.pdb
	-del *.events
	-del *~
