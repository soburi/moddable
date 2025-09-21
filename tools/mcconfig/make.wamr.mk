#
# Copyright (c) 2016-2025  Moddable Tech, Inc.
#
#   This file is part of the Moddable SDK Tools.
# 
#   The Moddable SDK Tools is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
# 
#   The Moddable SDK Tools is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
# 
#   You should have received a copy of the GNU General Public License
#   along with the Moddable SDK Tools.  If not, see <http://www.gnu.org/licenses/>.
#

WASI_SDK_PATH ?=
WAMR_RUNTIME ?= iwasm

ifeq ($(strip $(WASI_SDK_PATH)),)
CC ?= clang
AR ?= llvm-ar
SYSROOT ?=
else
CC ?= $(WASI_SDK_PATH)/bin/clang
AR ?= $(WASI_SDK_PATH)/bin/llvm-ar
SYSROOT ?= $(WASI_SDK_PATH)/share/wasi-sysroot
SYSROOT_FLAG := $(if $(strip $(SYSROOT)),--sysroot=$(SYSROOT),)
endif

WAMR_STACK_SIZE ?= 1048576
WAMR_RUNTIME_FLAGS ?=

WAMR_DEFAULT_RUNTIME_FLAGS := --stack-size=$(strip $(WAMR_STACK_SIZE))
WAMR_ALL_RUNTIME_FLAGS := $(strip $(WAMR_DEFAULT_RUNTIME_FLAGS) $(WAMR_RUNTIME_FLAGS))

TARGET_FLAG := --target=wasm32-wasi

XS_DIRECTORIES = \
	$(XS_DIR)/includes \
	$(XS_DIR)/platforms \
	$(XS_DIR)/sources

XS_HEADERS = \
	$(XS_DIR)/platforms/wamr_xs.h \
	$(XS_DIR)/platforms/xsPlatform.h \
	$(XS_DIR)/includes/xs.h \
	$(XS_DIR)/includes/xsmc.h \
	$(XS_DIR)/sources/xsCommon.h \
	$(XS_DIR)/sources/xsAll.h \
	$(XS_DIR)/sources/xsScript.h

XS_OBJECTS = \
	$(LIB_DIR)/wamr_xs.c.o \
	$(LIB_DIR)/xsAll.c.o \
	$(LIB_DIR)/xsAPI.c.o \
	$(LIB_DIR)/xsArguments.c.o \
	$(LIB_DIR)/xsArray.c.o \
	$(LIB_DIR)/xsAtomics.c.o \
	$(LIB_DIR)/xsBigInt.c.o \
	$(LIB_DIR)/xsBoolean.c.o \
	$(LIB_DIR)/xsCode.c.o \
	$(LIB_DIR)/xsCommon.c.o \
	$(LIB_DIR)/xsDataView.c.o \
	$(LIB_DIR)/xsDate.c.o \
	$(LIB_DIR)/xsDebug.c.o \
	$(LIB_DIR)/xsError.c.o \
	$(LIB_DIR)/xsFunction.c.o \
	$(LIB_DIR)/xsGenerator.c.o \
	$(LIB_DIR)/xsGlobal.c.o \
	$(LIB_DIR)/xsJSON.c.o \
	$(LIB_DIR)/xsLexical.c.o \
	$(LIB_DIR)/xsMapSet.c.o \
	$(LIB_DIR)/xsMarshall.c.o \
	$(LIB_DIR)/xsMath.c.o \
	$(LIB_DIR)/xsMemory.c.o \
	$(LIB_DIR)/xsModule.c.o \
	$(LIB_DIR)/xsNumber.c.o \
	$(LIB_DIR)/xsObject.c.o \
	$(LIB_DIR)/xsPlatforms.c.o \
	$(LIB_DIR)/xsPromise.c.o \
	$(LIB_DIR)/xsProperty.c.o \
	$(LIB_DIR)/xsProxy.c.o \
	$(LIB_DIR)/xsRegExp.c.o \
	$(LIB_DIR)/xsRun.c.o \
	$(LIB_DIR)/xsScope.c.o \
	$(LIB_DIR)/xsScript.c.o \
	$(LIB_DIR)/xsSourceMap.c.o \
	$(LIB_DIR)/xsString.c.o \
	$(LIB_DIR)/xsSymbol.c.o \
	$(LIB_DIR)/xsSyntaxical.c.o \
	$(LIB_DIR)/xsTree.c.o \
	$(LIB_DIR)/xsType.c.o \
	$(LIB_DIR)/xsdtoa.c.o \
	$(LIB_DIR)/xsmc.c.o \
	$(LIB_DIR)/xsre.c.o

HEADERS += $(XS_HEADERS)

C_DEFINES = \
	-DXS_ARCHIVE=1 \
	-DINCLUDE_XSPLATFORM=1 \
	-DXSPLATFORM=\"wamr_xs.h\" \
	-DkCommodettoBitmapFormat=$(COMMODETTOBITMAPFORMAT) \
	-DkPocoRotation=$(POCOROTATION)

C_INCLUDES += $(DIRECTORIES)
C_INCLUDES += $(foreach dir,$(XS_DIRECTORIES) $(TMP_DIR),-I$(dir))
C_INCLUDES += -I$(BUILD_DIR)/simulators/modules

C_FLAGS = -c $(TARGET_FLAG) $(SYSROOT_FLAG) -D_WASI_EMULATED_PROCESS_CLOCKS  -mllvm -wasm-enable-sjlj
ifeq ($(DEBUG),)
	C_FLAGS += -D_RELEASE=1 -O3
else
        C_FLAGS += -D_DEBUG=1 -DmxDebug=1 -g -Og -Wall -Wextra -Wno-missing-field-initializers -Wno-unused-parameter
endif

LINK_OPTIONS = \
	$(TARGET_FLAG) $(SYSROOT_FLAG) \
	-Wl,--no-entry \
	-Wl,--export=fxMainIdle \
	-Wl,--export=fxMainLaunch \
	-Wl,--export=fxMainQuit \
	-Wl,--export=fxMainTouch

LINK_LIBRARIES = -lc -lsetjmp -lm

VPATH += $(XS_DIRECTORIES)

.PHONY: all clean build run

all: build

build: $(LIB_DIR) $(BIN_DIR)/mc.wasm

run: build
	@echo "# run mc.wasm"
	$(WAMR_RUNTIME) $(WAMR_ALL_RUNTIME_FLAGS) $(BIN_DIR)/mc.wasm

$(LIB_DIR):
	mkdir -p $(LIB_DIR)
	
$(BIN_DIR)/mc.wasm: $(XS_OBJECTS) $(TMP_DIR)/mc.xs.c.o $(TMP_DIR)/mc.resources.c.o $(OBJECTS) $(TMP_DIR)/mc.main.c.o
	@echo "# ld mc.wasm"
	$(CC) -O3 $(LINK_OPTIONS) $(LINK_LIBRARIES) $^ -o $@

$(XS_OBJECTS) : $(XS_HEADERS)
$(LIB_DIR)/%.c.o: %.c
	@echo "# cc" $(<F)
	$(CC) $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) $< -o $@
	
$(TMP_DIR)/mc.xs.c.o: $(TMP_DIR)/mc.xs.c $(HEADERS)
	@echo "# cc" $(<F)
	$(CC) $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) $< -o $@
	
$(TMP_DIR)/mc.xs.c: $(MODULES) $(MANIFEST)
	@echo "# xsl modules"
	xsl -b $(MODULES_DIR) -o $(TMP_DIR) $(PRELOADS) $(STRIPS) $(CREATION) $(MODULES)

$(TMP_DIR)/mc.xs.h: $(TMP_DIR)/mc.xs.c

$(TMP_DIR)/mc.resources.c.o: $(TMP_DIR)/mc.resources.c $(HEADERS)
	@echo "# cc" $(<F)
	$(CC) $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) $< -o $@

$(TMP_DIR)/mc.resources.c: $(DATA) $(RESOURCES) $(MANIFEST)
	@echo "# mcrez resources"
	mcrez $(DATA) $(RESOURCES) -o $(TMP_DIR) -r mc.resources.c
	
$(TMP_DIR)/mc.main.c.o: $(BUILD_DIR)/makefiles/wamr/main.c $(HEADERS) $(TMP_DIR)/mc.xs.h
	@echo "# cc" $(<F)
	$(CC) $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) $< -o $@
	
MAKEFLAGS += --jobs
ifneq ($(VERBOSE),1)
MAKEFLAGS += --silent
endif
