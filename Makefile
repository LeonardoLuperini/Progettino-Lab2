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

.PHONY: all val deb clean test1 test2 test3 test4 test5 test6 testfun

all: $(BUILDFOL) $(OBJFOL) $(RELFOL) $(MAS) $(COL)

$(MAS): $(MAS_RELOBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

$(COL): $(COL_RELOBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

$(RELFOL)/%.o: $(SRC)/%.c
	@mkdir -p $(RELFOL)
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -MF $(RELFOL)/$*.d -o $@ -c $<

-include $(RELFOL)/*.d

val: CFLAGS += -g 
val: $(BUILDFOL) $(OBJFOL) $(DEBFOL) $(DEB_MAS) $(DEB_COL) 

$(DEB_MAS): $(MAS_DEBOBJS) 
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

$(DEB_COL): $(COL_DEBOBJS) 
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

$(DEBFOL)/%.o: $(SRC)/%.c 
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -MF $(DEBFOL)/$*.d -o $@ -c $<

-include $(DEBFOL)/*.d

deb: CFLAGS += -DDEBUG
deb: val

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

test3: val
	valgrind $(DEB_MAS) 5 $(TESTDIR) & valgrind $(DEB_COL)

test4: val
	valgrind $(DEB_COL) & valgrind $(DEB_MAS) 5 $(TESTDIR) 

test5: deb
	valgrind $(DEB_MAS) 5 $(TESTDIR) & valgrind $(DEB_COL)

test6: deb
	valgrind $(DEB_COL) & valgrind $(DEB_MAS) 5 $(TESTDIR) 

testfun: all
	$(MAS) 1000 $(TESTDIR) & $(COL)

clean:
	rm -fr $(BUILDFOL) $(OBJFOL) ./socket634318
