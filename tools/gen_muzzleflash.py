#!/usr/bin/env python3
# gen_muzzleflash.py -- procedural muzzle-flash sprites for Solitude.
# Stdlib-only. Packs animated IDSP (one sprite GROUP frame; the engine
# animates groups at RENDER framerate from cl.time, r_sprite.c:52-71)
# and writes a PNG contact sheet for review. No PIL/numpy.
#
# Color input = the weapon's extracted ramp (RAMP below; provenance:
# pixel histograms of the shipped solitude/sprites/muzzleflash*.spr
# against solitude/gfx/palette.lmp). Quantization is RESTRICTED to that
# ramp so dithering stays inside the known-good palette subset;
# transparency is index-255 (binary, alpha-tested at 0.666), softened by
# an ordered-dither band -- same convention as the original art.
#
# Shapes:
#   radial -- lumpy flame star (upgrade path of the classic look)
#   claw   -- asymmetric multi-tongue burst: white core + long wispy
#             flame tongues of unequal length (Halo-3-AR style)
#
# Examples:
#   python3 tools/gen_muzzleflash.py --shape claw --ramp gunpowder \
#       --size 64 --frames 6 --seed 7 --synctype rand \
#       --old solitude/sprites/muzzleflash1.spr \
#       --out build/sprites/muzzle_ar.spr --preview build/sprites/muzzle_ar.png

import argparse, math, os, random, struct, sys, zlib

# ---------------------------------------------------------------- palette
def load_palette(path):
    raw = open(path, "rb").read()
    if len(raw) != 768:
        sys.exit("palette must be raw 768 bytes: %s (%d)" % (path, len(raw)))
    return [tuple(raw[i * 3 + j] for j in range(3)) for i in range(256)]

def lum(c):
    return 0.2126 * c[0] + 0.7152 * c[1] + 0.0722 * c[2]

def ramp_rgb(v, ramp):
    """Intensity [0,1+] -> interpolated rgb triple from the sorted ramp
    palette list. Linear-in-ramp is the spr32 colour path (no dither)."""
    q = min(max(v, 0.0), 1.0) * (len(ramp) - 1)
    i0 = int(q)
    if i0 >= len(ramp) - 1:
        return ramp[-1]
    t = q - i0
    return tuple(ramp[i0][k] + (ramp[i0 + 1][k] - ramp[i0][k]) * t
                 for k in range(3))

# Extracted from the shipped sprites (2026-11-14 pixel analysis).
# Quake palette indices; sorted by luminance at startup.
RAMP = {
    "gunpowder": [122, 240, 123, 124, 125, 236, 126, 237, 127, 238, 160,
                  239, 14, 15, 252, 253, 254],
    "plasma":    [32, 33, 222, 221, 220, 219, 218, 217, 216, 215, 213, 214,
                  42, 43, 44, 45, 46, 47, 244, 13, 14, 15, 254],
    "needler":   [144, 162, 161, 160, 14, 15, 254],
}

BAYER8 = [
     0, 32,  8, 40,  2, 34, 10, 42,
    48, 16, 56, 24, 50, 18, 58, 26,
    12, 44,  4, 36, 14, 46,  6, 38,
    60, 28, 52, 20, 62, 30, 54, 22,
     3, 35, 11, 43,  1, 33,  9, 41,
    51, 19, 59, 27, 49, 17, 57, 25,
    15, 47,  7, 39, 13, 45,  5, 37,
    61, 29, 53, 21, 63, 31, 55, 23,
]

# ------------------------------------------------------------- angle noise
class Noise:
    """Smooth periodic 1-D noise for flame edges / wobble (deterministic)."""
    def __init__(self, rng, k=4):
        terms = [(rng.uniform(0.5, 1.5) / (i + 1.5),
                  i + 2 + rng.randint(0, 1),
                  rng.uniform(0.0, 2 * math.pi)) for i in range(k)]
        norm = sum(a for a, _, _ in terms)
        self.terms = [(a / norm, w, p) for a, w, p in terms]

    def ang(self, th):
        return sum(a * math.sin(w * th + p) for a, w, p in self.terms)

def wrapang(a):
    while a > math.pi:  a -= 2 * math.pi
    while a < -math.pi: a += 2 * math.pi
    return a

# ------------------------------------------------------------------ shapes
def make_tongues(rng, n_lo=3, n_hi=4):
    """Claw-burst tongues: [base_angle, length, half_width, phase]."""
    n = rng.randint(n_lo, n_hi)
    tongues = []
    for _ in range(n):
        a = rng.uniform(-math.pi, math.pi)
        L = rng.uniform(0.30, 0.50)
        w = rng.uniform(0.13, 0.26)
        tongues.append([a, L, w, rng.uniform(0.0, 2 * math.pi)])
    rng.choice(tongues)[1] = rng.uniform(0.52, 0.60)   # dominant streamer
    return tongues

def v_radial(r, th, fr, nf, nz):
    t = fr / (nf - 1) if nf > 1 else 0.0
    R = 0.55 * (1.0 + 0.26 * nz.ang(th + 0.7 * fr * 1.3)) * (1.0 - 0.32 * t)
    v = math.exp(-((r / max(R, 0.05)) ** 3))
    v = max(v, math.exp(-((r / (0.16 * (1.0 - 0.35 * t))) ** 2)))
    return v

def v_claw(r, th, fr, nf, nz, tongues):
    t = fr / (nf - 1) if nf > 1 else 0.0
    base = 0.13 * (1.0 - 0.35 * t)
    v = math.exp(-((r / base) ** 2))
    for (a0, ln, w0, ph) in tongues:
        L = ln * (1.0 - 0.35 * t) * (1.0 + 0.08 * nz.ang(ph + fr * 1.7))
        L = max(L, 0.05)
        tt = (r - base) / L
        if tt > 1.0 or tt < -0.30:
            continue
        tc = tt if tt >= 0.0 else 0.0
        a = a0 + 0.06 * nz.ang(ph + fr * 0.8)
        d = abs(wrapang(th - a))
        w = w0 * (1.0 - 0.72 * tc) + 0.02
        radf = (1.0 - tc) ** 2.0 if tt >= 0.0 else 1.0
        angf = math.exp(-((d / w) ** 2))
        turb = 0.85 + 0.15 * nz.ang(d * 24.0 + fr * 2.1 + ph)
        fv = radf * angf * turb
        if fv > v:
            v = fv
    return v

# ------------------------------------------------------------- pixel shade
def shade(v, ramp_idx, bayer8, thr0=0.045, thr1=0.14):
    """Value [0,1+] -> palette index, dithered toward 255 in a soft band.
    bayer8 is the 0..63 cell value."""
    if v <= thr0:
        return 255
    if v < thr1:
        if bayer8 / 64.0 > (v - thr0) / (thr1 - thr0):
            return 255
    q = min(v, 1.0) * (len(ramp_idx) - 1)
    i0 = int(q)
    if i0 >= len(ramp_idx) - 1:
        return ramp_idx[-1]
    if (q - i0) > bayer8 / 64.0:
        return ramp_idx[i0 + 1]
    return ramp_idx[i0]

def render_frame(shape, N, fr, nf, nz, tongues, ramp_idx, gain_lo=0.55,
                 cscale=1.0):
    """cscale < 1 shrinks the art into the canvas center: keeps texel
    density while cutting the in-world footprint (1 texel = 1 unit)."""
    c = (N / 2.0 - 0.5) * cscale
    t = fr / (nf - 1) if nf > 1 else 0.0
    gain = 1.0 - (1.0 - gain_lo) * t
    rows = []
    for y in range(N):
        dy = (y - (N / 2.0 - 0.5)) / c
        line = bytearray(N)
        for x in range(N):
            dx = (x - (N / 2.0 - 0.5)) / c
            r = math.sqrt(dx * dx + dy * dy)
            th = math.atan2(dy, dx)
            if shape == "claw":
                v = v_claw(r, th, fr, nf, nz, tongues)
            else:
                v = v_radial(r, th, fr, nf, nz)
            line[x] = shade(v * gain, ramp_idx, BAYER8[(y & 7) * 8 + (x & 7)])
        rows.append(line)
    return rows

def render_frame_rgba(shape, N, fr, nf, nz, tongues, ramp_rgb_list,
                      gain_lo=0.55, cscale=1.0):
    """spr32 path: rows are raw R,G,B,A bytes (engine uploads RGBA8, so
    straight byte order). Soft alpha = sqrt curve over a feather band --
    no index-255 dither anywhere."""
    c = (N / 2.0 - 0.5) * cscale
    t = fr / (nf - 1) if nf > 1 else 0.0
    gain = 1.0 - (1.0 - gain_lo) * t
    rows = []
    for y in range(N):
        dy = (y - (N / 2.0 - 0.5)) / c
        line = bytearray(N * 4)
        for x in range(N):
            dx = (x - (N / 2.0 - 0.5)) / c
            r = math.sqrt(dx * dx + dy * dy)
            th = math.atan2(dy, dx)
            if shape == "claw":
                v = v_claw(r, th, fr, nf, nz, tongues) * gain
            else:
                v = v_radial(r, th, fr, nf, nz) * gain
            a = max(0.0, min(1.0, (v - 0.03) / 0.30)) ** 0.6
            if a <= 0.004:
                continue
            col = ramp_rgb(v, ramp_rgb_list)
            o = x * 4
            line[o:o + 4] = bytes((int(col[0] + 0.5), int(col[1] + 0.5),
                                   int(col[2] + 0.5), int(a * 255)))
        rows.append(line)
    return rows

# ------------------------------------------------------------ IDSP packer
def pack_spr(path, frames, N, orientation, synctype, interval, version=1):
    """Engine layout (gl_model.c Mod_LoadSpriteGroup:3474-3497): header,
    <i type=1> <i numsubs>, then a CONTIGUOUS block of numsubs float
    intervals, THEN numsubs back-to-back frame records (i ox, i oy,
    i w, i h, indexed pixels) with no interval fields."""
    out = bytearray()
    out += struct.pack("<4siifiiifi", b"IDSP", version, orientation,
                       math.hypot(N, N) / 2.0, N, N, 1, 0.0, synctype)
    out += struct.pack("<ii", 1, len(frames))          # SPR_GROUP, n subs
    out += struct.pack("<%df" % len(frames), *([interval] * len(frames)))
    for fr in frames:
        out += struct.pack("<iiii", -(N // 2), (N + 1) // 2, N, N)
        for line in fr:
            out += line
    open(path, "wb").write(bytes(out))
    return len(out)

# -------------------------------------------------------------- old sprite
def parse_spr(path):
    """Reader that mirrors the engine loader, used as a self-check that the
    packed layout above is what gl_model.c will actually parse."""
    data = open(path, "rb").read()
    ident, ver = struct.unpack_from("<4si", data, 0)
    assert ident == b"IDSP" and ver == 1, path
    nfr = struct.unpack_from("<i", data, 24)[0]
    pos, frames = 36, []
    def one():
        nonlocal pos
        ox, oy, fw, fh = struct.unpack_from("<iiii", data, pos); pos += 16
        px = data[pos:pos + fw * fh]; pos += fw * fh
        frames.append((fw, fh, [px[y * fw:(y + 1) * fw] for y in range(fh)]))
    for _ in range(nfr):
        ftype = struct.unpack_from("<i", data, pos); pos += 4
        if ftype[0] == 0:
            one()
        else:
            n = struct.unpack_from("<i", data, pos)[0]; pos += 4
            pos += 4 * n                               # contiguous intervals
            for _s in range(n):
                one()
    return frames

def _check_spr(data, nsubs, N):
    """Strict replay of Mod_LoadSpriteModel/Group over the raw bytes:
    header <4s i i f i i i f i>; per top frame: i type; group = i numframes,
    numframes CONTIGUOUS float intervals (cumulative, all > 0), then
    numframes records of <2i 2i> + w*h pixels. Consumes the whole file."""
    ident, ver, otype, brad, w, h, numframes, beam, sync = \
        struct.unpack_from("<4siifiiifi", data, 0)
    assert ident == b"IDSP" and ver in (1, 32)
    px = 4 if ver == 32 else 1
    assert numframes == 1, "expected single top-level frame"
    pos = 36
    ftype, = struct.unpack_from("<i", data, pos); pos += 4
    assert ftype == 1, "top frame must be SPR_GROUP"
    n, = struct.unpack_from("<i", data, pos); pos += 4
    assert n == nsubs, "group numframes %d != %d" % (n, nsubs)
    prev = 0.0
    for _ in range(n):
        iv, = struct.unpack_from("<f", data, pos); pos += 4
        assert iv > 0.0, "interval <= 0 at load"
        prev += iv                                   # loader accumulates
    for i in range(n):
        ox, oy, fw, fh = struct.unpack_from("<4i", data, pos); pos += 16
        assert fw == N and fh == N, \
            "subframe %d geometry %dx%d != %d (misparse!)" % (i, fw, fh, N)
        assert ox == -(N // 2) and oy == (N + 1) // 2, "origin off-center"
        pos += fw * fh * px
    assert pos == len(data), "parser consumed %d of %d bytes" % (pos, len(data))

# ------------------------------------------------------------- PNG preview
def png_write(path, W, H, rgb):
    raw = b"".join(b"\x00" + rgb[y * W * 3:(y + 1) * W * 3] for y in range(H))
    def chunk(t, d):
        return (struct.pack(">I", len(d)) + t + d +
                struct.pack(">I", zlib.crc32(t + d) & 0xFFFFFFFF))
    open(path, "wb").write(
        b"\x89PNG\r\n\x1a\n" +
        chunk(b"IHDR", struct.pack(">IIBBBBB", W, H, 8, 2, 0, 0, 0)) +
        chunk(b"IDAT", zlib.compress(raw, 9)) +
        chunk(b"IEND", b""))

def sheet_write(path, frame_sets, palette, scale=8, pad=6,
                bg=(16, 18, 22), rgba_flags=None):
    """frame_sets: rows of (w,h,rows) indexed frames.
    Row 0 = new art, row 1 (optional) = old art, side by side."""
    row_h = []
    for srow in frame_sets:
        row_h.append(max(fh for (_w, fh, _r) in srow) * scale + pad * 2)
    W = pad + max(sum(fw * scale + pad for (fw, _fh, _r) in srow) + pad
                  for srow in frame_sets)
    H = sum(row_h) + pad
    canvas = bytearray(W * H * 3)              # black gaps
    y_off = pad
    for ri, (srow, rh) in enumerate(zip(frame_sets, row_h)):
        is_rgba = bool(rgba_flags[ri]) if rgba_flags else False
        x_off = pad
        for (fw, fh, frows) in srow:
            for yy in range(fh):
                for xx in range(fw):
                    if is_rgba:
                        px = frows[yy][xx * 4:xx * 4 + 4]
                        a = px[3] / 255.0
                        rgb = tuple(int(px[k] * a + bg[k] * (1.0 - a))
                                    for k in range(3))
                    else:
                        idx = frows[yy][xx]
                        rgb = bg if idx == 255 else palette[idx]
                    for sy in range(scale):
                        dst = ((y_off + yy * scale + sy) * W
                               + x_off + xx * scale) * 3
                        for sx in range(scale):
                            o = dst + sx * 3
                            canvas[o:o + 3] = rgb
            x_off += fw * scale + pad
        y_off += rh
    png_write(path, W, H, canvas)

# -------------------------------------------------------------------- main
def main():
    ap = argparse.ArgumentParser(
        description="Procedural muzzle-flash sprite generator (Solitude).")
    here = os.path.dirname(os.path.abspath(__file__))
    root = os.path.dirname(here)
    ap.add_argument("--palette", default=os.path.join(
        root, "solitude", "gfx", "palette.lmp"))
    ap.add_argument("--shape", choices=["radial", "claw"], default="radial")
    ap.add_argument("--ramp", choices=sorted(RAMP), default="gunpowder")
    ap.add_argument("--size", type=int, default=48)
    ap.add_argument("--cscale", type=float, default=1.0,
                    help="art fraction of canvas (0.5 = half-size flash, "
                         "same texel density)")
    ap.add_argument("--frames", type=int, default=6)
    ap.add_argument("--interval", type=float, default=0.0167,
                    help="per-frame duration in seconds (0.0167 ~ 60fps)")
    ap.add_argument("--seed", type=int, default=1)
    ap.add_argument("--synctype", choices=["sync", "rand"], default="rand",
                    help="rand = each entity starts at a random phase")
    ap.add_argument("--orientation", type=int, default=2,
                    help="2=VP_PARALLEL (billboard), 0=VP_PARALLEL_UPRIGHT")
    ap.add_argument("--out", required=True)
    ap.add_argument("--preview", required=True)
    ap.add_argument("--old", help="existing .spr to render alongside for review")
    ap.add_argument("--rgba", action="store_true",
                    help="spr32: version 32 RGBA with soft alpha (no dither)")
    ap.add_argument("--scale", type=int, default=8)
    a = ap.parse_args()

    if a.size % 2 or a.size < 16:
        sys.exit("--size must be even and >= 16")
    palette = load_palette(a.palette)
    ramp_idx = sorted(RAMP[a.ramp], key=lambda i: lum(palette[i]))
    rng = random.Random(a.seed)
    nz = Noise(rng)
    tongues = make_tongues(rng) if a.shape == "claw" else []
    if a.rgba:
        ramp_cols = [palette[i] for i in ramp_idx]
        frames = [render_frame_rgba(a.shape, a.size, fr, a.frames, nz,
                                     tongues, ramp_cols, cscale=a.cscale)
                  for fr in range(a.frames)]
    else:
        frames = [render_frame(a.shape, a.size, fr, a.frames, nz, tongues,
                               ramp_idx, cscale=a.cscale)
                  for fr in range(a.frames)]
    os.makedirs(os.path.dirname(a.out) or ".", exist_ok=True)
    n = pack_spr(a.out, frames, a.size, a.orientation,
                 1 if a.synctype == "rand" else 0, a.interval,
                 version=32 if a.rgba else 1)
    # self-check with an engine-accurate parser (gl_model.c byte layout);
    # a wrong group layout must fail HERE, not in the player's face.
    with open(a.out, "rb") as f:
        _check_spr(f.read(), len(frames), a.size)
    print("%s  %d B  shape=%s ramp=%s size=%d frames=%d seed=%d" %
          (a.out, n, a.shape, a.ramp, a.size, a.frames, a.seed))

    sets = [[(a.size, a.size, fr) for fr in frames]]
    rgba_flags = [bool(a.rgba)]
    if a.old:
        sets.append(parse_spr(a.old)[:a.frames])
        rgba_flags.append(False)
    sheet_write(a.preview, sets, palette, scale=a.scale, rgba_flags=rgba_flags)
    print("preview %s" % a.preview)

if __name__ == "__main__":
    main()
