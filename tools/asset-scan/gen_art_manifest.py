#!/usr/bin/env python3
"""gen_art_manifest.py — produce ART_SHA256.txt (decision #1: art bytes stay
local, git records WHICH version shipped). Walks the local-only art trees
outside the repo and writes `path|size|sha256` lines at the repo root.
Run:  python3 tools/asset-scan/gen_art_manifest.py
Paths are relative to the workspace root (projects-vita/), e.g.
  solitude/local/vendored/wads/highground.wad|4072908|<sha>
"""
import hashlib
import os

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, '..', '..'))
WS = os.path.abspath(os.path.join(REPO, '..', '..'))  # projects-vita/
DIRS = ['solitude/local/vendored/wads', 'solitude/local/art']
SKIP_NAMES = {'.DS_Store'}


def sha256(path):
    h = hashlib.sha256()
    with open(path, 'rb') as f:
        for chunk in iter(lambda: f.read(1 << 20), b''):
            h.update(chunk)
    return h.hexdigest()


def main():
    lines = ['# ART_SHA256.txt — local art fingerprint (path|size|sha256), '
             'workspace-root-relative. Regenerate: python3 tools/asset-scan/gen_art_manifest.py']
    n = 0
    total = 0
    for d in DIRS:
        root = os.path.join(WS, d)
        if not os.path.isdir(root):
            lines.append('# MISSING DIR: %s' % d)
            continue
        for dirpath, _dirs, files in sorted(os.walk(root)):
            for f in sorted(files):
                if f in SKIP_NAMES:
                    continue
                full = os.path.join(dirpath, f)
                rel = os.path.relpath(full, WS)
                size = os.path.getsize(full)
                lines.append('%s|%d|%s' % (rel, size, sha256(full)))
                n += 1
                total += size
    out = os.path.join(REPO, 'ART_SHA256.txt')
    open(out, 'w').write('\n'.join(lines) + '\n')
    print('ART_SHA256.txt: %d files, %.2f GB -> %s' % (n, total / 2**30, out))


if __name__ == '__main__':
    main()
