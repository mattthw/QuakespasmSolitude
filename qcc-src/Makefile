
CFLAGS = -g -Wall -Werror -Wstrict-prototypes -Wunused

EXES = qcc

all: $(EXES)

install:
	make app
	cp $(EXES) /LocalApps

app:
	make "CFLAGS = -O3"

debug:
	make "CFLAGS = -g -Wall"

profile:
	make "CFLAGS = -pg -Wall"

clean:
	rm -f *.o $(EXES)

.c.o: ; gcc -c $(CFLAGS) -o $@ $*.c

QCCFILES = qcc.o pr_lex.o pr_comp.o cmdlib.o
qcc : $(QCCFILES)
	gcc $(CFLAGS) -o qcc $(QCCFILES)