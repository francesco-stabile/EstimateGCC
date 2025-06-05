# Nome dell'eseguibile
TARGET = estimate_algorithm

# File sorgente
SRCS = estimate_algorithm.c

# File oggetto
OBJS = $(SRCS:.c=.o)

# Compilatore
CC = gcc

# Flag di compilazione
CFLAGS = -Wall -Wextra

# Flag per l'inclusione e il linking della libreria graphviz (cgraph)
INCLUDES = -I/opt/homebrew/Cellar/graphviz/12.2.1/include
LDFLAGS  = -L/opt/homebrew/Cellar/graphviz/12.2.1/lib
LDLIBS   = -lcgraph

# Controllo del sistema operativo
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)  # macOS
    CFLAGS += -D_DARWIN_C_SOURCE
else  # Linux
    CFLAGS += -D_GNU_SOURCE
endif

# Regola di compilazione principale
all: $(TARGET)

# Regola per creare l'eseguibile
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) $(SRCS) -o $(TARGET) $(LDFLAGS) $(LDLIBS)

# Pulizia dei file compilati
clean:
	rm -f $(TARGET) $(OBJS)

# Rebuild completo
rebuild: clean all
