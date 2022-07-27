# Compiler
CC=gcc

# Directories
SRCDIR=src
INCDIR=include
BINDIR=bin
OBJDIR=obj

# Files
SRC=$(wildcard $(SRCDIR)/*.c)
BIN=$(BINDIR)/watcher
OBJ=$(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC))

# install
install: all
	cp $(BIN) /usr/bin/

# uninstall
uninstall:
	rm /usr/$(BIN)

# all
all: dir $(BIN)

# Directory structure
dir:
	mkdir -p $(SRCDIR) $(INCDIR) $(OBJDIR) $(BINDIR)

# Compile objects to elf
$(BIN): $(OBJ)
	$(CC) $^ -o $(BIN)

# Compile c to objects
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -c $< -o $@ -I$(INCDIR)

# Clean
clean:
	rm -rf $(BINDIR) $(OBJDIR)
