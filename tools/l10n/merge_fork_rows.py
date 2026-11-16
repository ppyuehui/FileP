#!/usr/bin/env python3
"""Re-apply FileP's edits to upstream catalog rows after merging a new release.

The catalog is merged by hand as "official file + our appended rows". That is
fine for rows we added, but it silently drops FileP's edits to rows that already
exist upstream (the About title, "关于 FileP", was the first casualty; only the
feature guard noticed).

This helper rebuilds those edits from git history:

    base      the FileP commit the merge started from (merge base / first parent)
    upstream  the official release that was merged
    catalog   the working tree's catalog.tsv

Only the text columns (zh / en / tw) are ever rewritten, never ids, id names or
row order: our added rows get renumbered on purpose, and the catalog files are
generated from by tools/l10n/gen_strings.py afterwards.

Exit code 1 means something needs a human: a row we edited is gone, or a row we
added disappeared during the merge.
"""
from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path

TEXT_FROM = 3  # columns: name, id_name, id, zh, en, tw


def git_show(repo: Path, ref: str, path: str) -> str:
    out = subprocess.run(["git", "show", f"{ref}:{path}"], cwd=repo, capture_output=True)
    if out.returncode != 0:
        sys.exit(f"cannot read {path} at {ref}: {out.stderr.decode('utf-8', 'replace').strip()}")
    return out.stdout.decode("utf-8")


def parse(text: str) -> dict[str, list[str]]:
    rows: dict[str, list[str]] = {}
    for line in text.splitlines():
        stripped = line.strip()
        if not stripped or stripped.startswith("#") or stripped.startswith("//"):
            continue
        parts = line.split("\t")
        if len(parts) not in (5, 6):
            continue
        while len(parts) < 6:
            parts.append("")
        rows[parts[0]] = parts
    return rows


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--repo", default=".")
    ap.add_argument("--base", required=True, help="FileP commit the merge started from")
    ap.add_argument("--upstream", required=True, help="official release merged in")
    ap.add_argument("--catalog", default="tools/l10n/catalog.tsv")
    ap.add_argument("--dry-run", action="store_true")
    args = ap.parse_args()

    repo = Path(args.repo).resolve()
    path = args.catalog
    base = parse(git_show(repo, args.base, path))
    upstream = parse(git_show(repo, args.upstream, path))

    live_text = (repo / path).read_text(encoding="utf-8")
    lines = live_text.splitlines()
    index: dict[str, int] = {}
    for i, line in enumerate(lines):
        stripped = line.strip()
        if not stripped or stripped.startswith("#") or stripped.startswith("//"):
            continue
        index[line.split("\t")[0]] = i

    ours_added = [n for n in base if n not in upstream]
    ours_edited = [n for n in base if n in upstream and base[n][TEXT_FROM:] != upstream[n][TEXT_FROM:]]

    reapplied: list[str] = []
    problems: list[str] = []
    for name in ours_edited:
        want = base[name]
        if name not in index:
            problems.append(f"{name}: 我们改过这一行，但合并结果里没有它了（官方删了？）")
            continue
        parts = lines[index[name]].split("\t")
        while len(parts) < 6:
            parts.append("")
        if parts[TEXT_FROM:] != want[TEXT_FROM:]:
            parts[TEXT_FROM:] = want[TEXT_FROM:]
            lines[index[name]] = "\t".join(parts)
            reapplied.append(name)
    for name in ours_added:
        if name not in index:
            problems.append(f"{name}: 我们新增的字符串在合并结果里不见了")

    new_text = "\n".join(lines) + "\n"
    changed = new_text != live_text
    if changed and not args.dry_run:
        raw = (repo / path).read_bytes()
        bom = raw.startswith(b"\xef\xbb\xbf")
        (repo / path).write_bytes((b"\xef\xbb\xbf" if bom else b"") + new_text.encode("utf-8"))

    print(f"catalog: base={args.base} upstream={args.upstream}")
    print(f"  rows we added: {len(ours_added)}   our edits to official rows: {len(ours_edited)}")
    if reapplied:
        print(f"  re-applied ({len(reapplied)}): {', '.join(reapplied[:8])}" + (" ..." if len(reapplied) > 8 else ""))
    if not changed:
        print("  nothing to fix")
    elif args.dry_run:
        print("  would fix the rows above (dry run)")
    for problem in problems:
        print(f"  PROBLEM: {problem}")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
