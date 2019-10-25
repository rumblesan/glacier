CC         = clang
CFLAGS     = -c -Wall
OBJDIR     = .tmp
SOURCES    = $(wildcard src/*.c)
OBJECTS    = $(addprefix $(OBJDIR)/, $(SOURCES:.c=.o))
HEADERS    = include/
EXECUTABLE = glacier

LIBS       = -lportaudio

.PHONE: clean

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LIBS) $(OBJECTS) -o $@

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -I $(HEADERS) $(LDFLAGS) $< -o $@

clean:
	rm -rf $(OBJDIR)/src/*.o $(EXECUTABLE)
