# Compiler and flags
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O2 -DNDEBUG
CXXFLAGS_DEBUG := -std=c++17 -Wall -Wextra -Wpedantic -g -O0 -DDEBUG
LDFLAGS := -lpthread

# Target executable
TARGET := server
DEBUG_TARGET := server_debug

# Source files
SOURCES := main.cpp

# Default target
.PHONY: all
all: $(TARGET)

# Release build
$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)
	@echo "Build complete: $(TARGET)"
	@echo "Run with: ./$(TARGET) [port]"

# Debug build
.PHONY: debug
debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS_DEBUG) $(SOURCES) -o $(DEBUG_TARGET) $(LDFLAGS)
	@echo "Debug build complete: $(DEBUG_TARGET)"

# Clean build artifacts
.PHONY: clean
clean:
	rm -f $(TARGET) $(DEBUG_TARGET)
	@echo "Clean complete"

# Run the server (default port 8080)
.PHONY: run
run: $(TARGET)
	./$(TARGET)

# Run with custom port
.PHONY: run-port
run-port: $(TARGET)
	@read -p "Enter port number: " port; \
	./$(TARGET) $$port

# Help target
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  make          - Build release version"
	@echo "  make debug    - Build debug version"
	@echo "  make run      - Build and run server on port 8080"
	@echo "  make run-port - Build and run server on custom port"
	@echo "  make clean    - Remove build artifacts"
	@echo "  make help     - Show this help message"
