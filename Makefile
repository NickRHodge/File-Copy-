CFLAGS = -g -Wformat-security -Wall -Wextra -Werror -ansi
LDFLAGS = -lpthread
CC = cc
LD = cc

TARG = a5

OBJS = a5.o

$(TARG): $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o $(TARG)

clean:
	rm -f $(OBJS) $(TARG) core a.out
