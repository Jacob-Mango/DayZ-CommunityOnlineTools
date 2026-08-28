#!/usr/bin/env python3
"""Generate GUI/styles/cot_merged.styles from vanilla DayZ styles + COT styles.

WHY THIS EXISTS
---------------
MissionGameplay.OnInit() calls the engine's LoadWidgetStyles() to register COT's
own widget styles. That call operates on the ONE global widget-style table, and a
file that does not mention a widget type leaves that type with nothing.

COT's authored file (GUI/styles/cot.styles) only describes ButtonWidget,
CheckBoxWidget, EditBoxWidget, GridSpacerWidget, PanelWidget, ScrollWidget and
WrapSpacerWidget, and only the COT* styles inside them. Everything vanilla
defines - WindowWidget's "Default", PanelWidget's "blank" / "rover_sim_black" /
"rover_sim_colorable", the whole TextWidget set - was absent, which is what
stopped every MapWidget in the game from drawing: both DayZ's own map layout and
COT's mount their map inside a WindowWidget and back it with those panel styles.

So COT ships a MERGED table instead: vanilla's file with COT's styles folded into
the matching widget blocks. Loading that is correct whether the engine replaces
the table or merges into it - under a merge, the vanilla half is simply
re-registering values it already had.

USAGE
-----
    python Workbench/Batchfiles/MergeWidgetStyles.py [--vanilla PATH] [--check]

Run it after every edit to GUI/styles/cot.styles. cot.styles stays the authored
source of truth; cot_merged.styles is generated and should not be hand-edited.
--check exits non-zero if the generated file is out of date, for CI.
"""

import argparse
import io
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, "..", ".."))

COT_STYLES = os.path.join(REPO, "GUI", "styles", "cot.styles")
OUT_STYLES = os.path.join(REPO, "GUI", "styles", "cot_merged.styles")

DEFAULT_VANILLA = r"P:\gui\looknfeel\dayzwidgets.styles"

WIDGET_BLOCK = re.compile(r'<Widget\s+Name="([^"]+)"\s*>(.*?)</Widget>', re.S)

HEADER = """<!--
    GENERATED FILE - DO NOT EDIT.

    Produced by Workbench/Batchfiles/MergeWidgetStyles.py from
    DayZ's gui/looknfeel/dayzwidgets.styles plus GUI/styles/cot.styles.

    Edit GUI/styles/cot.styles and re-run the script.

    This is the file MissionGameplay.OnInit() hands to LoadWidgetStyles(): the
    engine keeps one global widget-style table, so a file that omits a widget
    type can leave that type with no styles at all. Shipping vanilla's table
    with COT's styles folded in keeps every vanilla style reachable no matter
    how the engine treats the call.
-->
"""


def read(path):
    with io.open(path, encoding="utf-8", errors="replace", newline="") as f:
        return f.read()


def parse_blocks(text):
    """Ordered list of (widget_name, inner_xml)."""
    return [(m.group(1), m.group(2)) for m in WIDGET_BLOCK.finditer(text)]


def style_names(inner):
    return re.findall(r'<Style\s+Name="([^"]+)"', inner)


def build(vanilla_path):
    vanilla = read(vanilla_path)
    cot = read(COT_STYLES)

    cot_blocks = parse_blocks(cot)
    if not cot_blocks:
        sys.exit("no <Widget> blocks found in %s" % COT_STYLES)

    merged = vanilla
    appended = []

    for name, inner in cot_blocks:
        added = style_names(inner)
        if not added:
            continue

        # Drop any vanilla style COT redefines, so the COT copy is the only one
        # left for that (widget, style) pair rather than a duplicate key.
        pattern = re.compile(
            r'<Widget\s+Name="%s"\s*>(.*?)</Widget>' % re.escape(name), re.S
        )
        match = pattern.search(merged)

        if match:
            body = match.group(1)
            for style in added:
                body = drop_style(body, style)
            new_block = '<Widget Name="%s">%s\n%s\n    </Widget>' % (
                name,
                body.rstrip(),
                inner.strip("\n").rstrip(),
            )
            merged = merged[: match.start()] + new_block + merged[match.end():]
        else:
            appended.append('    <Widget Name="%s">%s\n    </Widget>' % (name, inner.rstrip()))

    if appended:
        merged = merged.replace("</WidgetStyles>", "\n".join(appended) + "\n</WidgetStyles>")

    # Header goes after the root open tag; a comment before it is not valid here.
    merged = merged.replace("<WidgetStyles>", "<WidgetStyles>\n" + HEADER, 1)
    return merged


def drop_style(body, style):
    """Remove one <Style Name="style" .../> or <Style ...>...</Style> from a block."""
    self_closing = re.compile(r'\s*<Style\s+Name="%s"[^>]*?/>' % re.escape(style))
    body, n = self_closing.subn("", body)
    if n:
        return body

    paired = re.compile(
        r'\s*<Style\s+Name="%s"[^>]*?>.*?</Style>' % re.escape(style), re.S
    )
    return paired.sub("", body)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--vanilla", default=DEFAULT_VANILLA)
    ap.add_argument("--check", action="store_true")
    args = ap.parse_args()

    if not os.path.exists(args.vanilla):
        sys.exit(
            "vanilla styles not found: %s\n"
            "Mount the work drive (dz-project-manager mount-workdrive) or pass --vanilla."
            % args.vanilla
        )

    merged = build(args.vanilla)

    if args.check:
        if not os.path.exists(OUT_STYLES) or read(OUT_STYLES) != merged:
            sys.exit("cot_merged.styles is out of date - re-run MergeWidgetStyles.py")
        print("cot_merged.styles up to date")
        return

    with io.open(OUT_STYLES, "w", encoding="utf-8", newline="\n") as f:
        f.write(merged)

    print(
        "wrote %s (%d bytes, %d widget blocks)"
        % (OUT_STYLES, len(merged), len(parse_blocks(merged)))
    )


if __name__ == "__main__":
    main()
