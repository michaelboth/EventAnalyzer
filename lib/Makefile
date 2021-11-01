CFLAGS       := -std=gnu99 -Wall -Werror -I../inc
C_OBJS       := unikorn.o
HEADER_FILES := unikorn.h
LIBRARY      := libunikorn.a

# Check if threading is enabled
ifeq ($(THREADED),Yes)
    CFLAGS += -pthread -DALLOW_THREADING
endif

# Check if release distribution is enabled
ifeq ($(RELEASE),Yes)
    CFLAGS += -O2
else
    CFLAGS += -g -O0
endif

vpath %.c ../src
vpath %.h ../inc

all: $(LIBRARY)

clean:
	rm -f *.o
	rm -f *~
	rm -f $(LIBRARY)

$(C_OBJS): %.o: %.c $(HEADER_FILES)
	gcc $(CFLAGS) -c $< -o $@

$(LIBRARY): $(C_OBJS)
	ar rcs $@ $^
