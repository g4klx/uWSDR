all:		SDRHelp

OBJS :=	SDRHelpApp.o

SDRHelp:	$(OBJS)
		$(CC) $(LDFLAGS) -o SDRHelp $(OBJS) $(LIBS)

SDRHelpApp.o:	SDRHelpApp.cpp SDRHelpApp.h SDRHelp.xpm
		$(CC) $(CFLAGS) -c SDRHelpApp.cpp

install:	SDRHelp
		install -g bin -o root -m 0775 SDRHelp $(BINDIR)

clean:
		rm -f core SDRHelp *.o *~ *.bak
