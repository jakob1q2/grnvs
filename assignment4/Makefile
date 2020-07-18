TARGET=asciiclient
LDFLAGS=
CFLAGS+=-Wall -Wextra -fdiagnostics-color=auto -g

BUILDDIR=build
OBJECTS=$(BUILDDIR)/arguments.o $(BUILDDIR)/assignment4.o


.PHONY: all clean fresh

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $(TARGET)




$(BUILDDIR)/%.o: src/%.c $(BUILDDIR)/.empty
	$(CC) $(CFLAGS) $< -c -o $@

.PRECIOUS: $(BUILDDIR)/.empty

$(BUILDDIR)/.empty:
	@mkdir -p $(BUILDDIR)
	@touch $@

clean:
	-rm -rf $(BUILDDIR)
	-rm -f $(TARGET)

fresh: clean
	$(MAKE) all
