# Sample Makefile
# CC - compiler
# OBJ - compiled source files that should be linked
# COPT - compiler flags
# BIN - binary
CC=gcc
OBJ=helper1.o
COPT=-Wall -Wpedantic -g
BIN_PHASE1=phase1
BIN_PHASE2=dns_svr

# Running "make" with no argument will make the first target in the file
all: $(BIN_PHASE1) $(BIN_PHASE2)

# Rules of the form
#     target_to_be_made : dependencies_to_be_up-to-date_first
#     <tab>commands_to_make_target
# (Note that spaces will not work.)

$(BIN_PHASE2): main.c $(OBJ)
	$(CC) -o $(BIN_PHASE2) main.c $(OBJ) $(COPT)

$(BIN_PHASE1): phase1.c $(OBJ)
	$(CC) -o $(BIN_PHASE1) phase1.c $(OBJ) $(COPT)

# Wildcard rule to make any  .o  file,
# given a .c and .h file with the same leading filename component
%.o: %.c %.h
	$(CC) -c $< $(COPT) -g

format:
	clang-format -i *.c *.h

clean:
	rm -f *.o dns_svr.log $(BIN_PHASE1) $(BIN_PHASE2)
	# implement this, to remove $(BIN_PHASE[12]) and any .o files
