CC			:= gcc
CFLAGS		:= -Wall -Wextra -pedantic
CPPFLAGS	:= -pthread
LDLIBS		:= -lm -lrt
LDFLAGS		:= -pthread $(LDLIBS)

SRC_MAS		:= master.c
SRC_COL		:= collector.c

BUILDFOL 	:= ./build
MAS			:= $(BUILDFOL)/master.out
COL			:= $(BUILDFOL)/collector.out
DEB_MAS		:= $(BUILDFOL)/debmaster.out
DEB_COL		:= $(BUILDFOL)/debcollector.out

SRC			:= ./src
SRCS		:= $(wildcard $(SRC)/*.c)
MAS_SRCS	:= $(filter-out $(SRC)/$(SRC_COL), $(SRCS))
COL_SRCS	:= $(filter-out $(SRC)/$(SRC_MAS), $(SRCS))
OBJFOL 		:= ./obj
RELFOL		:= $(OBJFOL)/relobj
DEBFOL		:= $(OBJFOL)/debobj
MAS_RELOBJS	:= $(MAS_SRCS:$(SRC)/%.c=$(RELFOL)/%.o)
COL_RELOBJS	:= $(COL_SRCS:$(SRC)/%.c=$(RELFOL)/%.o)
MAS_DEBOBJS	:= $(MAS_SRCS:$(SRC)/%.c=$(DEBFOL)/%.o)
COL_DEBOBJS	:= $(COL_SRCS:$(SRC)/%.c=$(DEBFOL)/%.o)

TESTDIR 	:= ./testdir

.PHONY: all clean deb test1 test2 test3 test4

all: $(BUILDFOL) $(OBJFOL) $(RELFOL) $(MAS) $(COL)

$(MAS): $(MAS_RELOBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

$(COL): $(COL_RELOBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

$(RELFOL)/%.o: $(SRC)/%.c
	@mkdir -p $(RELFOL)
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -MF $(RELFOL)/$*.d -o $@ -c $<

-include $(RELFOL)/*.d

deb: $(BUILDFOL) $(OBJFOL) $(DEBFOL) $(DEB_MAS) $(DEB_COL) 

$(DEB_MAS): $(MAS_DEBOBJS) 
	$(CC) -g -DDEBUG $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

$(DEB_COL): $(COL_DEBOBJS) 
	$(CC) -g -DDEBUG $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

$(DEBFOL)/%.o: $(SRC)/%.c 
	$(CC) -g -DDEBUG $(CFLAGS) $(CPPFLAGS) -MMD -MP -MF $(DEBFOL)/$*.d -o $@ -c $<

-include $(DEBFOL)/*.d

$(DEBFOL):
	@mkdir -p $(DEBFOL)

$(RELFOL):
	@mkdir -p $(RELFOL)

$(OBJFOL):
	@mkdir -p $(OBJFOL)

$(BUILDFOL):
	@mkdir -p $(BUILDFOL)

test1: all
	$(MAS) 1 $(TESTDIR) & $(COL)

test2: all
	$(MAS) 5 $(TESTDIR) & $(COL)

test3: all
	valgrind $(MAS) 5 $(TESTDIR) & valgrind $(COL)

test4: all
	valgrind $(COL) & valgrind $(MAS) 5 $(TESTDIR) 

clean:
	rm -fr $(BUILDFOL) $(OBJFOL) ./socket634318
