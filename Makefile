PLUGINNAME = alias-base
LIBNAME = lib$(PLUGINNAME)-plugin.so
#
CC = gcc
LD = gcc
CFLAGS = -fPIC -I /usr/include/nspr4 -Wall
LDFLAGS = -shared -z defs -L/usr/lib64/dirsrv -lslapd
OBJS = $(PLUGINNAME).o
all: $(LIBNAME)
$(LIBNAME): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
.c.o:
	$(CC) $(CFLAGS) -c $<
clean:
	-rm -f $(OBJS) $(LIBNAME)
