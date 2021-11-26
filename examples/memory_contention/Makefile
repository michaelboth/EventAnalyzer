CFLAGS       := -std=gnu99 -Wall -Werror -pthread -I. -I../../inc
CFLAGS       += -O2 -DRELEASE_BUILD
#CFLAGS       += -g -O0
C_OBJS       := memory_contention.o
HEADER_FILES := custom_folders_and_events.h
LIBS         := -pthread -lm
TARGET       := memory_contention

ifeq ($(INSTRUMENT_APP),Yes)
    CFLAGS       += -DINSTRUMENT_APP # Needed to enable event recording
    CFLAGS       += -DALLOW_THREADS  # Needed by unikorn.c if threading needs to be enabled
    C_OBJS       += unikorn.o event_recorder_file_flush.o
    HEADER_FILES += unikorn.h event_recorder_clock.h event_recorder_file_flush.h
    # Define a clock
    ifeq ($(CLOCK),gettime)
	C_OBJS += event_recorder_clock_gettime.o
    else ifeq ($(CLOCK),gettimeofday)
	C_OBJS += event_recorder_clock_gettimeofday.o
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
