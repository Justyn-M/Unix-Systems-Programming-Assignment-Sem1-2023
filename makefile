CC = gcc
CFLAGS = -Wall
OBJS = calculator.o calc_funcs.o
EXEC = calculator

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJS)

calculator.o: calculator.c calc_funcs.h
	$(CC) $(CFLAGS) -c calculator.c

calc_funcs.o: calc_funcs.c macros.h
	$(CC) $(CFLAGS) -c calc_funcs.c

clean:
	rm -f $(EXEC) $(OBJS)