CC := g++
SRCEXT := cpp
BUILDDIR := build
TARGET := bin/cpp-networking

# Program files
PROGRAM_DIR := src
PROGRAM_SOURCE := $(shell find $(PROGRAM_DIR) -type f -name *.$(SRCEXT))
PROGRAM_OBJECTS := $(patsubst $(PROGRAM_DIR)/%,$(BUILDDIR)/%,$(PROGRAM_SOURCE:.$(SRCEXT)=.o))

# Test Files
TEST_DIR := test
TEST_SOURCE := $(shell find $(TEST_DIR) -type f -name *.$(SRCEXT))
TEST_OBJECTS := $(patsubst $(TEST_DIR)/%, $(BUILDDIR)/test/%, $(TEST_SOURCE:.$(SRCEXT)=.o))

# Compiler flags
CFLAGS := -g -std=c++14 -pedantic-errors -Wall -shared
INC := -Iinclude
LIBS := -lboost_system -lboost_thread -lpthread -lboost_unit_test_framework -lboost_log_setup -lboost_log -lboost_date_time -lboost_filesystem # -lcrypto -lssl

# Top executable linking
$(TARGET): $(PROGRAM_OBJECTS)
	$(CC) $^ -o $(TARGET) $(LIBS)

test: $(PROGRAM_OBJECTS) $(TEST_OBJECTS)
	$(CC) $^ -o bin/test $(LIBS)

standalone/%: $(PROGRAM_OBJECTS)
	$(CC) $(CFLAGS) $(INC) $^ $@.$(SRCEXT) -o bin/$(@F) $(LIBS)

# Build directory targets. Order them from most to least specific
$(BUILDDIR)/test/%.o: $(TEST_DIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<
$(BUILDDIR)/%.o: $(PROGRAM_DIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	@echo " Cleaning..."
	$(RM) -r $(BUILDDIR) bin/*

.PHONY: clean test
