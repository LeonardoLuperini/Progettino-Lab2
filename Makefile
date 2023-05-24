CC			:= gcc
CFLAGS		:= -Wall -Wextra -pedantic
CPPFLAGS	:= -pthread
LDLIBS		:= -lm -lrt
LDFLAGS		:= -pthread $(LDLIBS)

SRC_MAS		:= master.c
SRC_COL		:= collector.c

MAS			:= master.out
COL			:= collector.out
DEB_MAS		:= debmaster.out
DEB_COL		:= debcollector.out

SRC			:= ./src
SRCS		:= $(wildcard $(SRC)/*.c)
MAS_SRCS	:= $(filter-out $(SRC)/$(SRC_COL), $(SRCS))
COL_SRCS	:= $(filter-out $(SRC)/$(SRC_MAS), $(SRCS))
RELFOL		:= ./relobj
DEBFOL		:= ./debobj
MAS_RELOBJS	:= $(MAS_SRCS:$(SRC)/%.c=$(RELFOL)/%.o)
COL_RELOBJS	:= $(COL_SRCS:$(SRC)/%.c=$(RELFOL)/%.o)
MAS_DEBOBJS	:= $(MAS_SRCS:$(SRC)/%.c=$(DEBFOL)/%.o)
COL_DEBOBJS	:= $(COL_SRCS:$(SRC)/%.c=$(DEBFOL)/%.o)

.PHONY: all clean deb

all: $(MAS) $(COL)

$(MAS): $(MAS_RELOBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

$(COL): $(COL_RELOBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

$(RELFOL)/%.o: $(SRC)/%.c
	@mkdir -p $(RELFOL)
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -MF $(RELFOL)/$*.d -o $@ -c $<

-include $(RELFOL)/*.d

deb: $(DEB_MAS) $(DEB_COL)

$(DEB_MAS): $(MAS_DEBOBJS) 
	$(CC) -g -DDEBUG $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

$(DEB_COL): $(COL_DEBOBJS) 
	$(CC) -g -DDEBUG $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

$(DEBFOL)/%.o: $(SRC)/%.c
	@mkdir -p $(DEBFOL)
	$(CC) -g -DDEBUG $(CFLAGS) $(CPPFLAGS) -MMD -MP -MF $(DEBFOL)/$*.d -o $@ -c $<

-include $(DEBFOL)/*.d

clean:
	rm -fr $(MAS) $(DEB_MAS) $(COL) $(DEB_COL) $(RELFOL) $(DEBFOL)
