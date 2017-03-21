includedir = @includedir@/ami
libami_la_CFLAGS = -I@srcdir@/include
libami_la_LDFLAGS =  -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
lib_LTLIBRARIES = libami.la 
include_HEADERS = include/ami/admin.h  include/ami/ami.h  include/ami/channel.h  \
                  include/ami/core.h  include/ami/info.h include/ami/manager.h
libami_la_SOURCES = admin.c  channel.c  core.c  info.c  manager.c
EXTRA_DIST = Library.txt

