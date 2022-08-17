This folder and makefiles are just a convenience for creating the Unikorn library
instead of directly compiling unikorn.c into your application.

Linux and Mac
  Build:
    > cd unikorn/lib
    > make RELEASE=Yes ATOMIC_RECORDING=Yes
  When linking the library into your app add the following to the link line:
    -L../../lib -lunikorn -pthread


Windows
  Assumtions:
    - Visual Studio 2019 or greater x64 console
    - pthreads4w is installed
      1. Get the source code from: https://sourceforge.net/projects/pthreads4w/
      2. Unzip, rename to 'pthreads4w' and put in the C:\ folder
      3. Start a Visual Studio x64 native shell
      > cd c:\pthreads4w
      > nmake VC VC-debug VC-static VC-static-debug install DESTROOT=.\install
  Build:
    > cd unikorn\lib
    > nmake -f windows.Makefile RELEASE=Yes ATOMIC_RECORDING=No
    > nmake -f windows.Makefile RELEASE=Yes ATOMIC_RECORDING=Yes
  When linking the library into your app add the following to the link line:
    Threaded:
      ../../lib/unikornMT.lib -nodefaultlib:MSVCRTD.LIB c:/pthreads4w/install/lib/libpthreadVC3.lib -nodefaultlib:LIBCMT.LIB
    Not Threaded:
      ../../lib/unikorn.lib -nodefaultlib:MSVCRTD.LIB
