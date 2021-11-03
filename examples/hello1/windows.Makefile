# Check if threading is enabled
!IF "$(THREAD_SAFE)" == "Yes"
THREAD_CFLAGS = -DALLOW_THREADING -Ic:/pthreads4w/install/include
THREAD_LIBS   = c:/pthreads4w/install/lib/libpthreadVC3.lib -nodefaultlib:LIBCMT.LIB
#THREAD_LIBS   = c:/pthreads4w/install/lib/libpthreadVC3d.lib -nodefaultlib:LIBCMT.LIB
!ELSE
THREAD_CFLAGS =
THREAD_LIBS   =
!ENDIF

OPTIMIZATION_CFLAGS  = -O2 -MD  # Release: -MT means static linking, and -MD means dynamic linking.
#OPTIMIZATION_CFLAGS  = -Zi -MDd # Debug: -MTd or -MDd

CFLAGS  = $(OPTIMIZATION_CFLAGS) -nologo -WX -W3 -I. -I../../inc $(THREAD_CFLAGS)
C_OBJS  = hello.obj
LDFLAGS = -nologo -incremental:no -manifest:embed -subsystem:console
LIBS    = ../../lib/unikorn.lib -nodefaultlib:MSVCRTD.LIB $(THREAD_LIBS)
TARGET  = hello.exe

.SUFFIXES: .c

all: $(TARGET)

{.\}.c{}.obj::
	cl -c $(CFLAGS) -Fo $<

$(TARGET): $(C_OBJS)
	link $(LDFLAGS) $(C_OBJS) $(LIBS) -out:$(TARGET)

clean:
	-del $(TARGET)
	-del *.obj
	-del *.pdb
	-del *.events
	-del *~
