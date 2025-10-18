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

# Zephyr application locations and build configuration
ZEPHYR_APP_DIR ?= $(MODDABLE)/build/devices/zephyr/app
ZEPHYR_BUILD_DIR ?= $(TMP_DIR)/zephyr
ZEPHYR_BINARY_DIR = $(ZEPHYR_BUILD_DIR)/build
ZEPHYR_GENERATOR ?= Ninja
ZEPHYR_CMAKE ?= cmake

ifeq ($(strip $(ZEPHYR_GENERATOR)),)
ZEPHYR_GENERATOR_ARG =
else
ZEPHYR_GENERATOR_ARG = -G "$(ZEPHYR_GENERATOR)"
endif

ZEPHYR_BUILD_TYPE ?= $(if $(DEBUG),Debug,Release)
ZEPHYR_CMAKE_ARGS += -DCMAKE_BUILD_TYPE=$(ZEPHYR_BUILD_TYPE)

ifdef BOARD
ZEPHYR_CMAKE_ARGS += -DBOARD=$(BOARD)
endif

.PHONY: all clean zephyr-configure zephyr-build

XS_SOURCES = $(TMP_DIR)/xs_sources.cmake

XS_RUNTIME_SOURCES_LIST = \
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

XS_PLATFORM_SOURCES_LIST = \
$(XS_DIR)/platforms/mc/xsHosts.c \
$(XS_DIR)/platforms/zephyr/xsHost.c \
$(XS_DIR)/platforms/zephyr/xsPlatform.c

all: zephyr-build

zephyr-build: zephyr-configure
	@echo "# build zephyr"
	@$(ZEPHYR_CMAKE) --build $(ZEPHYR_BINARY_DIR)

zephyr-configure: $(XS_SOURCES)
	@echo "# configure zephyr"
	@mkdir -p $(ZEPHYR_BINARY_DIR)
	@MCGEN_DIR=$(TMP_DIR) $(ZEPHYR_CMAKE) $(ZEPHYR_GENERATOR_ARG) $(ZEPHYR_CMAKE_ARGS) -S $(ZEPHYR_APP_DIR) -B $(ZEPHYR_BINARY_DIR)

$(XS_SOURCES): $(TMP_DIR)/mc.xs.c $(TMP_DIR)/mc.resources.c
	@echo "# generate xs_sources.cmake"
	@mkdir -p $(dir $@)
	@echo "set(MODDABLE_MC_XS $(TMP_DIR)/mc.xs.c)" > $@
	@echo "set(MODDABLE_MC_RESOURCES $(TMP_DIR)/mc.resources.c)" >> $@
	@printf "set(XS_RUNTIME_SOURCES" >> $@
	@for file in $(XS_RUNTIME_SOURCES_LIST); do \
	printf "\n    %s" "$$file" >> $@; \
	done; \
	printf "\n)\n" >> $@
	@printf "set(XS_PLATFORM_SOURCES" >> $@
	@for file in $(XS_PLATFORM_SOURCES_LIST); do \
	printf "\n    %s" "$$file" >> $@; \
	done; \
	printf "\n)\n" >> $@
	@printf "set(MC_SOURCES" >> $@
	@for file in $(CSOURCES); do \
	printf "\n    %s" "$$file" >> $@; \
	done; \
	printf "\n)\n" >> $@
	@printf "set(MC_INCLUDE_DIRS" >> $@
	@for dir in $(INCLUDE_DIRS) $(TMP_DIR) $(XS_DIR)/includes $(XS_DIR)/platforms $(XS_DIR)/sources; do \
	printf "\n    %s" "$$dir" >> $@; \
	done; \
	printf "\n)\n" >> $@
	@printf "set(MC_COMPILE_DEFINITIONS\n    XS_ARCHIVE=1\n    INCLUDE_XSPLATFORM=1\n    XSPLATFORM=\"xsPlatform.h\"" >> $@
	@if [ -n "$(COMMODETTOBITMAPFORMAT)" ]; then \
	printf "\n    kCommodettoBitmapFormat=%s" "$(COMMODETTOBITMAPFORMAT)" >> $@; \
	fi
	@if [ -n "$(POCOROTATION)" ]; then \
	printf "\n    kPocoRotation=%s" "$(POCOROTATION)" >> $@; \
	fi
	@if [ "$(INSTRUMENT)" = "1" ]; then \
	printf "\n    MODINSTRUMENTATION=1\n    mxInstrument=1" >> $@; \
	fi
	@if [ "$(DEBUG)" = "1" ]; then \
	printf "\n    _DEBUG=1\n    mxDebug=1" >> $@; \
	fi
	@printf "\n)\n" >> $@

$(TMP_DIR)/mc.xs.c: $(MODULES) $(MANIFEST)
	@echo "# xsl modules"
	xsl -b $(MODULES_DIR) -o $(TMP_DIR) $(PRELOADS) $(STRIPS) $(CREATION) $(MODULES)

$(TMP_DIR)/mc.resources.c: $(DATA) $(RESOURCES) $(MANIFEST)
	@echo "# mcrez resources"
	mcrez $(DATA) $(RESOURCES) -o $(TMP_DIR) -r mc.resources.c

clean:
	@echo "# clean zephyr"
	-rm -f $(XS_SOURCES) $(TMP_DIR)/mc.xs.c $(TMP_DIR)/mc.resources.c
	-rm -rf $(ZEPHYR_BUILD_DIR)

