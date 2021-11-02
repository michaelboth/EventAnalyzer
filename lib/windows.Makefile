# Check if threading is enabled
!IF "$(THREAD_SAFE)" == "Yes"
THREAD_CFLAGS = -DALLOW_THREADING -Ic:\pthreads4w\install\include
!ELSE
THREAD_CFLAGS =
!ENDIF

# Check if release distribution is enabled
!IF "$(RELEASE)" == "Yes"
OPTIMIZATION_CFLAGS  = -O2 -MD  # Release: -MT means static linking, and -MD means dynamic linking.
!ELSE
OPTIMIZATION_CFLAGS  = -Zi -MDd # Debug: -MTd or -MDd
!ENDIF

CFLAGS           = $(OPTIMIZATION_CFLAGS) -nologo -Zm200 -D_CRT_SECURE_NO_WARNINGS=1 -WX -W3 -Zc:wchar_t- -w34189 -GR -EHsc -I..\inc $(THREAD_CFLAGS)
C_OBJS           = unikorn.obj
LIBRARY          = unikorn.lib

.SUFFIXES: .c

all: $(LIBRARY)

clean:
	del $(LIBRARY)
	del *.obj
	del *~

$(LIBRARY): $(C_OBJS)
	lib /NOLOGO $(C_OBJS) /OUT:$(LIBRARY)

{..\src\}.c.obj::
	cl -c $(CFLAGS) -Fo.\ $<
