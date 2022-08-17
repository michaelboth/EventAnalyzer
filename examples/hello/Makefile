CFLAGS       := -std=gnu99 -Wall -Werror -Wextra -pthread -I. -I../../inc
CFLAGS       += -O2 -DUNIKORN_RELEASE_BUILD
#CFLAGS       += -g -O0
C_OBJS       := hello.o
HEADER_FILES := unikorn_instrumentation.h
LIBS         := -pthread
TARGET       := hello

ifeq ($(INSTRUMENT_APP),Yes)
    CFLAGS       += -DENABLE_UNIKORN_RECORDING
    CFLAGS       += -DENABLE_UNIKORN_ATOMIC_RECORDING  # Needed by unikorn.c if mutliple threads use a single unikorn session
    C_OBJS       += unikorn.o unikorn_file_flush.o
    HEADER_FILES += unikorn.h unikorn_clock.h unikorn_file_flush.h
    # Define a clock
    ifeq ($(CLOCK),gettime)
	C_OBJS += unikorn_clock_gettime.o
    else ifeq ($(CLOCK),gettimeofday)
	C_OBJS += unikorn_clock_gettimeofday.o
    else
        $(error 'ERROR: need to specify one of: CLOCK=gettime, CLOCK=gettimeofday')
    endif
endif

vpath %.c ../../src
vpath %.h ../../inc

all: $(TARGET)

clean:
	rm -f *.o
	rm -f *~
	rm -f *.events
	rm -f $(TARGET)

$(C_OBJS): %.o: %.c $(HEADER_FILES)
	gcc $(CFLAGS) -c $< -o $@

$(TARGET): $(C_OBJS)
	gcc $(C_OBJS) $(LIBS) -o $@
