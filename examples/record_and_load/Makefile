CFLAGS       := -std=gnu99 -Wall -Werror -pthread -I. -I../../inc -I../../ref
C_OBJS       := record_and_load.o
HEADER_FILES := event_instrumenting.h
PRJ_LIBS     := -pthread
TARGET       := record_and_load

ifeq ($(INSTRUMENT_APP),Yes)
    CFLAGS       += -DINSTRUMENT_APP -DPRINT_LOAD_INFO
    C_OBJS       += event_clocks.o event_file_flush.o events_loader.o event_instrumenting.o
    HEADER_FILES := unikorn.h event_clocks.h event_file_flush.h events_loader.h
    PRJ_LIBS     += -L ../../lib -lunikorn
    # Define the clock
    ifeq ($(CLOCK),clock_gettime)
        CFLAGS += -DUSE_clock_gettime_MONOTONIC_CLOCK
    else ifeq ($(CLOCK),gettimeofday)
        CFLAGS += -DUSE_gettimeofday_CLOCK
    else
        $(error 'ERROR: need to specify one of: CLOCK=clock_gettime, CLOCK=gettimeofday')
    endif
endif

# Check for OS
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    PRJ_LIBS += -lm
endif
ifeq ($(UNAME_S),Darwin)
    PRJ_LIBS +=
endif

# Check if release distribution is enabled
ifeq ($(RELEASE),Yes)
    CFLAGS += -O2
else
    CFLAGS += -g -O0
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
	gcc $(C_OBJS) $(PRJ_LIBS) -o $@
