CFLAGS       := -std=gnu99 -Wall -Werror -Wextra -pthread -I. -I../../inc
CFLAGS       += -O2 -DUNIKORN_RELEASE_BUILD
#CFLAGS       += -g -O0
C_OBJS       := test_record_and_load.o
HEADER_FILES := unikorn_instrumentation.h
LIBS         := -pthread -lm
TARGET       := test_record_and_load

ifeq ($(INSTRUMENT_APP),Yes)
    CFLAGS       += -DENABLE_UNIKORN_RECORDING
    CFLAGS       += -DENABLE_UNIKORN_ATOMIC_RECORDING  # Needed by unikorn.c if mutliple threads use a single unikorn session
    CFLAGS       += -DPRINT_UNIKORN_LOAD_INFO # Needed by unikorn_file_loader.c to print all the loaded event information
    C_OBJS       += unikorn.o unikorn_file_flush.o unikorn_file_loader.o
    HEADER_FILES += unikorn.h unikorn_clock.h unikorn_file_flush.h unikorn_file_loader.h
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
