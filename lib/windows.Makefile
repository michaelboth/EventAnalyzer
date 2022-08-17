# Check if threading is enabled
!IF "$(ATOMIC_RECORDING)" == "Yes"
THREAD_CFLAGS = -DENABLE_UNIKORN_ATOMIC_RECORDING -Ic:/pthreads4w/install/include
LIBRARY = unikornMT.lib
!ELSE
THREAD_CFLAGS =
LIBRARY = unikorn.lib
!ENDIF

# Check if release distribution is enabled
!IF "$(RELEASE)" == "Yes"
OPTIMIZATION_CFLAGS  = -O2 -MD -DUNIKORN_RELEASE_BUILD # Release: -MT means static linking, and -MD means dynamic linking.
!ELSE
OPTIMIZATION_CFLAGS  = -Zi -MDd # Debug: -MTd or -MDd
!ENDIF

CFLAGS  = $(OPTIMIZATION_CFLAGS) -nologo -WX -W3 -I../inc $(THREAD_CFLAGS)

.SUFFIXES: .c

all: $(LIBRARY)

clean:
	-del *.lib
	-del *.obj
	-del *.pdb
	-del *~

unikorn.lib:
	cl -c $(CFLAGS) -Fo.\unikorn.obj ..\src\unikorn.c
	lib -nologo unikorn.obj -out:unikorn.lib

unikornMT.lib:
	cl -c $(CFLAGS) -Fo.\unikornMT.obj ..\src\unikorn.c
	lib -nologo unikornMT.obj -out:unikornMT.lib
