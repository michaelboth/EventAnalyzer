CFLAGS       := -std=gnu99 -Wall -Werror -pthread -I. -I../../inc -I../../ref
CFLAGS       += -O2
#CFLAGS       += -g -O0
C_OBJS       := hello.o
HEADER_FILES := event_instrumenting.h
LIBS         := -pthread
TARGET       := hello

ifeq ($(INSTRUMENT_APP),Yes)
    CFLAGS       += -DINSTRUMENT_APP
    C_OBJS       += event_clocks.o event_file_flush.o event_instrumenting.o
    HEADER_FILES := unikorn.h event_clocks.h event_file_flush.h
    LIBS         += -L ../../lib -lunikorn
    # Define a clock
    ifeq ($(CLOCK),clock_gettime)
        CFLAGS += -DUSE_clock_gettime_MONOTONIC_CLOCK
    else ifeq ($(CLOCK),gettimeofday)
        CFLAGS += -DUSE_gettimeofday_CLOCK
    else
        $(error 'ERROR: need to specify one of: CLOCK=clock_gettime, CLOCK=gettimeofday')
    endif
endif

vpath %.c ../../ref
vpath %.h ../../ref
vpath %.h ../../inc

all: $(TARGET)

clean:
	rm -f *.o
	rm -f *~
	rm -f $(TARGET)
	rm -f $(TARGET).events

$(C_OBJS): %.o: %.c $(HEADER_FILES)
	gcc $(CFLAGS) -c $< -o $@

$(TARGET): $(C_OBJS)
	gcc $(C_OBJS) $(LIBS) -o $@
