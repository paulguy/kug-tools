COMMONOBJS		= kugfile.o kugerr.o
LSKUGOBJS		= lskug.o
EXKUGOBJS		= exkug.o
PKKUGOBJS		= pkkug.o
GEOKUGOBJS		= geokug.o
LSKUG			= lskug
EXKUG			= exkug
PKKUG			= pkkug
GEOKUG			= geokug

CFLAGS		= -pedantic -Wall -Wextra -std=gnu99 -ggdb
#CFLAGS		= -pedantic -Wall -Wextra -std=gnu99 
LDFLAGS		= 

all:			$(LSKUG) $(EXKUG) $(PKKUG) $(GEOKUG)

$(LSKUG):		$(COMMONOBJS) $(LSKUGOBJS)
	$(CC) $(LDFLAGS) -o $(LSKUG) $(COMMONOBJS) $(LSKUGOBJS)

$(EXKUG):		$(COMMONOBJS) $(EXKUGOBJS)
	$(CC) $(LDFLAGS) -o $(EXKUG) $(COMMONOBJS) $(EXKUGOBJS)

$(PKKUG):		$(COMMONOBJS) $(PKKUGOBJS)
	$(CC) $(LDFLAGS) -o $(PKKUG) $(COMMONOBJS) $(PKKUGOBJS)

$(GEOKUG):		$(COMMONOBJS) $(GEOKUGOBJS)
	$(CC) $(LDFLAGS) -o $(GEOKUG) $(COMMONOBJS) $(GEOKUGOBJS)

clean:
	rm -f  $(COMMONOBJS) $(LSKUGOBJS) $(EXKUGOBJS) $(PKKUGOBJS) $(GEOKUGOBJS) $(LSKUG) $(EXKUG) $(PKKUG) $(GEOKUG)

.PHONY: clean
