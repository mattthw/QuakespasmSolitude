#!/usr/bin/env python3
"""dupe_audit.py — byte-level containment audit between file trees.

Answers: "is tree A redundant with tree B?" before anything is deleted.
This is the machine-checked proof class used to delete
legacy/solitude-production (contained in solitude-vita-backup, commit
1aaf5bc2). Grouping is size-then-md5: same size bucket, md5 decides.

A "pseudo-tree" can also come from .pak files (Quake PACK format), so a
runtime dir can be checked against loose files + shipped paks at once.

Usage:
  python3 dupe_audit.py A_DIR B_DIR [B_DIR2 ...] [--pak FILE ...] [--json OUT]

Exit code 0 if everything in A is contained in the B-set (A is safe to
delete), 1 otherwise. Uniques are reported per kind (missing / differs).
"""
import hashlib
import json
import os
import struct
import sys


def md5_file(path, block=1 << 20):
    h = hashlib.md5()
    with open(path, "rb") as f:
        while True:
            b = f.read(block)
            if not b:
                break
            h.update(b)
    return h.hexdigest()


def walk_tree(root):
    """Return {relpath: (abspath, size)} for every regular file under root."""
    out = {}
    for dirpath, _dirs, files in os.walk(root):
        for fn in files:
            p = os.path.join(dirpath, fn)
            if os.path.islink(p):
                continue
            try:
                out[os.path.relpath(p, root)] = (p, os.path.getsize(p))
            except OSError:
                pass
    return out


def pak_entries(pak_path):
    """{name: size} for a Quake PACK .pak. Entries store backslash paths.
    Header: magic(4) dict(4) dirOfs(4) dirLen(4) — dirLen/64 = count."""
    out = {}
    with open(pak_path, "rb") as f:
        data = f.read()
    if data[:4] != b"PACK":
        return out
    dir_ofs, dir_len = struct.unpack("<ii", data[8:16])
    for i in range(dir_len // 64):
        rec = data[dir_ofs + i * 64: dir_ofs + i * 64 + 64]
        name = rec[:56].split(b"\0")[0].decode("ascii", "replace")
        _ofs, size = struct.unpack("<ii", rec[56:64])
        out[name.replace("\\", "/").lower()] = size
    return out


def index_by_md5(files):
    """files: {label: (path, size)} -> {md5: [labels]} (size buckets first)."""
    by_size = {}
    for label, (_p, size) in files.items():
        by_size.setdefault(size, []).append(label)
    digests = {}
    for size, labels in by_size.items():
        buckets = {}
        for label in labels:
            if len(labels) == 1:
                buckets.setdefault(md5_file(files[label][0]), []).append(label)
            else:
                buckets.setdefault(md5_file(files[label][0]), []).append(label)
        for digest, group in buckets.items():
            digests[digest] = group
    return digests


def main():
    args = sys.argv[1:]
    if len(args) < 2:
        print(__doc__)
        sys.exit(2)
    a_dir = args[0]
    b_dirs = []
    paks = []
    json_out = None
    i = 1
    while i < len(args):
        if args[i] == "--pak":
            i += 1
            while i < len(args) and not args[i].startswith("--"):
                paks.append(args[i])
                i += 1
            continue
        if args[i] == "--json":
            json_out = args[i + 1]
            i += 2
            continue
        b_dirs.append(args[i])
        i += 1

    a_files = walk_tree(a_dir)
    b_files = {}
    for d in b_dirs:
        for rel, (p, size) in walk_tree(d).items():
            b_files[rel] = (p, size)
    # pak members act as B entries: only name+size known, so hash-compare
    # against pak content ranges lazily below
    pak_ofs = {}
    pak_data = {}
    for pk in paks:
        for name, size in pak_entries(pk).items():
            b_files.setdefault("pak:" + name, (None, size))
        with open(pk, "rb") as f:
            pak_data[pk] = f.read()
        dir_ofs, dir_len = struct.unpack("<ii", pak_data[pk][8:16])
        for k in range(dir_len // 64):
            rec = pak_data[pk][dir_ofs + k * 64: dir_ofs + k * 64 + 64]
            name = rec[:56].split(b"\0")[0].decode("ascii", "replace")
            ofs, size = struct.unpack("<ii", rec[56:64])
            pak_ofs["pak:" + name.replace("\\", "/").lower()] = (pk, ofs, size)

    b_md5 = {}
    for label, (p, size) in b_files.items():
        if p is None:
            pk, ofs, size2 = pak_ofs[label]
            digest = hashlib.md5(pak_data[pk][ofs: ofs + size2]).hexdigest()
        else:
            digest = md5_file(p)
        b_md5.setdefault(digest, []).append(label)

    contained, uniques = [], []
    for rel, (p, size) in sorted(a_files.items()):
        digest = md5_file(p)
        if digest in b_md5:
            contained.append({"file": rel, "matches": b_md5[digest]})
        else:
            same_size = [lbl for lbl, (_pp, ss) in b_files.items() if ss == size]
            uniques.append({
                "file": rel,
                "size": size,
                "kind": "differs" if same_size else "missing",
                "same_size_in_B": same_size[:5],
            })

    report = {
        "a": a_dir,
        "b_dirs": b_dirs,
        "paks": paks,
        "a_total": len(a_files),
        "a_bytes": sum(s for _, s in a_files.values()),
        "contained": len(contained),
        "unique_count": len(uniques),
        "unique_bytes": sum(u["size"] for u in uniques),
        "uniques": uniques,
    }
    if json_out:
        with open(json_out, "w") as f:
            json.dump(report, f, indent=1)
    print(f"A={a_dir}  files={report['a_total']}  ({report['a_bytes']/1e6:.1f} MB)")
    print(f"contained in B: {report['contained']}")
    print(f"unique to A:    {report['unique_count']}  ({report['unique_bytes']/1e6:.1f} MB)")
    for u in uniques[:40]:
        print(f"  [{u['kind']}] {u['file']}")
    if len(uniques) > 40:
        print(f"  ... {len(uniques)-40} more (see --json)")
    sys.exit(0 if not uniques else 1)


if __name__ == "__main__":
    main()
