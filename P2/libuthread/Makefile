# Uncomment object files when completed
objs := queue.o uthread.o context.o sem.o preempt.o

CC := gcc

## uncomment for options
CFLAGS := -Wall -Wextra
CFLAGS += -Werror # Treat all warnings as errors
CFLAGS += -MMD #for dependency tracking

## Debug if make D=1, otherwise optomise flag
ifneq ($(D),1)
CFLAGS  += -O2
else
CFLAGS  += -g
endif

## Print all flag options
ifneq ($(V),1)
Q=@
endif

# Target library
lib := libuthread.a

all: $(lib)

deps := $(patsubst %.o,%.d,$(objs))
-include $(deps)

## TODO: Phase 1
libuthread.a: $(objs)
	@echo "CC $@"
	$(Q)ar -rcs $@ $^

%.o: %.c
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

clean: 
	@echo "clean"
	$(Q)rm -f $(lib) $(objs) $(deps)

