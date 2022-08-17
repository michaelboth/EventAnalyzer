CFLAGS       := -std=gnu99 -Wall -Werror -Wextra -fPIC -I../inc
C_OBJS       := unikorn.o
HEADER_FILES := unikorn.h
LIBRARY      := libunikorn.a

# Check if threading is enabled
ifeq ($(ATOMIC_RECORDING),Yes)
    CFLAGS += -pthread -DENABLE_UNIKORN_ATOMIC_RECORDING
endif

# Check if release distribution is enabled
ifeq ($(RELEASE),Yes)
    CFLAGS += -O2 -DUNIKORN_RELEASE_BUILD
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
