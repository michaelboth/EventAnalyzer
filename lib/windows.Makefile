# Check if threading is enabled
!IF "$(THREAD_SAFE)" == "Yes"
THREAD_CFLAGS = -DALLOW_THREADING -Ic:/pthreads4w/install/include
!ELSE
THREAD_CFLAGS =
!ENDIF

# Check if release distribution is enabled
!IF "$(RELEASE)" == "Yes"
OPTIMIZATION_CFLAGS  = -O2 -MD -DRELEASE_BUILD # Release: -MT means static linking, and -MD means dynamic linking.
!ELSE
OPTIMIZATION_CFLAGS  = -Zi -MDd # Debug: -MTd or -MDd
!ENDIF

CFLAGS  = $(OPTIMIZATION_CFLAGS) -nologo -WX -W3 -I../inc $(THREAD_CFLAGS)
C_OBJS  = unikorn.obj
LIBRARY = unikorn.lib

.SUFFIXES: .c

all: $(LIBRARY)

clean:
	-del $(LIBRARY)
	-del *.obj
	-del *.pdb
	-del *~

$(LIBRARY): $(C_OBJS)
	lib -nologo $(C_OBJS) -out:$(LIBRARY)

{..\src\}.c.obj::
	cl -c $(CFLAGS) -Fo.\ $<
