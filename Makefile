GITHUB_DEPS += simplerobot/build-scripts
GITHUB_DEPS += simplerobot/test
GITHUB_DEPS += simplerobot/logger
include ../build-scripts/build/release/include.make

BUILD_DIR = build
LIBRARY_BUILD_DIR = $(BUILD_DIR)/library
TEST_BUILD_DIR = $(BUILD_DIR)/test
RELEASE_DIR = $(BUILD_DIR)/release

CC = g++
CFLAGS = -Wall -Werror -DTEST -I$(MAIN_SOURCE_DIR) -I$(PKG_TEST_DIR) -I$(PKG_LOGGER_DIR) -fsanitize=address -static-libasan -g -Og

SOURCE_DIR = source
MAIN_SOURCE_DIR = $(SOURCE_DIR)/main
TEST_SOURCE_DIR = $(SOURCE_DIR)/test

LIBRARY_FILES = $(notdir $(wildcard $(MAIN_SOURCE_DIR)/*))

TEST_SOURCE_DIRS = $(MAIN_SOURCE_DIR) $(TEST_SOURCE_DIR) $(PKG_TEST_DIR) $(PKG_LOGGER_DIR)
TEST_SOURCE_FILES = $(notdir $(wildcard $(TEST_SOURCE_DIRS:%=%/*.cpp) $(TEST_SOURCE_DIRS:%=%/*.c)))
TEST_O_FILES = $(addsuffix .o,$(basename $(TEST_SOURCE_FILES)))

VPATH = $(TEST_SOURCE_DIRS)

.PHONY: default all library test release clean

default : release

all : release

library : $(LIBRARY_FILES:%=$(LIBRARY_BUILD_DIR)/%)

$(LIBRARY_BUILD_DIR)/% : $(MAIN_SOURCE_DIR)/% | $(LIBRARY_BUILD_DIR)
	cp $< $@

$(LIBRARY_BUILD_DIR) :
	mkdir -p $@

test : library $(TEST_BUILD_DIR)/a.out
	$(TEST_BUILD_DIR)/a.out

$(TEST_BUILD_DIR)/a.out : $(TEST_O_FILES:%=$(TEST_BUILD_DIR)/%)
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_BUILD_DIR)/%.o : %.cpp Makefile | $(TEST_BUILD_DIR)
	$(CC) -c $(CFLAGS) -MMD -o $@ $<

$(TEST_BUILD_DIR)/%.o : %.c Makefile | $(TEST_BUILD_DIR)
	$(CC) -c $(CFLAGS) -MMD -o $@ $<

$(TEST_BUILD_DIR) :
	mkdir -p $@

release: library test $(LIBRARY_FILES:%=$(RELEASE_DIR)/%)

$(RELEASE_DIR)/% : $(LIBRARY_BUILD_DIR)/% | $(RELEASE_DIR)
	cp $< $@

$(RELEASE_DIR) :
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

-include $(wildcard $(TEST_BUILD_DIR)/*.d)
