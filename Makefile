CC                = clang
CFLAGS            = -Wall -g
SDLFLAGS          = `sdl2-config --cflags --libs`
LDFLAGS           = -c
VPATH             = src src/core src/tests
LIBS              = -lportaudio -lportmidi -lck -llo -lSDL2_ttf -lconfig

TEST_C_FLAGS      = $(CFLAGS)
TEST_LIBS         =

DIAGRAM_FORMAT    = png

MAIN_BUILD_DIR    = build/main
TEST_BUILD_DIR    = build/tests
IMAGE_BUILD_DIR   = build/images
HEADER_DIRS       = include

INCLUDES          = $(addprefix -I, $(HEADER_DIRS))

CORE_SOURCES      = $(notdir $(wildcard src/core/*.c))
TEST_SOURCES      = $(notdir $(wildcard src/tests/*.c)) $(CORE_SOURCES)
MAIN_SOURCES      = main.c $(CORE_SOURCES)
DIAGRAMS          = $(notdir $(wildcard diagrams/*.dot))

MAIN_OBJECTS      = $(addprefix $(MAIN_BUILD_DIR)/, $(MAIN_SOURCES:.c=.o))
TEST_OBJECTS      = $(addprefix $(TEST_BUILD_DIR)/, $(TEST_SOURCES:.c=.o))
DIAGRAM_IMAGES    = $(addprefix $(IMAGE_BUILD_DIR)/, $(DIAGRAMS:.dot=.$(DIAGRAM_FORMAT)))

EXECUTABLE        = glacier
TEST_EXECUTABLE   = $(addprefix $(TEST_BUILD_DIR)/, test_$(EXECUTABLE))

.PHONY: clean echo

all: $(EXECUTABLE)

tests: $(TEST_EXECUTABLE)
	./$(TEST_EXECUTABLE)

echo:
	echo $(IMAGE_BUILD_DIR)
	echo $(DIAGRAM_IMAGES)
	echo $(DIAGRAMS)

diagrams: $(DIAGRAM_IMAGES)

$(IMAGE_BUILD_DIR)/%.$(DIAGRAM_FORMAT): diagrams/%.dot
	dot -T$(DIAGRAM_FORMAT) $< -o $@

$(MAIN_BUILD_DIR)/%.o: %.c
	$(CC) $(LDFLAGS) $(CFLAGS) $(INCLUDES) $< -o $@

$(TEST_BUILD_DIR)/%.o: %.c
	$(CC) $(LDFLAGS) $(TEST_CFLAGS) $(INCLUDES) $< -o $@

$(EXECUTABLE): $(MAIN_OBJECTS)
	$(CC) $(LIBS) $(SDLFLAGS) $(MAIN_OBJECTS) -o $@

$(TEST_EXECUTABLE): $(TEST_OBJECTS)
	$(CC) $(LIBS) $(SDLFLAGS) $(TEST_OBJECTS) -o $@

clean:
	rm -rf $(EXECUTABLE)
	rm -rf $(MAIN_BUILD_DIR)/*
	rm -rf $(TEST_BUILD_DIR)/*
	rm -rf $(IMAGE_BUILD_DIR)/*
