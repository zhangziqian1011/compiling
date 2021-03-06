#TOOLS
CC = gcc
FLEX = flex
BISON = bison
CFLAGS = -std=c99

#FILES
CFILES = $(shell find ./ -maxdepth 1 -name "*.c")
CMMFILES = $(shell find ../testcase -name "*.cmm")
OUTFILES = $(CMMFILES:.cmm=.s)
OBJS = $(CFILES:.c=.o)
LFILE = $(shell find ./ -name "*.l")
YFILE = $(shell find ./ -name "*.y")
LFC = $(shell find ./ -name "*.l" | sed s/[^/]*\\.l/lex.yy.c/)
YFC = $(shell find ./ -name "*.y" | sed s/[^/]*\\.y/syntax.tab.c/)
LFO = $(LFC:.c=.o)
YFO = $(YFC:.c=.o)

#TARGETS
parser: syntax $(filter-out $(LFO),$(OBJS))
	$(CC) -o ../parser $(filter-out $(LFO),$(OBJS)) -lfl -ly
syntax: lexical syntax-c
	$(CC) -c $(YFC) -o $(YFO)
lexical: $(LFILE)
	$(FLEX) -o $(LFC) $(LFILE)
syntax-c: $(YFILES)
	$(BISON) -o $(YFC) -d -v $(YFILE)
-include $(patsubst %.o, %.d, $(OBJS))


.PHONY: clean test

clean:
	rm -f ../parser lex.yy.c syntax.tab.c syntax.tab.h syntax.output
	rm -f $(OBJS) $(OBJS:.o=.d)
	rm -f $(OUTFILES)
	rm -f $(LFC) $(YFC) $(YFC:.c=.h)
	rm -f *~

test: parser
	../parser ../testcase/test1.cmm ../testcase/test1.s
	../parser ../testcase/test2.cmm ../testcase/test2.s
	../parser ../testcase/test3.cmm ../testcase/test3.s
	../parser ../testcase/test4.cmm ../testcase/test4.s
	../parser ../testcase/test5.cmm ../testcase/test5.s
	../parser ../testcase/test6.cmm ../testcase/test6.s

submit: clean
	zip -r ../../151220166_lab4.zip ../
	





