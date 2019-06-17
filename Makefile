CC = gcc -g
YFLAG = -d
FNAME = compiler_hw3
PARSER = myparser
OBJFILE = y.tab.o lex.yy.o parsingJVM.o
OBJECT = lex.yy.c y.tab.c y.tab.h ${FNAME}.j ${FNAME}.class

all: ${OBJFILE}
	@${CC} -o ${PARSER} $^

%.o: %.c
	@${CC} -c $<

lex.yy.c: ${FNAME}.l
	@lex $<

y.tab.c: ${FNAME}.y
	@yacc ${YFLAG} $<

test:
	rm -f *.j
	@./${PARSER} < ./example_input/basic_declaration.c
	@echo -e "\n\033[1;33mmain.class output\033[0m"
	@java -jar jasmin.jar ${FNAME}.j
	@java ${FNAME} 

clean:
	rm -f *.o ${PARSER} ${OBJECT}
	