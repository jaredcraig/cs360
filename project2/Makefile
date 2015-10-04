# Makefile for echo client and server

CXX=			g++ $(CCFLAGS)

ECHO-SERVER=		buffer.o echo-server.o server.o
OBJS =			$(ECHO-SERVER)

LIBS= -lpthread

CCFLAGS= -g

all:	echo-server

echo-server:$(ECHO-SERVER)
	$(CXX) -o server $(ECHO-SERVER) $(LIBS)

clean:
	rm -f $(OBJS) $(OBJS:.o=.d)

realclean:
	rm -f $(OBJS) $(OBJS:.o=.d) server


# These lines ensure that dependencies are handled automatically.
%.d:	%.cc
	$(SHELL) -ec '$(CC) -M $(CPPFLAGS) $< \
		| sed '\''s/\($*\)\.o[ :]*/\1.o $@ : /g'\'' > $@; \
		[ -s $@ ] || rm -f $@'

include	$(OBJS:.o=.d)
