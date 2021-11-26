INSTRUMENT_CFLAGS =
INSTRUMENT_C_OBJS =
CLOCK_C_OBJ       =

!IF "$(INSTRUMENT_APP)" == "Yes"
INSTRUMENT_CFLAGS       = -DINSTRUMENT_APP -DPRINT_LOAD_INFO
INSTRUMENT_C_OBJS       = unikorn.obj event_recorder_file_flush.obj event_file_loader.obj
# Define a clock
CLOCK_C_OBJ = unset
!  IF "$(CLOCK)" == "queryperformancecounter"
CLOCK_C_OBJ = event_recorder_clock_queryperformancecounter.obj
!  ENDIF
!  IF "$(CLOCK)" == "ftime"
CLOCK_C_OBJ = event_recorder_clock_ftime.obj
!  ENDIF
!  IF "$(CLOCK_C_OBJ)" == "unset"
!  ERROR 'ERROR: need to specify one of: CLOCK=queryperformancecounter, CLOCK=ftime'
!  ENDIF
!ENDIF

# Check if threading is enabled
THREAD_SAFE = Yes
!IF "$(THREAD_SAFE)" == "Yes"
THREAD_CFLAGS = -DALLOW_THREADS -Ic:/pthreads4w/install/include
THREAD_LIBS   = c:/pthreads4w/install/lib/libpthreadVC3.lib -nodefaultlib:LIBCMT.LIB
#THREAD_LIBS   = c:/pthreads4w/install/lib/libpthreadVC3d.lib -nodefaultlib:LIBCMT.LIB
!ELSE
THREAD_CFLAGS =
THREAD_LIBS   =
!ENDIF

OPTIMIZATION_CFLAGS  = -O2 -MD -DRELEASE_BUILD  # Release: -MT means static linking, and -MD means dynamic linking.
#OPTIMIZATION_CFLAGS  = -Zi -MDd                # Debug: -MTd or -MDd

CFLAGS  = $(OPTIMIZATION_CFLAGS) -nologo -WX -W3 -I. -I../../inc $(INSTRUMENT_CFLAGS) $(THREAD_CFLAGS)
LDFLAGS = -nologo -incremental:no -manifest:embed -subsystem:console
LIBS    = $(THREAD_LIBS)
C_OBJS  = test_record_and_load.obj $(INSTRUMENT_C_OBJS) $(CLOCK_C_OBJ)
TARGET  = test_record_and_load.exe

.SUFFIXES: .c

all: $(TARGET)

{.\}.c{}.obj::
	cl -c $(CFLAGS) -Fo $<

{..\..\src}.c{}.obj::
	cl -c $(CFLAGS) -Fo $<

$(TARGET): $(C_OBJS)
	link $(LDFLAGS) $(C_OBJS) $(LIBS) -out:$(TARGET)

clean:
	-del $(TARGET)
	-del *.obj
	-del *.pdb
	-del *.events
	-del *~
