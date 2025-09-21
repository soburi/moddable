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

WEST ?= west
ZEPHYR_BOARD ?= native_sim/native/64
ZEPHYR_APP ?= $(MODDABLE)/build/devices/zephyr/app
ZEPHYR_BUILD_DIR ?= $(BIN_DIR)/zephyr

XS_DIR ?= $(MODDABLE)/xs

ifeq ($(DEBUG),1)
BUILD_TYPE = Debug
else
BUILD_TYPE = Release
endif

.PHONY: all build clean

ifeq ($(strip $(WEST_FLAGS)),)
WEST_CMAKE_FLAGS =
else
WEST_CMAKE_FLAGS = -- $(WEST_FLAGS)
endif

all: build

build: $(TMP_DIR)/xs_sources.cmake
	@echo "# west build $(ZEPHYR_BOARD)"
	MCGEN_DIR=$(TMP_DIR) $(WEST) build -b $(ZEPHYR_BOARD) $(ZEPHYR_APP) --build-dir $(ZEPHYR_BUILD_DIR) $(WEST_CMAKE_FLAGS)

clean:
	@echo "# Clean Zephyr build"
	-$(WEST) build --build-dir $(ZEPHYR_BUILD_DIR) -t clean >/dev/null 2>&1 || true
	-rm -rf $(ZEPHYR_BUILD_DIR)
	-rm -rf $(TMP_DIR)

XS_PLATFORM_SOURCES = \
        $(XS_DIR)/platforms/zephyr/xsPlatform.c \
        $(XS_DIR)/platforms/zephyr/xsHost.c \
        $(XS_DIR)/platforms/mc/xsHosts.c

XS_RUNTIME_SOURCES = \
        $(XS_DIR)/sources/xsAll.c \
        $(XS_DIR)/sources/xsAPI.c \
        $(XS_DIR)/sources/xsArguments.c \
        $(XS_DIR)/sources/xsArray.c \
        $(XS_DIR)/sources/xsAtomics.c \
        $(XS_DIR)/sources/xsBigInt.c \
        $(XS_DIR)/sources/xsBoolean.c \
        $(XS_DIR)/sources/xsCode.c \
        $(XS_DIR)/sources/xsCommon.c \
        $(XS_DIR)/sources/xsDataView.c \
        $(XS_DIR)/sources/xsDate.c \
        $(XS_DIR)/sources/xsDebug.c \
        $(XS_DIR)/sources/xsError.c \
        $(XS_DIR)/sources/xsFunction.c \
        $(XS_DIR)/sources/xsGenerator.c \
        $(XS_DIR)/sources/xsGlobal.c \
        $(XS_DIR)/sources/xsJSON.c \
        $(XS_DIR)/sources/xsLexical.c \
        $(XS_DIR)/sources/xsMapSet.c \
        $(XS_DIR)/sources/xsMarshall.c \
        $(XS_DIR)/sources/xsMath.c \
        $(XS_DIR)/sources/xsMemory.c \
        $(XS_DIR)/sources/xsModule.c \
        $(XS_DIR)/sources/xsNumber.c \
        $(XS_DIR)/sources/xsObject.c \
        $(XS_DIR)/sources/xsPlatforms.c \
        $(XS_DIR)/sources/xsPromise.c \
        $(XS_DIR)/sources/xsProperty.c \
        $(XS_DIR)/sources/xsProxy.c \
        $(XS_DIR)/sources/xsRegExp.c \
        $(XS_DIR)/sources/xsRun.c \
        $(XS_DIR)/sources/xsScope.c \
        $(XS_DIR)/sources/xsScript.c \
        $(XS_DIR)/sources/xsSourceMap.c \
        $(XS_DIR)/sources/xsString.c \
        $(XS_DIR)/sources/xsSymbol.c \
        $(XS_DIR)/sources/xsSyntaxical.c \
        $(XS_DIR)/sources/xsTree.c \
        $(XS_DIR)/sources/xsType.c \
        $(XS_DIR)/sources/xsdtoa.c \
        $(XS_DIR)/sources/xsmc.c \
        $(XS_DIR)/sources/xsre.c

ifeq ($(COMMODETTOBITMAPFORMAT),)
BITMAP_DEFINE =
else
BITMAP_DEFINE = kCommodettoBitmapFormat=$(COMMODETTOBITMAPFORMAT)
endif

ifeq ($(POCOROTATION),)
ROTATION_DEFINE =
else
ROTATION_DEFINE = kPocoRotation=$(POCOROTATION)
endif

COMMON_DEFINES = XS_ARCHIVE=1 INCLUDE_XSPLATFORM=1 __ZEPHYR__

ifeq ($(DEBUG),1)
COMMON_DEFINES += _DEBUG=1 mxDebug=1
else
COMMON_DEFINES += _RELEASE=1
endif

INCLUDE_DIRS = $(sort $(dir $(CSOURCES)))

ifeq ($(INSTRUMENT),1)
COMMON_DEFINES += MODINSTRUMENTATION=1 mxInstrument=1
endif

ifneq ($(BITMAP_DEFINE),)
COMMON_DEFINES += $(BITMAP_DEFINE)
endif

ifneq ($(ROTATION_DEFINE),)
COMMON_DEFINES += $(ROTATION_DEFINE)
endif


$(TMP_DIR)/xs_sources.cmake: $(CSOURCES) $(HEADERS)
	@echo "# generate xs_sources.cmake"
	@mkdir -p $(TMP_DIR)
	@{ \
	echo "# Auto-generated file"; \
	echo "set(MODDABLE_ROOT \"$(MODDABLE)\")"; \
	echo "set(XS_DIR \"$(XS_DIR)\")"; \
	echo "set(MCGEN_DIR \"$(TMP_DIR)\")"; \
	echo ""; \
	echo "set(XS_PLATFORM_SOURCES"; \
	for src in $(XS_PLATFORM_SOURCES); do \
	echo "    \"$${src}\""; \
	done; \
	echo ")"; \
	echo ""; \
	echo "set(XS_RUNTIME_SOURCES"; \
	for src in $(XS_RUNTIME_SOURCES); do \
	echo "    \"$${src}\""; \
	done; \
	echo ")"; \
	echo ""; \
	echo "set(MC_SOURCES"; \
	for src in $(CSOURCES); do \
	echo "    \"$${src}\""; \
	done; \
	echo ")"; \
	echo ""; \
	echo "set(MC_INCLUDE_DIRS"; \
	echo "    \"$(TMP_DIR)\""; \
	for dir in $(INCLUDE_DIRS); do \
	echo "    \"$${dir}\""; \
	done; \
	echo "    \"$(XS_DIR)/includes\""; \
	echo "    \"$(XS_DIR)/sources\""; \
	echo "    \"$(XS_DIR)/platforms\""; \
	echo "    \"$(XS_DIR)/platforms/zephyr\""; \
	echo "    \"$(XS_DIR)/platforms/mc\""; \
	echo ")"; \
	echo ""; \
	echo "set(MC_COMPILE_DEFINITIONS"; \
	for def in $(COMMON_DEFINES); do \
	echo "    \"$${def}\""; \
	done; \
	echo ")"; \
	} > $@
