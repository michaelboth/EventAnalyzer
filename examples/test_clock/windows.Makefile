CLOCK_CFLAGS =

# Define a clock
CLOCK_CFLAGS = unset
!IF "$(CLOCK)" == "QueryPerformanceCounter"
CLOCK_CFLAGS = -DUSE_QueryPerformanceCounter_CLOCK
!ENDIF
!IF "$(CLOCK)" == "ftime"
CLOCK_CFLAGS = -DUSE_ftime_CLOCK
!ENDIF
!IF "$(CLOCK_CFLAGS)" == "unset"
!  ERROR 'The variable CLOCK is not defined!'
!ENDIF

OPTIMIZATION_CFLAGS  = -O2 -MD  # Release: -MT means static linking, and -MD means dynamic linking.
#OPTIMIZATION_CFLAGS  = -Zi -MDd # Debug: -MTd or -MDd

CFLAGS  = $(OPTIMIZATION_CFLAGS) -nologo -WX -W3 -I. -I../../inc -I../../ref $(CLOCK_CFLAGS)
C_OBJS  = test_clock.obj event_clocks.obj
LDFLAGS = -nologo -incremental:no -manifest:embed -subsystem:console
LIBS    = 
TARGET  = test_clock.exe

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
	-del *~
