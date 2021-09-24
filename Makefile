OPTS=-Wall
CFLAGS=-I/usr/include/cairo  ${OPTS}
LDLIBS=$(shell pkg-config x11 --libs) -lXfixes -lcairo -lXcomposite -lXext

all: cc

cc:
	g++ -ggdb -o draw-border ${CFLAGS} draw-border.c ${LDLIBS} && ./draw-border

vi:
	vim Makefile draw-border.c
