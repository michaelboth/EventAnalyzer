CFLAGS       := -std=gnu99 -Wall -Werror -pthread -I../../inc
CFLAGS       += -O2
#CFLAGS       += -g -O0
C_OBJS       := test_clock.o
HEADER_FILES := event_recorder_clock.h
LIBS         := 
TARGET       := test_clock

ifneq ($(MAKECMDGOALS),clean)
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
	rm -f $(TARGET)

$(C_OBJS): %.o: %.c $(HEADER_FILES)
	gcc $(CFLAGS) -c $< -o $@

$(TARGET): $(C_OBJS)
	gcc $(C_OBJS) $(LIBS) -o $@
