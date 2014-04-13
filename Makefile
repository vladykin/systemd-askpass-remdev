TARGET=systemd-ask-password-remdev

OBJECTDIR=build
OBJECTFILES=\
    $(OBJECTDIR)/askfile.o \
    $(OBJECTDIR)/pwdfile.o \
    $(OBJECTDIR)/main.o

SRCFLAGS=-pedantic -Wall -Wextra -std=c99 -D_GNU_SOURCE -D_BSD_SOURCE
CFLAGS=-O2
LDFLAGS=-liniparser


$(TARGET): $(OBJECTFILES)
	$(LINK.c) -o "$@" $(OBJECTFILES)


$(OBJECTDIR)/%.o: src/%.c
	mkdir -p $(OBJECTDIR)
	$(COMPILE.c) $(SRCFLAGS) -o "$@" "$<"


clean:
	rm -r $(OBJECTDIR)
	rm $(TARGET)

