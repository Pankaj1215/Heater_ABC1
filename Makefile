#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := eheat

PROJ_DIR = $(CURDIR)/../../

#PATH := $(PATH):$(HOME)/esp32_sandbox/toolchain_list/extensa-esp32-elf_new/bin
PATH := $(PATH):$(PROJ_DIR)/toolchain_list/xtensa-esp32-elf_new/bin/
SHELL := env PATH=$(PATH) /bin/bash

EXTRA_COMPONENT_DIRS =: $(PROJ_DIR)/components

IDF_PATH = $(PROJ_DIR)/sdk_list/esp-idf_40
include $(IDF_PATH)/make/project.mk
