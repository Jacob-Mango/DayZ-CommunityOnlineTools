# Converting an image to `.edds` and `.paa`

DayZ's UI reads two texture formats, and both work anywhere a texture path is
accepted -- `image0` in a `.layout`, `ImageWidget.LoadImageFile()`,
`SetImage()`, an `.imageset` atlas source. Everything below was verified against
the tools in this repo; sizes are measured, not estimated.

Related: [[imagesets]] for atlas packing, [[colors-alpha]] for tinting a white
texture at runtime.

## Which format

| | `.paa` | `.edds` |
|---|---|---|
| What it is | BI's DXT container (DXT1/DXT5/ARGB...) | Enfusion container: DDS header + per-mip LZ4 over BGRA |
| Compression | block DXT, **lossy** | LZ4, **lossless** |
| Made by | `ImageToPAA.exe` (DayZ Tools) | `Workbench/Batchfiles/icons/build_icons.py` |
| Disk, 136 icons @128px | 2.98 MB | **745 KB** |
| VRAM, one 128px texture + mips | **~22 KB** (stays DXT on the GPU) | ~87 KB (BGRA8, 4 bytes/px) |

Those two rows are the whole trade-off, and they point opposite ways:

- **`.edds` for UI icons and line art.** DXT works on 4x4 blocks, so a hard
  white-on-transparent edge picks up block artifacts that are very visible on a
  crisp glyph. LZ4 is lossless, and sparse art compresses far better than DXT's
  fixed rate -- hence 4x smaller on disk for the icon set.
- **`.paa` for large, photographic, or many-at-once textures.** DXT stays
  compressed in video memory; `.edds` is expanded to BGRA8. On a full-screen
  background or a big atlas that difference matters more than disk size does.

Both carry a full mip chain, and both are already in `Workbench/includes.txt`,
so either gets packed into the PBO with no extra config.

## Source image rules

Applies to both formats:

- **Power-of-two dimensions.** `ImageToPAA` hard-fails otherwise:
  `Error (Img is not of power of 2 size)`. Non-square is fine as long as both
  edges are powers of two (256x64 converts cleanly).
- **RGBA with a real alpha channel.** For anything you intend to tint, author it
  **pure white with a shaped alpha** and colour it at runtime with `SetColor()`.
  One texture then serves every state and theme. This is the convention for
  `GUI/textures/icons/` and `GUI/textures/ui/`.
- **128px is the project default for icons.** DayZ draws them at 24-48 px, so
  128 leaves hi-dpi headroom without bloating the PBO.
- PNG and TGA are both accepted as input by either pipeline.

## To `.paa`

`ImageToPAA.exe` ships with DayZ Tools:

```
"C:\Program Files (x86)\Steam\steamapps\common\DayZ Tools\Bin\ImageToPAA\ImageToPAA.exe" <source> [<destination>]
```

```bash
# single file
ImageToPAA.exe icon.png icon.paa

# downscale on the way in
ImageToPAA.exe -size=32 icon.png icon32.paa

# whole tree -- recurses into subfolders, writes each .paa beside its source
ImageToPAA.exe path/to/textures
```

`-size=<n>` is the only documented option (`ImageToPAA.exe -help` prints the
usage under the tool's original name, `pal2pace`).

### The output format is chosen by the filename

`ImageToPAA` does not take a format flag. It matches the filename against
`Bin/ImageToPAA/TexConvert.cfg` and picks from that. The suffix rules that
matter for UI work:

| Source name | Format | First 2 bytes of the `.paa` |
|---|---|---|
| `icon.png` (no suffix) | DXT5 | `05 FF` |
| `icon_ca.png` | DXT5 -- colour **+ alpha** | `05 FF` |
| `icon_co.png` | DXT1 -- colour, **alpha discarded** | `01 FF` |
| `icon_8888.png` | ARGB1555 | `55 15` |

Verified: the same 128px RGBA icon comes out 22016 bytes as `cog.png` or
`cog_ca.png`, and 11096 bytes as `cog_co.png` -- half the size, because DXT1
threw the alpha away. **A `_co` suffix on a UI icon silently destroys its
transparency.** Leave icons unsuffixed (or use `_ca`), and check the tag bytes
if something renders as an opaque block:

```bash
xxd -p -l2 icon.paa      # expect 05ff for DXT5
```

`TexConvert.cfg` carries many more rules (`_nohq` normal maps, `_smdi`, ...)
that apply to model textures, not UI.

## To `.edds`

`.edds` has no converter in DayZ Tools -- this repo ships one at
[`Workbench/Batchfiles/icons/`](../../../Workbench/Batchfiles/icons/). It needs
`node`/`npm` on PATH (the rasteriser deps install themselves on first run) and
Python with `pillow`, `numpy`, `lz4`.

### From SVG (the icon-set path)

```bash
python Workbench/Batchfiles/icons/build_icons.py \
    --svg <svgDir> --out <eddsDir> --size 128 --verify
```

It rasterises every SVG in the tree to a square RGBA PNG, then packs each one.
`currentColor` in the source SVG is replaced with `--color` (default
`#FFFFFF`) so the output follows the white-plus-alpha convention. `--size` must
be a power of two. `--verify` decodes every output again and checks that the mip
chain reaches 1x1.

### From a single PNG or TGA

`edds.py` is importable for one-off work:

```python
import sys
sys.path.insert(0, "Workbench/Batchfiles/icons")

import edds
from PIL import Image

with Image.open("icon.png") as image:
    open("icon.edds", "wb").write(edds.encode(image))
```

`edds.decode(bytes)` goes the other way, returning
`[(width, height, rgba_bytes), ...]` with mip 0 first -- useful for eyeballing
what actually landed in a texture:

```python
levels = edds.decode(open("icon.edds", "rb").read())
w, h, px = levels[0]
Image.frombytes("RGBA", (w, h), px).save("check.png")
```

### The container

Documented in full in
[`Workbench/Batchfiles/icons/README.md`](../../../Workbench/Batchfiles/icons/README.md)
and in [[styles-system]]. The short version:

```
[0x00 .. 0x80)   128-byte DDS header, uncompressed A8R8G8B8 (pfFlags 0x41),
                 ASCII tag "ENF1" in reserved[1] at offset 0x24
[0x80 .. )       chunk table, one 8-byte entry per mip, SMALLEST MIP FIRST:
                   char[4] codec ("COPY" or "LZ4 "), u32 chunkSize
[ .. EOF)        chunk payloads, same order:
                   "COPY" -> raw BGRA bytes
                   "LZ4 " -> u32 rawSize, u32 (compSize | 0x80000000),
                             then a raw LZ4 block (no frame, no size prefix)
```

Two things that bite when reading one by hand: the tag sits at `0x24`, not
`0x28`, and the LZ4 payload starts 8 bytes into the chunk -- decompressing from
byte 0 fails with a corrupt-input error.

A Workbench-imported `.edds` also gets a `.meta` sidecar naming its source
image. The engine does not need it; the icon sets under `GUI/textures/icons/`
ship without one.

### Why downsampling runs on premultiplied alpha

`edds.py` premultiplies before each `resize` and undoes it after. Skip that and
the black RGB sitting behind fully transparent pixels gets averaged into the
edge of a glyph, so the icon grows a dark halo at the smaller mips -- which is
exactly what you see on screen, since a 24 px draw of a 128 px texture samples
mip 2.

## Referencing the result

Paths are PBO-relative and case-insensitive at runtime:

```cpp
// .layout
image0 "JM/COT/GUI/textures/icons/close.edds";
```

```c
// Enforce Script
static const string ICON_COG = "JM/COT/GUI/textures/icons/cog.edds";
image.LoadImageFile( 0, JMConstants.ICON_COG );

// the Lucide set is addressed by name instead of one constant per icon
image.LoadImageFile( 0, JMConstants.Lucide( "shield-check" ) );
```

A raw texture path **cannot** be used from a `.styles` `<Item>` -- styles
resolve against an imageset atlas, and a raw path there crashes the renderer on
first paint. See [[styles-system]].

## Checklist

1. Power-of-two dimensions, RGBA, white + shaped alpha if it will be tinted.
2. Icon or line art -> `.edds`. Large or photographic -> `.paa`.
3. `.paa`: no `_co` suffix unless you actually want the alpha gone; confirm
   `05ff`.
4. `.edds`: run `build_icons.py --verify`, or `edds.decode()` it back to a PNG
   and look at it.
5. `./dz-project-manager.exe build` -- textures are client-side, so a server
   `BOOT_OK` proves the script compiles, not that the texture paints. Launch a
   client to confirm that.
