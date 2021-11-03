CFLAGS       := -std=gnu99 -Wall -Werror -pthread -I. -I../../inc
CFLAGS       += -O2
#CFLAGS       += -g -O0
C_OBJS       := hello.o
HEADER_FILES :=
LIBS         := -pthread -L ../../lib -lunikorn
TARGET       := hello

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
