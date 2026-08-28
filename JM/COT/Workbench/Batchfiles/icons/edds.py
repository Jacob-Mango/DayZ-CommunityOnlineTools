"""Enfusion .edds container: encode / decode.

An .edds file is NOT a plain DDS. Layout:

    [0x00 .. 0x80)   standard 128-byte DDS header, always uncompressed
                     A8R8G8B8 (pfFlags 0x41, R=0x00FF0000 G=0x0000FF00
                     B=0x000000FF A=0xFF000000), with the ASCII tag "ENF1"
                     written into reserved[1] at offset 0x24.
    [0x80 .. )       chunk table, one 8-byte entry per mip, ordered
                     SMALLEST mip first:  char[4] codec, u32 chunkSize
    [ .. EOF)        chunk payloads in the same order:
                       "COPY" -> raw BGRA bytes
                       "LZ4 " -> u32 rawSize, u32 (compSize | 0x80000000),
                                 then a raw LZ4 block (no frame, no size prefix)

Verified by round-tripping GUI/textures/new_cursor.edds: the 128-byte header
this module emits is byte-identical to the Workbench-authored one, and mip 0
decodes to identical pixels.
"""

import struct

import lz4.block
import numpy
from PIL import Image

ENF_TAG = b"ENF1"
HEADER_SIZE = 128


def _premultiply(image):
	rgba = numpy.asarray(image, dtype=numpy.uint16)
	alpha = rgba[:, :, 3:4]
	out = numpy.empty_like(rgba)
	out[:, :, :3] = rgba[:, :, :3] * alpha // 255
	out[:, :, 3:4] = alpha
	return Image.fromarray(out.astype(numpy.uint8), "RGBA")


def _unpremultiply(image):
	rgba = numpy.asarray(image, dtype=numpy.uint16)
	alpha = rgba[:, :, 3:4]
	safe = numpy.maximum(alpha, 1)
	out = numpy.empty_like(rgba)
	out[:, :, :3] = numpy.minimum(rgba[:, :, :3] * 255 // safe, 255)
	out[:, :, 3:4] = alpha
	return Image.fromarray(out.astype(numpy.uint8), "RGBA")


def build_mips(image):
	"""Full mip chain down to 1x1. Downsampling runs on premultiplied alpha so
	fully transparent pixels cannot bleed their (black) colour into the edges of
	a line-art icon."""
	image = image.convert("RGBA")
	mips = [image]
	w, h = image.size
	while w > 1 or h > 1:
		w = max(1, w // 2)
		h = max(1, h // 2)
		small = _premultiply(mips[-1]).resize((w, h), Image.LANCZOS)
		mips.append(_unpremultiply(small))
	return mips


def _swap_rb(data):
	"""RGBA <-> BGRA (the swap is its own inverse)."""
	flat = numpy.frombuffer(data, dtype=numpy.uint8).reshape(-1, 4)
	return flat[:, [2, 1, 0, 3]].tobytes()


def _header(width, height, mip_count):
	hdr = bytearray(HEADER_SIZE)
	hdr[0:4] = b"DDS "
	# size, flags (CAPS|HEIGHT|WIDTH|PITCH|PIXELFORMAT|MIPMAPCOUNT), h, w, pitch, depth, mips
	struct.pack_into("<7I", hdr, 4, 124, 0x0002100F, height, width, width * 4, 0, mip_count)
	hdr[0x24:0x28] = ENF_TAG
	struct.pack_into("<2I", hdr, 0x4C, 32, 0x41)  # pfSize, ALPHAPIXELS|RGB
	struct.pack_into("<5I", hdr, 0x58, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000)
	struct.pack_into("<I", hdr, 0x6C, 0x00401008)  # COMPLEX|TEXTURE|MIPMAP
	return bytes(hdr)


def encode(image):
	"""RGBA PIL image -> .edds bytes."""
	mips = build_mips(image)
	width, height = mips[0].size

	table = bytearray()
	blobs = bytearray()
	for mip in reversed(mips):  # smallest mip first
		raw = _swap_rb(mip.tobytes())
		comp = lz4.block.compress(raw, mode="high_compression", compression=12, store_size=False)
		if len(comp) + 8 < len(raw):
			chunk = struct.pack("<2I", len(raw), len(comp) | 0x80000000) + comp
			tag = b"LZ4 "
		else:
			chunk = raw
			tag = b"COPY"
		table += tag + struct.pack("<I", len(chunk))
		blobs += chunk

	return _header(width, height, len(mips)) + bytes(table) + bytes(blobs)


def decode(data):
	"""`.edds` bytes -> list of (width, height, rgba_bytes), mip 0 first."""
	if data[:4] != b"DDS " or data[0x24:0x28] != ENF_TAG:
		raise ValueError("not an Enfusion .edds (missing DDS magic or ENF1 tag)")
	height, width = struct.unpack_from("<2I", data, 12)
	mip_count = struct.unpack_from("<I", data, 28)[0]

	off = HEADER_SIZE
	table = []
	for _ in range(mip_count):
		table.append((data[off:off + 4], struct.unpack_from("<I", data, off + 4)[0]))
		off += 8

	levels = []
	for index, (tag, size) in enumerate(table):
		level = mip_count - 1 - index
		mw = max(1, width >> level)
		mh = max(1, height >> level)
		blob = data[off:off + size]
		off += size
		if tag == b"COPY":
			raw = blob
		elif tag == b"LZ4 ":
			raw_size = struct.unpack_from("<I", blob, 0)[0]
			raw = lz4.block.decompress(blob[8:], uncompressed_size=raw_size)
		else:
			raise ValueError("unknown .edds chunk codec %r" % tag)
		if len(raw) != mw * mh * 4:
			raise ValueError("mip %d size mismatch: %d != %d" % (level, len(raw), mw * mh * 4))
		levels.append((mw, mh, _swap_rb(raw)))

	return list(reversed(levels))
