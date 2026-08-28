"""SVG icon set -> DayZ .edds.

	python build_icons.py --svg <svgDir> --out <eddsDir> [--size 128]
	                      [--only names.txt] [--color "#FFFFFF"] [--verify]

Two stages: node rasterises every SVG to a square RGBA PNG (npm deps are
installed on first run), then edds.py packs each PNG into an Enfusion .edds
with a full mip chain. See edds.py for the container format.

Icons come out pure white with a shaped alpha channel, so an ImageWidget can
tint them with SetColor() instead of needing one texture per colour.
"""

import argparse
import os
import shutil
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from PIL import Image

import edds

HERE = os.path.dirname(os.path.abspath(__file__))


def ensure_node_deps():
	if os.path.isdir(os.path.join(HERE, "node_modules", "@resvg")):
		return
	print("[icons] installing node deps ...")
	# On Windows shutil.which() happily returns the extensionless shell script,
	# which CreateProcess cannot run -- ask for the .cmd shim first.
	npm = shutil.which("npm.cmd") or shutil.which("npm")
	if not npm:
		sys.exit("npm not found on PATH; required to rasterise SVGs")
	subprocess.run([npm, "install", "--no-audit", "--no-fund"], cwd=HERE, check=True)


def rasterize(svg_dir, png_dir, size, color):
	ensure_node_deps()
	node = shutil.which("node.exe") or shutil.which("node")
	if not node:
		sys.exit("node not found on PATH; required to rasterise SVGs")
	subprocess.run(
		[node, os.path.join(HERE, "rasterize_svg.js"), svg_dir, png_dir, str(size), color],
		cwd=HERE,
		check=True,
	)


def pack(png_dir, out_dir, keep):
	os.makedirs(out_dir, exist_ok=True)
	total = 0
	count = 0
	for name in sorted(os.listdir(png_dir)):
		if not name.lower().endswith(".png"):
			continue
		stem = name[:-4]
		if keep is not None and stem not in keep:
			continue
		with Image.open(os.path.join(png_dir, name)) as image:
			blob = edds.encode(image)
		dest = os.path.join(out_dir, stem + ".edds")
		with open(dest, "wb") as handle:
			handle.write(blob)
		total += len(blob)
		count += 1
	return count, total


def verify(out_dir):
	bad = 0
	for name in sorted(os.listdir(out_dir)):
		if not name.endswith(".edds"):
			continue
		path = os.path.join(out_dir, name)
		try:
			levels = edds.decode(open(path, "rb").read())
			if not levels or levels[-1][0] != 1 or levels[-1][1] != 1:
				raise ValueError("mip chain does not reach 1x1")
		except Exception as err:
			bad += 1
			print("VERIFY FAIL %s: %s" % (name, err))
	return bad


def main():
	parser = argparse.ArgumentParser()
	parser.add_argument("--svg", required=True, help="directory of source SVGs (searched recursively)")
	parser.add_argument("--out", required=True, help="output directory for .edds files")
	parser.add_argument("--size", type=int, default=128, help="edge length in pixels (power of two)")
	parser.add_argument("--color", default="#FFFFFF", help="colour substituted for currentColor")
	parser.add_argument("--only", help="file listing icon names (one per line) to keep")
	parser.add_argument("--verify", action="store_true", help="decode every output again after packing")
	parser.add_argument("--png-dir", help="keep the intermediate PNGs here instead of a temp dir")
	args = parser.parse_args()

	if args.size & (args.size - 1):
		sys.exit("--size must be a power of two (got %d)" % args.size)

	keep = None
	if args.only:
		with open(args.only) as handle:
			keep = {line.strip() for line in handle if line.strip()}

	png_dir = args.png_dir
	temp_dir = None
	if not png_dir:
		temp_dir = tempfile.mkdtemp(prefix="cot_icons_")
		png_dir = temp_dir

	try:
		rasterize(os.path.abspath(args.svg), os.path.abspath(png_dir), args.size, args.color)
		count, total = pack(png_dir, os.path.abspath(args.out), keep)
		print("[icons] packed %d .edds (%.1f MB) -> %s" % (count, total / 1048576.0, args.out))
		if args.verify:
			bad = verify(os.path.abspath(args.out))
			print("[icons] verify: %d bad" % bad)
			return 1 if bad else 0
	finally:
		if temp_dir:
			shutil.rmtree(temp_dir, ignore_errors=True)
	return 0


if __name__ == "__main__":
	sys.exit(main())
