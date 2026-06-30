#!/usr/bin/env python3
"""
Scan repository text files for invalid UTF-8 and convert legacy-encoded files to
UTF-8 without BOM.

Default conversion uses gb18030, which covers GB2312/GBK/GB18030. Add extra
--encoding values if your tree contains files from other legacy code pages.

Examples:
  python scripts/fix_invalid_utf8.py --check
  python scripts/fix_invalid_utf8.py
  python scripts/fix_invalid_utf8.py --encoding gb18030 --encoding shift_jis
"""

from __future__ import annotations

import argparse
import codecs
import fnmatch
import os
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable, Sequence


DEFAULT_SOURCE_ENCODINGS = ("gb18030",)

DEFAULT_SKIP_DIR_PATTERNS = {
    ".git",
    ".vs",
    ".idea",
    ".vscode",
    ".cache",
    ".agents",
    ".codex",
    ".codex_tmp*",
    ".pytest_cache",
    "__pycache__",
    "build",
    "build-*",
    "build_*",
    "cmake-build-*",
    "deps",
    "deps_src",
    "out",
    "sandboxes",
}

DEFAULT_BINARY_EXTENSIONS = {
    ".3mf",
    ".7z",
    ".a",
    ".avi",
    ".bin",
    ".bmp",
    ".bz2",
    ".cur",
    ".dat",
    ".dll",
    ".dylib",
    ".exe",
    ".gif",
    ".gz",
    ".icns",
    ".ico",
    ".jar",
    ".jpeg",
    ".jpg",
    ".lib",
    ".mov",
    ".mp3",
    ".mp4",
    ".obj",
    ".o",
    ".otf",
    ".pdf",
    ".pdb",
    ".png",
    ".pyc",
    ".rar",
    ".so",
    ".stl",
    ".tar",
    ".tga",
    ".ttf",
    ".wav",
    ".webp",
    ".woff",
    ".woff2",
    ".xz",
    ".zip",
    ".zst",
}

BOM_ENCODINGS = (
    (codecs.BOM_UTF32_LE, "utf-32-le"),
    (codecs.BOM_UTF32_BE, "utf-32-be"),
    (codecs.BOM_UTF16_LE, "utf-16-le"),
    (codecs.BOM_UTF16_BE, "utf-16-be"),
)


@dataclass
class Stats:
    scanned: int = 0
    valid_utf8: int = 0
    converted: int = 0
    would_convert: int = 0
    unresolved: int = 0
    skipped_binary: int = 0
    skipped_large: int = 0
    read_errors: int = 0


def parse_args() -> argparse.Namespace:
    script_dir = Path(__file__).resolve().parent
    repo_root = script_dir.parent

    parser = argparse.ArgumentParser(
        description="Find files that are not valid UTF-8 and convert legacy text files to UTF-8 without BOM.",
    )
    parser.add_argument(
        "--root",
        type=Path,
        default=repo_root,
        help="Root directory to scan. Defaults to the repository root.",
    )
    parser.add_argument(
        "--check",
        action="store_true",
        help="Report invalid UTF-8 files without modifying them. Exits 1 if convertible files are found.",
    )
    parser.add_argument(
        "--encoding",
        action="append",
        dest="encodings",
        help=(
            "Legacy source encoding to try, in order. Can be passed multiple times. "
            "Defaults to gb18030."
        ),
    )
    parser.add_argument(
        "--include-vendored",
        action="store_true",
        help="Also scan deps/ and deps_src/. They are skipped by default.",
    )
    parser.add_argument(
        "--skip-dir",
        action="append",
        default=[],
        help="Additional directory name or fnmatch pattern to skip. Can be passed multiple times.",
    )
    parser.add_argument(
        "--no-binary-extension-skip",
        action="store_true",
        help="Do not skip files by common binary extensions. NUL-byte binary detection still applies.",
    )
    parser.add_argument(
        "--max-bytes",
        type=int,
        default=50 * 1024 * 1024,
        help="Skip files larger than this many bytes. Use 0 for no size limit. Default: 52428800.",
    )
    parser.add_argument(
        "--keep-mtime",
        action="store_true",
        help="Preserve each converted file's modification timestamp.",
    )
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="Print valid UTF-8 files as they are scanned.",
    )
    return parser.parse_args()


def should_skip_dir(name: str, patterns: Sequence[str]) -> bool:
    return any(fnmatch.fnmatchcase(name, pattern) for pattern in patterns)


def iter_files(root: Path, skip_patterns: Sequence[str]) -> Iterable[Path]:
    for dirpath, dirnames, filenames in os.walk(root):
        dirnames[:] = sorted(
            name for name in dirnames if not should_skip_dir(name, skip_patterns)
        )
        for filename in sorted(filenames):
            yield Path(dirpath) / filename


def is_binary_extension(path: Path) -> bool:
    return path.suffix.lower() in DEFAULT_BINARY_EXTENSIONS


def looks_binary(data: bytes) -> bool:
    if any(data.startswith(bom) for bom, _encoding in BOM_ENCODINGS):
        return False
    if data.startswith(codecs.BOM_UTF8):
        return False
    return b"\0" in data[:4096]


def relative(path: Path, root: Path) -> str:
    try:
        return str(path.relative_to(root))
    except ValueError:
        return str(path)


def decode_legacy(data: bytes, encodings: Sequence[str]) -> tuple[str, str] | None:
    for bom, encoding in BOM_ENCODINGS:
        if data.startswith(bom):
            text = data[len(bom) :].decode(encoding)
            return text, f"{encoding}-bom"

    for encoding in encodings:
        try:
            text = data.decode(encoding)
            if text.encode(encoding) != data:
                continue
            return text, encoding
        except (LookupError, UnicodeError):
            continue

    return None


def process_file(
    path: Path,
    root: Path,
    encodings: Sequence[str],
    args: argparse.Namespace,
    stats: Stats,
) -> None:
    if not args.no_binary_extension_skip and is_binary_extension(path):
        stats.skipped_binary += 1
        return

    try:
        size = path.stat().st_size
        if args.max_bytes and size > args.max_bytes:
            stats.skipped_large += 1
            return

        data = path.read_bytes()
    except OSError as exc:
        stats.read_errors += 1
        print(f"READ-ERROR {relative(path, root)}: {exc}")
        return

    if looks_binary(data):
        stats.skipped_binary += 1
        return

    stats.scanned += 1
    relpath = relative(path, root)

    try:
        data.decode("utf-8")
        stats.valid_utf8 += 1
        if args.verbose:
            print(f"OK {relpath}")
        return
    except UnicodeDecodeError as utf8_error:
        invalid_at = utf8_error.start

    try:
        decoded = decode_legacy(data, encodings)
    except UnicodeError as exc:
        decoded = None
        decode_error = str(exc)
    else:
        decode_error = "no configured source encoding matched"

    if decoded is None:
        stats.unresolved += 1
        print(f"UNRESOLVED {relpath}: invalid UTF-8 at byte {invalid_at}; {decode_error}")
        return

    text, source_encoding = decoded
    if args.check:
        stats.would_convert += 1
        print(f"WOULD-CONVERT {relpath}: {source_encoding} -> utf-8")
        return

    old_mtime_ns = path.stat().st_mtime_ns
    path.write_bytes(text.encode("utf-8"))
    if args.keep_mtime:
        os.utime(path, ns=(old_mtime_ns, old_mtime_ns))

    stats.converted += 1
    print(f"CONVERTED {relpath}: {source_encoding} -> utf-8")


def main() -> int:
    args = parse_args()
    root = args.root.resolve()
    encodings = tuple(args.encodings or DEFAULT_SOURCE_ENCODINGS)
    skip_patterns = set(args.skip_dir)

    vendored_patterns = {"deps", "deps_src"}
    for pattern in DEFAULT_SKIP_DIR_PATTERNS:
        if args.include_vendored and pattern in vendored_patterns:
            continue
        skip_patterns.add(pattern)

    stats = Stats()
    for path in iter_files(root, sorted(skip_patterns)):
        process_file(path, root, encodings, args, stats)

    print(
        "SUMMARY "
        f"scanned={stats.scanned} "
        f"valid_utf8={stats.valid_utf8} "
        f"converted={stats.converted} "
        f"would_convert={stats.would_convert} "
        f"unresolved={stats.unresolved} "
        f"skipped_binary={stats.skipped_binary} "
        f"skipped_large={stats.skipped_large} "
        f"read_errors={stats.read_errors}"
    )

    if stats.unresolved or stats.read_errors:
        return 2
    if args.check and stats.would_convert:
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
