#!/usr/bin/env python3
"""Generate the xs_sources.cmake fragment for Zephyr builds.

The script collects the manifest-generated source files for the active
build, merges them with the XS runtime and platform sources, and emits
CMake variables describing the complete list of sources, include
directories, and compile definitions.
"""

from __future__ import annotations

import argparse
from pathlib import Path
from typing import Iterable, List


def _append_unique(sequence: List[str], values: Iterable[str]) -> None:
    for value in values:
        if not value:
            continue
        if value in sequence:
            continue
        sequence.append(value)


def _gather_generated_sources(makefile: Path, tmp_dir: Path) -> List[str]:
    generated: List[str] = []
    try:
        content = makefile.read_text(encoding="utf-8")
    except FileNotFoundError:
        return generated

    prefixes = (f"$(TMP_DIR)/", f"{tmp_dir}/")
    for raw_line in content.splitlines():
        line = raw_line.strip()
        if not line or line.startswith("#") or ":" not in line:
            continue
        target, rest = line.split(":", 1)
        target = target.strip()
        if not target.startswith(prefixes):
            continue
        prereq = rest.split("|", 1)[0].strip()
        if not prereq:
            continue
        candidate = prereq.split()[0]
        if candidate.startswith("$("):
            continue
        _append_unique(generated, [candidate])
    return generated


def _write_cmake_list(lines: List[str], name: str, values: Iterable[str]) -> None:
    lines.append(f"set({name}")
    for value in values:
        lines.append(f"    {value}")
    lines.append(")")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--tmp-dir", required=True, type=Path)
    parser.add_argument("--makefile", required=True, type=Path)
    parser.add_argument("--runtime", action="append", default=[])
    parser.add_argument("--platform", action="append", default=[])
    parser.add_argument("--mc-source", action="append", default=[])
    parser.add_argument("--include", action="append", default=[])
    parser.add_argument("--define", dest="defines", action="append", default=[])

    args = parser.parse_args()

    output_lines: List[str] = []

    runtime_sources = [str(Path(path)) for path in args.runtime]
    platform_sources = [str(Path(path)) for path in args.platform]
    mc_sources = [str(Path(path)) for path in args.mc_source]

    generated = _gather_generated_sources(args.makefile, args.tmp_dir)
    _append_unique(mc_sources, generated)

    _write_cmake_list(output_lines, "MODDABLE_MC_XS", [str(args.tmp_dir / "mc.xs.c")])
    _write_cmake_list(output_lines, "MODDABLE_MC_RESOURCES", [str(args.tmp_dir / "mc.resources.c")])
    _write_cmake_list(output_lines, "XS_RUNTIME_SOURCES", runtime_sources)
    _write_cmake_list(output_lines, "XS_PLATFORM_SOURCES", platform_sources)
    _write_cmake_list(output_lines, "MC_SOURCES", mc_sources)

    include_dirs = [str(Path(path)) for path in args.include]
    _write_cmake_list(output_lines, "MC_INCLUDE_DIRS", include_dirs)

    _write_cmake_list(output_lines, "MC_COMPILE_DEFINITIONS", args.defines)

    args.output.write_text("\n".join(output_lines) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
