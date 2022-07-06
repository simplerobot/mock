GITHUB_DEPS += simplerobot/build-scripts
GITHUB_DEPS += simplerobot/test
include ../build-scripts/build/release/include.make

BUILD_DIR = build
LIBRARY_BUILD_DIR = $(BUILD_DIR)/library
TEST_BUILD_DIR = $(BUILD_DIR)/test
RELEASE_DIR = $(BUILD_DIR)/release

CC = g++
CFLAGS = -Wall -Werror -DTEST -I$(LIBRARY_BUILD_DIR) -I$(PKG_TEST_DIR) -fsanitize=address -static-libasan -g -Og

SOURCE_DIR = source
MAIN_SOURCE_DIR = $(SOURCE_DIR)/main
TEST_SOURCE_DIR = $(SOURCE_DIR)/test

LIBRARY_FILES = $(notdir $(wildcard $(MAIN_SOURCE_DIR)/*))
TEST_CPP_SOURCES = $(notdir $(wildcard $(MAIN_SOURCE_DIR)/*.cpp $(TEST_SOURCE_DIR)/*.cpp $(PKG_TEST_DIR)/*.cpp))

VPATH = $(MAIN_SOURCE_DIR) $(TEST_SOURCE_DIR) $(PKG_TEST_DIR)

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

$(TEST_BUILD_DIR)/a.out : $(TEST_CPP_SOURCES:%.cpp=$(TEST_BUILD_DIR)/%.o)
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_BUILD_DIR)/%.o : %.cpp Makefile | $(TEST_BUILD_DIR)
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
