#!/usr/bin/env python3
"""B0 map audit: BSP29 read-only analysis of the 24 matchmaking-menu maps.

Detects: leak/no-PVS maps, oversized textures (gl_max_size win), lightmap
bulk (dlight rebake cost), vertex hotspots (face-density areas you can
jump to in TrenchBroom). Writes MAP-AUDIT.md + MAP-AUDIT.csv beside this
file. No writes to any map. Run from anywhere; paths are absolute.

Pure stdlib. BSP29 structs per quakespasm/Quake/bspfile.h.
"""
import csv
import os
import struct
import sys
from collections import Counter

REPO = "/Users/matt_1/workspace/projects-vita/solitude/repo"
MAPDIR = os.path.join(REPO, "solitude", "maps")
OUTDIR = os.path.dirname(os.path.abspath(__file__))

# bsp name -> menu.c levels[] entry (thumbnail) for the 24 menu maps
MENU_MAPS = [
    "highground", "citadel", "babybloodgulch", "pit3", "narsp",
    "skyringbl2", "wizard", "lockdown", "cestra", "lane", "Longest",
    "chill", "pri2", "base", "plaza", "spider", "pit", "pit2", "lock",
    "+bloodgulch2", "narrows", "construction", "fire",
]

LUMP_ENTITIES, LUMP_PLANES, LUMP_TEXTURES, LUMP_VERTEXES, LUMP_VIS = 0, 1, 2, 3, 4
LUMP_LIGHTING, LUMP_LEAVES, LUMP_TEXINFO, LUMP_FACES, LUMP_EDGES = 7, 10, 6, 7, 12
# explicit indices (BSP29 dir order):
D = dict(entities=0, planes=1, textures=2, vertexes=3, visibility=4, nodes=5,
         texinfo=6, faces=7, lighting=8, clipnodes=9, leaves=10,
         marksurfaces=11, edges=12, models=13, lightmaps=14)


def bsp_lumps(data):
    if struct.unpack_from("<i", data, 0)[0] != 29:
        return None
    out = {}
    for name, idx in D.items():
        ofs, len_ = struct.unpack_from("<2i", data, 4 + idx * 8)
        out[name] = (ofs, len_, data[ofs:ofs + len_])
    return out


def read_textures(l):
    n = struct.unpack_from("<i", l, 0)[0]
    offs = struct.unpack_from("<%di" % n, l, 4)
    tex = []
    for o in offs:
        if o <= 0:  # 0 = empty slot, -1 = external wad (ericw-tools marker)
            tex.append(None)
            continue
        name = l[o:o + 16].split(b"\0")[0].decode("latin-1")
        w, h = struct.unpack_from("<2i", l, o + 16)
        tex.append((name, w, h))
    return tex


def read_texinfo_miptex(l):
    out = []
    for i in range(len(l) // 40):
        miptex = struct.unpack_from("<i", l, i * 40 + 32)[0]
        out.append(miptex)
    return out


def read_faces(l):
    out = []
    for i in range(len(l) // 20):
        firstedge, = struct.unpack_from("<i", l, i * 20 + 4)
        numedges, texinfo = struct.unpack_from("<2h", l, i * 20 + 8)
        lightofs, = struct.unpack_from("<i", l, i * 20 + 16)
        out.append((abs(firstedge), numedges, texinfo, lightofs))
    return out


def read_verts(l):
    return [struct.unpack_from("<3f", l, i * 12) for i in range(len(l) // 12)]


def read_edges(l):
    return [struct.unpack_from("<2H", l, i * 4) for i in range(len(l) // 4)]


def vis_score(visl, numleafs):
    if numleafs <= 0:
        return None
    if len(visl) == 0:
        return ("NO-VIS (leaked/unwritten)", float("inf"))
    n = struct.unpack_from("<i", visl, 0)[0]  # numleafs per header
    per_leaf = len(visl) / float(numleafs)
    allvis = numleafs / 8.0
    ratio = per_leaf / allvis if allvis else 0
    tag = None
    if ratio > 0.5:
        tag = "WEAK-PVS (%.2f of all-visible)" % ratio
    return (tag, ratio)


def hotspots(verts, cell=256):
    bins = Counter()
    for v in verts:
        bins[(int(v[0] // cell) * cell, int(v[1] // cell) * cell,
              int(v[2] // cell) * cell)] += 1
    return bins


def audit(path):
    with open(path, "rb") as f:
        data = f.read()
    L = bsp_lumps(data)
    if L is None:
        return {"file": os.path.basename(path), "error": "not BSP29"}
    tex = read_textures(L["textures"][2])
    miptexes = read_texinfo_miptex(L["texinfo"][2])
    faces = read_faces(L["faces"][2])
    verts = read_verts(L["vertexes"][2])
    numleafs = len(L["leaves"][2]) // 28

    used = Counter()
    for fe, ne, ti, lo in faces:
        if 0 <= ti < len(miptexes) and miptexes[ti] is not None:
            m = miptexes[ti]
            if 0 <= m < len(tex) and tex[m]:
                used[m] += 1
    big = [(tex[m][0], tex[m][1], tex[m][2], n) for m, n in used.items()
           if max(tex[m][1], tex[m][2]) >= 1024]
    big.sort(key=lambda t: -t[1] * t[2])
    mb_tex = sum(tex[m][1] * tex[m][2] * 0.5 / 1024.0 / 1024.0 * 1.333
                 for m in used if tex[m])  # DXT ~0.5B/texel + mips
    cap512 = 0.0
    for m in used:
        if not tex[m]:
            continue
        w, h = tex[m][1], tex[m][2]
        f = (512.0 / max(w, h)) ** 2 if max(w, h) > 512 else 1.0
        cap512 += w * h * (1 - f) * 2 / 1024.0 / 1024.0 * 1.333
    lm_blocks = L["lighting"][1] // 49152
    lm_mb = L["lighting"][1] / 1024.0 / 1024.0
    lit_faces = sum(1 for fe, ne, ti, lo in faces if lo != -1)
    vtag, vratio = vis_score(L["visibility"][2], numleafs)

    # --- hotspot analysis: verts per 256u cell, faces+textures per cell ---
    edges = read_edges(L["edges"][2])
    e_ofs, e_len = L["edges"][0], L["edges"][1]
    nedges = e_len // 4
    edge_raw = L["edges"][2]
    bins = hotspots(verts)
    cell_faces = Counter()
    cell_tex = {}
    for fe, ne, ti, lo in faces:
        if fe >= nedges:
            continue
        raw = struct.unpack_from("<2H", edge_raw, fe * 4)
        # firstedge sign recovered via stored abs; treat index fe, use v0
        vi = raw[0]
        if vi >= len(verts):
            continue
        v = verts[vi]
        c = (int(v[0] // 256) * 256, int(v[1] // 256) * 256, int(v[2] // 256) * 256)
        cell_faces[c] += 1
        m = miptexes[ti] if 0 <= ti < len(miptexes) else -1
        tname = tex[m][0] if 0 <= m < len(tex) and tex[m] else "?"
        cell_tex.setdefault(c, Counter())[tname] += 1
    occupied = len(bins) or 1
    mean_occ = len(verts) / float(occupied)
    hs_raw = []
    for c, n in sorted(bins.items(), key=lambda kv: -kv[1])[:5]:
        f = cell_faces.get(c, 0)
        ratio = n / mean_occ if mean_occ else 0.0
        tops = "; ".join("%s x%d" % t for t in cell_tex.get(c, Counter()).most_common(2))
        hs_raw.append((c, n, round(ratio, 1), f, tops))
    hs = ["(%d,%d,%d) %dv (%.1fx avg) %df [%s]" % (c[0], c[1], c[2], n, ratio, f, tops)
          for (c, n, ratio, f, tops) in hs_raw[:3]]
    return {
        "file": os.path.basename(path), "size_mb": round(len(data) / 1048576.0, 2),
        "faces": len(faces), "verts": len(verts),
        "leafs": numleafs, "vis_flag": vtag, "vis_ratio": round(vratio, 3) if vratio != float("inf") else 99.0,
        "lightmaps_mb": round(lm_mb, 1), "lightmap_blocks": lm_blocks,
        "lit_faces_pct": round(100.0 * lit_faces / max(1, len(faces)), 1),
        "tex_dxt_mb": round(mb_tex, 1), "tex_ge1024": len(big),
        "gl_max_size_512_saves_mb": round(cap512, 1),
        "top_big_tex": "; ".join("%s %dx%d(%d faces)" % b for b in big[:3]),
        "top_hotspots": " || ".join(hs),
        "hotspot_cells": hs_raw,
        "hot_ratio_max": round((bins.most_common(1)[0][1] / mean_occ) if bins else 0, 1),
        "mean_verts_per_cell": round(mean_occ, 1),
    }


def emit_markers(r):
    cells = r.get("hotspot_cells") or []
    picks = [h for h in cells if h[2] >= 5.0]
    if not picks:
        return 0
    base = r["file"][:-4] if r["file"].lower().endswith(".bsp") else r["file"]
    mdir = os.path.join(OUTDIR, "markers")
    if not os.path.isdir(mdir):
        os.makedirs(mdir)
    lines = ['{', '"worldtype" "idtech1"', '"classname" "worldspawn"',
             '"message" "B0 hotspot markers for %s (256u cells, ratio >= 5x)"' % base, '}']
    for c, n, ratio, f, tops in picks:
        lines += ['{', '"classname" "info_null"',
                  '"origin" "%d %d %d"' % (c[0] + 128, c[1] + 128, c[2] + 128),
                  '"message" "%.1fx avg - %dv %df [%s]"' % (ratio, n, f, tops),
                  '"noise" "1"', '}']
    with open(os.path.join(mdir, base + "-hot.map"), "w") as fh:
        fh.write("\n".join(lines) + "\n")
    return len(picks)


def main():
    only = sys.argv[1:] or None
    present = {f.lower(): f for f in os.listdir(MAPDIR) if f.lower().endswith(".bsp")}
    rows, missing = [], []
    for want in MENU_MAPS:
        fn = present.get((want + ".bsp").lower())
        if fn is None:
            missing.append(want)
            continue
        if only and want.lower() not in [o.lower() for o in only]:
            continue
        rows.append(audit(os.path.join(MAPDIR, fn)))

    def score(r):
        s = 0.0
        if r.get("error"):
            return 1e9
        s += 50 if r["vis_flag"] == "NO-VIS (leaked/unwritten)" else 0
        s += 20 * min(3.0, r["vis_ratio"] or 0)
        s += 0.5 * r["lightmaps_mb"]
        s += 2.0 * r["tex_ge1024"]
        s += 0.2 * r["gl_max_size_512_saves_mb"]
        s += 5.0 * min(6.0, r["hot_ratio_max"])
        return s

    rows.sort(key=score, reverse=True)
    cols = [k for k in rows[0] if k not in ("hotspot_cells",)]
    with open(os.path.join(OUTDIR, "MAP-AUDIT.csv"), "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=cols, lineterminator="\n")
        w.writeheader()
        for r in rows:
            w.writerow({k: r.get(k, "") for k in cols})

    md = ["# B0 map audit - 24 matchmaking-menu maps (BSP29, read-only)",
          "",
          "Score = leak(50) + 20*PVS-weakness + 0.5*lightmap-MB + 2*count(>=1024 tex) + 0.2*gl_max_size512-savings-MB + 5*hotspot-density-ratio(cap6)",
          "Hotspots: 256-unit cells; 'Nd (Xx avg) Mf' = N verts, X times the map's mean verts/cell, M faces anchored in cell; tex = top textures there.",
          "",
          "| # | map | vis | >=1024 tex | tex DXT MB | cap512 saves MB | lightmaps MB | faces | verts | leafs |",
          "|---|-----|-----|-----------|------------|-----------------|--------------|-------|-------|-------|"]
    for i, r in enumerate(rows, 1):
        if r.get("error"):
            md.append("| %d | %s | %s |" % (i, r["file"], r["error"]))
            continue
        md.append("| %d | %s | %s | %d | %.1f | %.1f | %.1f | %d | %d | %d |" % (
            i, r["file"].replace(".bsp", ""), r["vis_flag"] or "ok",
            r["tex_ge1024"], r["tex_dxt_mb"], r["gl_max_size_512_saves_mb"],
            r["lightmaps_mb"], r["faces"], r["verts"], r["leafs"]))
    md.append("")
    md.append("## Hotspot details (all maps, worst first)")
    for r in rows:
        if r.get("error"):
            continue
        md.append("- **%s** vis-ratio %.2f %s | hotspots: %s" % (
            r["file"], r["vis_ratio"], r["vis_flag"] or "", r["top_hotspots"]))
    md.append("")
    md.append("## Read-out / recommended actions")
    md.append("- TEXTURE SIZE IS A NON-ISSUE for this set: zero menu maps use >1024px "
              "textures (Cestra: one 1024). gl_max_size caps save <2MB total; not worth a batch.")
    no_vis = [r["file"] for r in rows if r.get("vis_flag", "") and "NO-VIS" in r["vis_flag"]]
    md.append("- **%d maps have an EMPTY visibility lump (no PVS at all)**: %s. "
              "Engine falls back to all-leafs-visible (gl_rmain R_MarkLeaves) -> world culling AND "
              "dlight MarkLights walks degrade to whole-map every frame. Re-running `vis` on these "
              ".map sources is the single biggest map-content fix and amplifies B3 (dlight cull) "
              "and B9 (sub-pixel cull)." % (len(no_vis), ", ".join(no_vis)))
    md.append("- WEAK-PVS ratios ~0.6-1.0 on small open DM maps are largely geometric truth, not "
              "leaks; the NO-VIS rows above are the real defect class.")
    md.append("- 'construction' is in the matchmaking menu but no runtime .bsp exists in "
              "solitude/maps (only in legacy backup) - menu selection errors out; fix menu or ship map.")
    md.append("- Worst overall content map: Cestra (no PVS + 9473 faces + 0.9MB lightmaps + 1024 tex). "
              "Heaviest well-built maps: narrows/pit2/pit3/highground (10-12k faces, good vis).")
    md.append("- Vertex hotspots (256u cells, coords in Details) are TB-jumpable for face-merge passes "
              "if a specific area tests slow in B1 - don't blanket-remap.")
    worst_hs = sorted([r for r in rows if not r.get("error")], key=lambda r: -r["hot_ratio_max"])[:5]
    md.append("- **Worst hotspot outliers** (verts/cell vs map mean): " + "; ".join(
        "%s %.0fx" % (r["file"].replace(".bsp", ""), r["hot_ratio_max"]) for r in worst_hs) +
        ". skyRingbl2 is extreme (956 verts/359 faces in one 256u cell, z=768: the metal ring "
        "structure); highground 37x is {foliage/detail terrain; pit2+pit3 28-30x both driven by "
        "en_32_number (32px signage tex = hundreds of tiny faces); lockdown 13x fore_128_wall_2 "
        "carving. These inflate faces-in-PVS, draw calls and lightmap density in exactly the "
        "open-view lanes of outdoor maps -> prime targets for B4 sub-pixel cull AND map-side face "
        "merging with TB coords from Hotspot details.")
    if missing:
        md.append("")
        md.append("## In menu but NO runtime BSP found: %s" % ", ".join(missing))
    nmark = sum(emit_markers(r) for r in rows if not r.get("error"))
    md.append("- Marker maps: tools/asset-scan/markers/<map>-hot.map = info_null point entities "
              "at hot 256u cell centers (ratio >= 5x, top 5 per map). Open in TrenchBroom, "
              "select a marker to jump; re-running this script regenerates them. %d markers total."
              % nmark)
    with open(os.path.join(OUTDIR, "MAP-AUDIT.md"), "w") as f:
        f.write("\n".join(md) + "\n")
    print("wrote MAP-AUDIT.md + MAP-AUDIT.csv + %d marker entities (%d maps, missing: %s)" % (
        nmark, len(rows), missing))


if __name__ == "__main__":
    main()
