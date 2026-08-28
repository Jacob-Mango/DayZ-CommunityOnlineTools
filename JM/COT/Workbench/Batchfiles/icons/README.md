# Icon pipeline (SVG -> `.edds`)

Converts an SVG icon set into DayZ `.edds` textures with a full mip chain, from
the command line. No Workbench step.

```
python build_icons.py --svg <svgDir> --out <eddsDir> [--size 128] [--verify]
```

`node` and `npm` must be on PATH (the rasteriser deps install themselves on the
first run) and Python needs `pillow`, `numpy` and `lz4`.

## The `.edds` container

`docs/ui/styling/styles-system.md` used to say `.edds` was an opaque Enfusion
format only Workbench could author. It is not opaque -- it is a DDS header
followed by per-mip LZ4:

```
[0x00 .. 0x80)   128-byte DDS header. Always uncompressed A8R8G8B8:
                 pfFlags 0x41, R=0x00FF0000 G=0x0000FF00 B=0x000000FF
                 A=0xFF000000, flags 0x0002100F, caps 0x00401008,
                 pitch = width * 4. The ASCII tag "ENF1" sits in
                 reserved[1] at offset 0x24 -- that tag is what makes it
                 an .edds rather than a plain .dds.
[0x80 .. )       chunk table, one 8-byte entry per mip, SMALLEST MIP FIRST:
                   char[4] codec ("COPY" or "LZ4 "), u32 chunkSize
[ .. EOF)        chunk payloads, same order:
                   "COPY" -> raw BGRA bytes
                   "LZ4 " -> u32 rawSize, u32 (compSize | 0x80000000),
                             then a raw LZ4 block (no frame, no size prefix)
```

The `0x80000000` bit on the second word is why a naive `lz4.block.decompress`
on the chunk fails -- the compressed data starts 8 bytes in, not at 0.

Confirmed against Workbench output: re-encoding `GUI/textures/new_cursor.edds`
reproduces its 128-byte header byte-for-byte and its mip 0 pixel-for-pixel. Only
the smaller mips differ, because Workbench uses a different downsample filter.

`edds.py` exposes `encode(pil_image)` and `decode(bytes)` if you need either
direction on its own.

For the wider picture -- `.edds` vs `.paa`, the `ImageToPAA` filename-suffix
rules, and how to convert a single image either way -- see
[`docs/ui/styling/textures-edds-and-paa.md`](../../../docs/ui/styling/textures-edds-and-paa.md).

## Conventions

- Icons render pure white with a shaped alpha channel, so one texture serves
  every colour: tint the `ImageWidget` with `SetColor()`. This matches the
  rounded-surface textures in `GUI/textures/ui/`.
- `currentColor` in the source SVG is substituted with `--color` (default
  `#FFFFFF`) before rasterising.
- `--size` must be a power of two. 128 is the project default: DayZ draws these
  at 24-48 px, so 128 leaves headroom for a hi-dpi pass without bloating the PBO.
- Downsampling runs on premultiplied alpha. Without that, the black RGB behind
  fully transparent pixels bleeds into the edge of a line-art glyph and the icon
  picks up a dark halo at small mips.

## Sets in this repo

| Output | Source | Count | License |
|---|---|---|---|
| `GUI/textures/icons/lucide/` | [Lucide](https://lucide.dev) `lucide-static` | 2034 | ISC -- no attribution required |
| `GUI/textures/icons/` | [game-icons.net](https://game-icons.net) (white/transparent archive) | 136 | CC BY 3.0 -- **attribution required**, see `GUI/textures/icons/CREDITS.md` |

`close.edds` is the one exception in the game-icons set: there is no `close` icon
upstream, so it comes from Lucide's `x`.
