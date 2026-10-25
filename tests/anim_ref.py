#!/usr/bin/env python3
"""Reference output for tests/anim_dump.cpp using ~/Code/proj/unpac (usage: anim_ref.py ARCHIVE.lzs)."""
import os, struct, sys, zlib
sys.path.insert(0, os.path.expanduser('~/Code/proj'))
from unpac.revolution.brres.chr0 import Chr0
from unpac.revolution.brres.srt0 import Srt0
from unpac.revolution.brres.pat0 import Pat0
from unpac.revolution.brres.scn0 import Scn0
from unpac.revolution.brres.clr0 import Clr0, TARGETS

def lzss(src, size):
    ring = bytearray(4096); r = 4078; out = bytearray(); i = 0; flags = 0
    while len(out) < size and i < len(src):
        flags >>= 1
        if not flags & 0x100:
            flags = src[i] | 0xff00; i += 1
        if flags & 1:
            c = src[i]; i += 1; out.append(c); ring[r] = c; r = (r + 1) & 4095
        else:
            a = src[i]; b = src[i + 1]; i += 2
            pos = a | ((b & 0xf0) << 4); n = (b & 0xf) + 3
            for k in range(n):
                c = ring[(pos + k) & 4095]; out.append(c); ring[r] = c; r = (r + 1) & 4095
    return bytes(out)

d = open(sys.argv[1], 'rb').read()
if d[:4] == b'SSZL':
    d = lzss(d[16:], struct.unpack('<I', d[12:16])[0])
n = struct.unpack('<I', d[4:8])[0]
for i in range(n):
    o, s = struct.unpack('<II', d[0x40 + i * 0x40:0x48 + i * 0x40])
    name = d[0x48 + i * 0x40:0x80 + i * 0x40].split(b'\0')[0].decode()
    b = d[o:o + s]
    if b[:4] != b'bres':
        continue
    def block(magic):
        k = b.find(magic)
        while k >= 0 and k % 4:
            k = b.find(magic, k + 1)
        return k
    k = block(b'CHR0')
    if k >= 0:
        c = Chr0.from_bytes(b, k)
        print(f"{name} CHR0 frames={c.num_frame} loop={int(c.loops)}")
        for bone in sorted(c.nodes):
            for f in (0.0, 3.25, 17.0):
                ev = c.nodes[bone].evaluate(f)
                sc = ev['scale'] or (1.0, 1.0, 1.0); ro = ev['rotate'] or (0.0,) * 3; tr = ev['translate'] or (0.0,) * 3
                print(" %s @%.2f %.3f %.3f %.3f | %.3f %.3f %.3f | %.3f %.3f %.3f" % ((bone, f) + tuple(sc) + tuple(ro) + tuple(tr)))
    k = block(b'SRT0')
    if k >= 0:
        sr = Srt0.from_bytes(b, k)
        print(f"{name} SRT0 frames={sr.num_frame} mode={sr.tex_mtx_mode} loop={int(sr.loops)}")
        for m in ("g10b_sky01", "sky02", "cloud", "a_cloud", "z_cloud"):
            for mtx in range(2):
                for f in (0.0, 100.0):
                    mat = sr.materials.get(m)
                    if mat and f"tex{mtx}" in mat.tex:
                        e = mat.tex[f"tex{mtx}"].evaluate(sr.clip(f))
                        print(" %s.%d @%.0f %.3f %.3f %.3f %.3f %.3f" % (m, mtx, f, e['scale'][0], e['scale'][1], e['rotate'], e['translate'][0], e['translate'][1]))
    k = block(b'CLR0')
    if k >= 0:
        cl = Clr0.from_bytes(b, k)
        print(f"{name} CLR0 frames={cl.num_frame} loop={int(cl.loops)}")
        for m in sorted(cl.materials):
            for f in (0.0, 1.0, 7.0):
                for t, tn in enumerate(TARGETS):
                    tg = cl.materials[m].targets.get(tn)
                    if tg is None:
                        continue
                    r, g, bb, a = tg.color.evaluate(cl.clip(f))
                    print(" %s.%d @%.0f mask=%08x rgba=%02x%02x%02x%02x" % (m, t, f, tg.mask, r, g, bb, a))
    k = block(b'PAT0')
    if k >= 0:
        p = Pat0.from_bytes(b, k)
        ev = p.evaluate(0).get('pac_eye', {}).get(0, (None,))[0]
        print(f"{name} PAT0 frames={p.num_frame} map0@0='{ev or ''}'")
    k = block(b'SCN0')
    if k >= 0 and Scn0.from_bytes(b, k).cameras:
        sc = Scn0.from_bytes(b, k)
        cam = next(iter(sc.cameras.values()))
        e = cam.evaluate(0)
        aim = e['proj_type'] == 0 and (cam.flags & 1)
        print("%s SCN0 frames=%d aim=%d proj=%d pos=%.3f %.3f %.3f aim=%.3f %.3f %.3f twist=%.3f asp=%.3f near=%.3f far=%.1f fov=%.3f" % (
            name, sc.num_frame, cam.flags & 1, e['proj_type'], e['pos_x'], e['pos_y'], e['pos_z'],
            *( (e['aim_x'], e['aim_y'], e['aim_z']) if cam.flags & 1 else (e['rot_x'], e['rot_y'], e['rot_z'])),
            e.get('twist', 0.0) if cam.flags & 1 else 0.0, e['aspect'], e['near'], e['far'], e.get('persp_fovy', e.get('ortho_height', 0.0))))
