# Define a clock
CLOCK_C_OBJ =
!IF "$(CLOCK)" == "queryperformancecounter"
CLOCK_C_OBJ = unikorn_clock_queryperformancecounter.obj
!ENDIF
!IF "$(CLOCK)" == "ftime"
CLOCK_C_OBJ = unikorn_clock_ftime.obj
!ENDIF

OPTIMIZATION_CFLAGS  = -O2 -MD  # Release: -MT means static linking, and -MD means dynamic linking.
#OPTIMIZATION_CFLAGS  = -Zi -MDd # Debug: -MTd or -MDd

CFLAGS  = $(OPTIMIZATION_CFLAGS) -nologo -WX -W3 -I. -I../../inc
C_OBJS  = test_clock.obj $(CLOCK_C_OBJ)
LDFLAGS = -nologo -incremental:no -manifest:embed -subsystem:console
LIBS    = 
TARGET  = test_clock.exe

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
	-del *~
