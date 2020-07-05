TARGET=trace6
TARGETJAR=$(TARGET).jar
CLASSFOLDER = classes
SOURCEFOLDER = src
CLASSES = Arguments GRNVS_RAW Timeout Assignment3
JAVAFILES = $(addprefix src/, $(addsuffix .java, $(CLASSES)))

# hack to get a JAVA_HOME
JAVA_HOME ?= $(shell dirname $(shell readlink -f $(shell which javac)))
JDK_INCLUDE:= $(JAVA_HOME)/../include

# LIBRAW/GRNVS_SOCK required stuff
LIBSOCKET=deps/libGRNVS.so
LIBRAWDIR=deps/libraw/
LIBRAW=$(LIBRAWDIR)libraw.a
LDFLAGS=$(LIBRAW)
CFLAGS+=-I$(LIBRAWDIR)include -I$(JDK_INCLUDE)
CFLAGS+=-I$(JDK_INCLUDE)/linux -Ideps -fPIC
CFLAGS+=-fdiagnostics-color=always

.PHONY: all clean fresh

all: $(TARGET) $(TARGETJAR) $(LIBSOCKET)

$(TARGETJAR): $(CLASSFOLDER)/.empty $(JAVAFILES)
	javac -encoding UTF-8 -classpath $(SOURCEFOLDER) -d $(CLASSFOLDER) $(JAVAFILES)
	@jar cfm $(TARGETJAR) deps/manifest.txt -C $(CLASSFOLDER) .


#sed replaces CRLF with just LF for windows + java users
$(TARGET): deps/run
	cp deps/run $(TARGET)
	@sed 's/\r$$//' -i $(TARGET)
	@chmod +x $(TARGET)

.PRECIOUS: $(CLASSFOLDER)/.empty
$(CLASSFOLDER)/.empty:
	@mkdir -p $(CLASSFOLDER)
	@touch $(CLASSFOLDER)/.empty

LIBSOCKETOBJECTS= deps/GRNVS_RAW.o

$(LIBSOCKET): $(LIBSOCKETOBJECTS) $(LIBRAW)
	$(CC) $(LIBSOCKETOBJECTS) $(LDFLAGS) -shared -o $(LIBSOCKET)

$(LIBRAW):
	$(MAKE) -C $(LIBRAWDIR)

clean:
	-$(MAKE) -C $(LIBRAWDIR) clean
	-rm -rf $(CLASSFOLDER)
	-rm -f $(TARGET)
	-rm -f $(LIBSOCKETOBJECTS)
	-rm -f $(LIBSOCKET)
	-rm -f $(TARGETJAR)

fresh: clean
	$(MAKE) all
