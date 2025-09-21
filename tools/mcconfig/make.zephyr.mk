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

.PHONY: all clean

XS_SOURCES = $(TMP_DIR)/xs_sources.cmake

all: $(XS_SOURCES)

$(XS_SOURCES): $(TMP_DIR)/mc.xs.c $(TMP_DIR)/mc.resources.c
	@echo "# generate xs_sources.cmake"
	@mkdir -p $(dir $@)
	@echo "set(MODDABLE_MC_XS $(TMP_DIR)/mc.xs.c)" > $@
	@echo "set(MODDABLE_MC_RESOURCES $(TMP_DIR)/mc.resources.c)" >> $@

$(TMP_DIR)/mc.xs.c: $(MODULES) $(MANIFEST)
	@echo "# xsl modules"
	xsl -b $(MODULES_DIR) -o $(TMP_DIR) $(PRELOADS) $(STRIPS) $(CREATION) $(MODULES)

$(TMP_DIR)/mc.resources.c: $(DATA) $(RESOURCES) $(MANIFEST)
	@echo "# mcrez resources"
	mcrez $(DATA) $(RESOURCES) -o $(TMP_DIR) -r mc.resources.c

clean:
	@echo "# clean zephyr"
	-rm -f $(XS_SOURCES) $(TMP_DIR)/mc.xs.c $(TMP_DIR)/mc.resources.c

