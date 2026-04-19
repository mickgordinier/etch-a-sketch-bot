# Compiler and flags
CXX := g++
CXXFLAGS := -Wall -Wextra -Iinclude -Ithird_party
CPPFLAGS :=

# Directories
SRC_DIR := src
BUILD_DIR := build
OUTPUT_DIR := output
TARGET := my_program.out

# Find all .cpp files in src/
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

# Default target
all: $(TARGET)

# Link object files into executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^

# Compile .cpp into .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

# Create build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(OUTPUT_DIR) $(TARGET)

# Phony targets
.PHONY: all clean
