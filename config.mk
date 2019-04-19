VERSION := v0.0.1

# System paths
PREFIX := /usr/local
BINPREFIX := $(PREFIX)/bin
INCLUDEPREFIX := $(PREFIX)/include
LIBPREFIX := $(PREFIX)/lib
MANPREFIX := $(PREFIX)/man

# Linking flags
LDFLAGS :=

# C Compiler settings
CC := cc
CFLAGS := -O2 -std=c99 -Iinclude -pedantic -Wall -Wextra -g
