SOURCES = sim_c.parser.c sim_c.lexer.c sim_c.parser.h utils.c optim.c \
	declare.c statement.c operator.c
EXEC_NAME = comp

OBJECTS = $(filter-out %.h, $(SOURCES))

all : $(EXEC_NAME)

$(EXEC_NAME) : $(OBJECTS)
	gcc -o $@ $^ -lm

%.parser.h : %.parser.c

%.parser.c : %.y
	bison -d -o $@ $<

%.lexer.c : %.l
	flex -o $@ $<

debug: $(OBJECTS)
	gcc -g -o comp_debug $^ -lm

clean:
	rm -rf sim_c.*.c *.o sim_c.*.h $(EXEC_NAME) comp_debug
