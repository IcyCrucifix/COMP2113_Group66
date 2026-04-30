CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -O2

SRC_DIR := src
OBJ_DIR := build
TARGET := slay_text

INCLUDES := -I$(SRC_DIR)/battle -I$(SRC_DIR)/bosses -I$(SRC_DIR)/buff -I$(SRC_DIR)/cards -I$(SRC_DIR)/deck -I$(SRC_DIR)/hero -I$(SRC_DIR)/save_load -I$(SRC_DIR)/utils
SOURCES := \
	$(SRC_DIR)/main.cpp \
	$(SRC_DIR)/hero/hero.cpp \
	$(SRC_DIR)/cards/card.cpp \
	$(SRC_DIR)/deck/deck.cpp \
	$(SRC_DIR)/battle/battle.cpp \
	$(SRC_DIR)/buff/buff.cpp \
	$(SRC_DIR)/save_load/save_load.cpp \
	$(SRC_DIR)/utils/utils.cpp
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SOURCES))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(OBJS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean
