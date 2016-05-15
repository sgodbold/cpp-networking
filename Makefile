CC := g++
SRCEXT := cpp
BUILDDIR := build

TARGET := bin/dropbox-mnt

# Program files
SOURCEDIR := src
SOURCES := $(shell find $(SOURCEDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SOURCEDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))

# Test files
TESTDIR := test
TEST_SRC := $(shell find $(TESTDIR) -type f -name *.$(SRCEXT))
TEST_OBJECTS := $(patsubst $(TESTDIR)/%,$(BUILDDIR)/test/%,$(TEST_SRC:.$(SRCEXT)=.o))
# TEST_MAIN := $(BUILDDIR)/test/tester.o

# Compiler flags
CFLAGS := -g -std=c++14 -pedantic-errors -Wall
LIBS := -lboost_system -lboost_thread # -lpthread # -lcrypto -lssl
INC := -Iinclude

# Top executable linking
$(TARGET): $(OBJECTS)
	@echo " Linking..."
	$(CC) $^ -o $(TARGET) $(LIBS)
# test: $(filter-out $(SOURCE_MAIN), $(OBJECTS)) $(TEST_OBJECTS)
test: $(OBJECTS) $(TEST_OBJECTS)
	@echo " Linking..."
	$(CC) $^ -o bin/test $(LIBS)

standalone/%: $(OBJECTS)
	$(CC) -lpthread $(CFLAGS) $(INC) $^ $@.$(SRCEXT) -o bin/$(@F) $(LIBS)

# Build director targets. Order them from most to least specific
$(BUILDDIR)/%.o: $(SOURCEDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	@echo " Cleaning..."
	$(RM) -r $(BUILDDIR) $(TARGET)

.PHONY: clean
