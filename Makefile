# Define the target executable and object files
TARGET = main
OBJS = main.o sqlite3.o gen_account_number.o utils_functions.o customer_system.o account_system.o

# Compiler flags
CFLAGS = -I.

# Default target
all: $(TARGET)

# Build the uuid library
uuid/libuuid.a:
	$(MAKE) -C uuid

# Compile the main program
$(TARGET): $(OBJS) uuid/libuuid.a
	$(CC) $(CFLAGS) $(OBJS) -L./uuid -luuid -o $(TARGET)

# Compile .c files to .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	$(MAKE) -C uuid clean
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
