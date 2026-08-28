// Rasterises a directory of SVGs to square RGBA PNGs at a fixed size.
//
// Called by build_icons.py; not normally run by hand.
//   node rasterize_svg.js <svgDir> <pngDir> <size> [strokeColor]
//
// `currentColor` is substituted with strokeColor (default #FFFFFF) so the icons
// come out pure white with a shaped alpha channel, and a DayZ ImageWidget can
// tint them with SetColor() the same way the rounded-surface textures work.

const fs = require('fs');
const path = require('path');
const { Resvg } = require('@resvg/resvg-js');

const [svgDir, pngDir, sizeArg, colorArg] = process.argv.slice(2);
const size = parseInt(sizeArg, 10) || 128;
const color = colorArg || '#FFFFFF';

function walk(dir) {
  const out = [];
  for (const entry of fs.readdirSync(dir, { withFileTypes: true })) {
    const full = path.join(dir, entry.name);
    if (entry.isDirectory()) out.push(...walk(full));
    else if (entry.name.toLowerCase().endsWith('.svg')) out.push(full);
  }
  return out;
}

fs.mkdirSync(pngDir, { recursive: true });

const files = walk(svgDir);
let done = 0;
let failed = 0;

for (const file of files) {
  const name = path.basename(file, '.svg');
  const dest = path.join(pngDir, name + '.png');
  try {
    let svg = fs.readFileSync(file, 'utf8').replace(/currentColor/g, color);
    const resvg = new Resvg(svg, {
      fitTo: { mode: 'width', value: size },
      background: 'rgba(0,0,0,0)',
    });
    fs.writeFileSync(dest, resvg.render().asPng());
    done++;
  } catch (err) {
    failed++;
    console.error(`FAIL ${name}: ${err.message}`);
  }
}

console.log(`rasterized ${done} svg -> ${pngDir} (${size}px)${failed ? `, ${failed} failed` : ''}`);
