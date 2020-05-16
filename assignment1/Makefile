TARGET=collatz
LDFLAGS=
CFLAGS+=-Wall -Wextra -fdiagnostics-color=auto
BUILDDIR=build

OBJECTS=build/arguments.o build/assignment1.o

.PHONY: all clean fresh

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $(TARGET)

$(BUILDDIR)/%.o: src/%.c $(BUILDDIR)
	$(CC) $(CFLAGS) $< -c -o $@

$(BUILDDIR):
	mkdir $(BUILDDIR)

clean:
	-rm -rf $(BUILDDIR)
	-rm -f $(TARGET)

fresh: clean all
