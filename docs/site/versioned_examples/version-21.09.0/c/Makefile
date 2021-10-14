PROGRAM=asapo-consume

LDFLAGS = "-Wl,-rpath,/opt/asapo/lib"
CFLAGS += `PKG_CONFIG_PATH=/opt/asapo/lib/pkgconfig pkg-config --cflags libasapo-consumer`
LIBS = `PKG_CONFIG_PATH=/opt/asapo/lib/pkgconfig pkg-config --libs libasapo-consumer`

# for default installation
#LDFLAGS =
#CFLAGS += `pkg-config --cflags libasapo-consumer`
#LIBS = `pkg-config --libs libasapo-consumer`

RM=rm -f

SRCS=consume.c
OBJS=$(subst .c,.o,$(SRCS))

all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $<

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) $(PROGRAM)
