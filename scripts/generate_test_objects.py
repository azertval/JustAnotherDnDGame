#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

"""Genere des textures d'objets interactifs de TEST pour JustAnotherDnDGame (LOT-45).

Ces images sont volontairement schematiques : elles servent a verifier que la surcharge de
texture par instance fonctionne (assignation, priorite sur le skin), pas a habiller le jeu. Un
artiste les remplacera par les vrais assets. Meme mecanique d'encodage PNG que
generate_test_skins.py/generate_test_backgrounds.py (aucune dependance externe).

Usage :
    python scripts/generate_test_objects.py [--output <dossier>]

Par defaut, ecrit dans Source/Elements/Assets/Objects/.
"""

from __future__ import annotations

import argparse
import pathlib
import struct
import zlib

TILE = 16
"""Cote d'une case, en pixels (hmi::TextureAtlas::TILE_SIZE)."""

Color = tuple[int, int, int, int]

TRANSPARENT: Color = (0, 0, 0, 0)


class Image:
    """Bitmap RGBA simple, origine en haut a gauche."""

    def __init__(self, width: int, height: int, fill: Color = TRANSPARENT) -> None:
        self.width = width
        self.height = height
        self.pixels: list[Color] = [fill] * (width * height)

    def set(self, x: int, y: int, color: Color) -> None:
        if 0 <= x < self.width and 0 <= y < self.height:
            self.pixels[y * self.width + x] = color

    def fill_rect(self, x: int, y: int, width: int, height: int, color: Color) -> None:
        for row in range(y, y + height):
            for column in range(x, x + width):
                self.set(column, row, color)

    def to_png(self) -> bytes:
        """Encode l'image en PNG (RGBA 8 bits, sans filtrage)."""
        raw = bytearray()
        for row in range(self.height):
            raw.append(0)  # type de filtre 0 (aucun) : suffisant a cette taille
            for column in range(self.width):
                raw.extend(self.pixels[row * self.width + column])

        def chunk(tag: bytes, data: bytes) -> bytes:
            body = tag + data
            return struct.pack(">I", len(data)) + body + struct.pack(">I", zlib.crc32(body))

        header = struct.pack(">IIBBBBB", self.width, self.height, 8, 6, 0, 0, 0)
        return (
            b"\x89PNG\r\n\x1a\n"
            + chunk(b"IHDR", header)
            + chunk(b"IDAT", zlib.compress(bytes(raw), 9))
            + chunk(b"IEND", b"")
        )


def door_red() -> Image:
    """Porte rouge : surcharge distincte de tout skin existant (LOT-42), bien reconnaissable."""
    panel: Color = (176, 40, 40, 255)
    frame: Color = (96, 16, 16, 255)
    handle: Color = (230, 200, 90, 255)

    cell = Image(TILE, TILE, panel)
    cell.fill_rect(0, 0, TILE, 2, frame)
    cell.fill_rect(0, TILE - 2, TILE, 2, frame)
    cell.fill_rect(0, 0, 2, TILE, frame)
    cell.fill_rect(TILE - 2, 0, 2, TILE, frame)
    cell.fill_rect(TILE - 5, TILE // 2 - 1, 2, 2, handle)
    return cell


def crate_blue() -> Image:
    """Caisse bleue : second exemple, pour verifier plusieurs surcharges distinctes cote a cote."""
    wood: Color = (60, 90, 150, 255)
    dark: Color = (30, 48, 84, 255)
    light: Color = (110, 150, 210, 255)

    cell = Image(TILE, TILE, wood)
    cell.fill_rect(0, 0, TILE, 1, light)
    cell.fill_rect(0, 0, 1, TILE, light)
    cell.fill_rect(0, TILE - 1, TILE, 1, dark)
    cell.fill_rect(TILE - 1, 0, 1, TILE, dark)
    for i in range(TILE):
        cell.set(i, i, dark)
        cell.set(TILE - 1 - i, i, dark)
    return cell


def barrel_brown() -> Image:
    """Tonneau brun : troisieme exemple, forme arrondie plutot que carree comme les deux premieres."""
    wood: Color = (110, 74, 40, 255)
    band: Color = (60, 40, 20, 255)
    light: Color = (150, 108, 62, 255)

    cell = Image(TILE, TILE, TRANSPARENT)
    cell.fill_rect(2, 1, TILE - 4, TILE - 2, wood)
    cell.fill_rect(1, 3, TILE - 2, TILE - 6, wood)
    for y in (2, TILE // 2, TILE - 3):
        cell.fill_rect(1, y, TILE - 2, 1, band)
    cell.fill_rect(4, 3, 2, TILE - 6, light)
    return cell


def sign_yellow() -> Image:
    """Panneau jaune : quatrieme exemple, motif d'avertissement contrastant avec les autres."""
    plate: Color = (222, 188, 64, 255)
    border: Color = (60, 50, 20, 255)
    mark: Color = (60, 50, 20, 255)

    cell = Image(TILE, TILE, plate)
    cell.fill_rect(0, 0, TILE, 2, border)
    cell.fill_rect(0, TILE - 2, TILE, 2, border)
    cell.fill_rect(0, 0, 2, TILE, border)
    cell.fill_rect(TILE - 2, 0, 2, TILE, border)
    cell.fill_rect(TILE // 2 - 1, 4, 2, 6, mark)
    cell.fill_rect(TILE // 2 - 1, 11, 2, 2, mark)
    return cell


def crate_green() -> Image:
    """Caisse verte : variante de teinte, pour distinguer deux blocs poussables du meme tableau."""
    wood: Color = (72, 118, 66, 255)
    dark: Color = (38, 68, 36, 255)
    light: Color = (116, 168, 106, 255)

    cell = Image(TILE, TILE, wood)
    cell.fill_rect(0, 0, TILE, 1, light)
    cell.fill_rect(0, 0, 1, TILE, light)
    cell.fill_rect(0, TILE - 1, TILE, 1, dark)
    cell.fill_rect(TILE - 1, 0, 1, TILE, dark)
    cell.fill_rect(2, TILE // 2 - 1, TILE - 4, 2, dark)
    return cell


def stone_block() -> Image:
    """Bloc de pierre : appareillage en briques decalees, contraste avec les caisses en bois."""
    stone: Color = (132, 128, 136, 255)
    joint: Color = (86, 82, 90, 255)
    light: Color = (166, 162, 172, 255)

    cell = Image(TILE, TILE, stone)
    for row, y in enumerate(range(0, TILE, 4)):
        cell.fill_rect(0, y, TILE, 1, joint)
        offset = 0 if row % 2 == 0 else TILE // 2
        cell.fill_rect(offset, y, 1, 4, joint)
        cell.fill_rect((offset + TILE // 2) % TILE, y, 1, 4, joint)
        cell.fill_rect(1, y + 1, TILE - 2, 1, light)
    return cell


def metal_plate() -> Image:
    """Plaque metallique : tole rivetee, note industrielle assortie au fond `test_industrial`."""
    metal: Color = (108, 116, 130, 255)
    dark: Color = (66, 72, 84, 255)
    rivet: Color = (156, 164, 178, 255)

    cell = Image(TILE, TILE, metal)
    cell.fill_rect(0, 0, TILE, 1, rivet)
    cell.fill_rect(0, TILE - 1, TILE, 1, dark)
    cell.fill_rect(0, 0, 1, TILE, rivet)
    cell.fill_rect(TILE - 1, 0, 1, TILE, dark)
    for y in (2, TILE - 4):
        for x in (2, TILE - 4):
            cell.fill_rect(x, y, 2, 2, rivet)
    return cell


def ice_block() -> Image:
    """Bloc de glace : bleu clair translucide, quatrieme famille visuelle de bloc poussable."""
    ice: Color = (152, 206, 232, 235)
    bright: Color = (206, 238, 250, 235)
    dark: Color = (96, 154, 190, 235)

    cell = Image(TILE, TILE, ice)
    cell.fill_rect(0, 0, TILE, 1, bright)
    cell.fill_rect(0, 0, 1, TILE, bright)
    cell.fill_rect(0, TILE - 1, TILE, 1, dark)
    cell.fill_rect(TILE - 1, 0, 1, TILE, dark)
    # Deux eclats obliques : la lecture « translucide » vient du contraste, pas de l'alpha seul.
    for i in range(TILE - 6):
        cell.set(3 + i, 3 + i, bright)
    for i in range(5):
        cell.set(TILE - 5 + i, 3 + i, bright)
    return cell


OBJECTS = {
    "door_red.png": door_red,
    "crate_blue.png": crate_blue,
    "barrel_brown.png": barrel_brown,
    "sign_yellow.png": sign_yellow,
    "crate_green.png": crate_green,
    "stone_block.png": stone_block,
    "metal_plate.png": metal_plate,
    "ice_block.png": ice_block,
}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--output",
        type=pathlib.Path,
        default=pathlib.Path(__file__).resolve().parent.parent
        / "Source"
        / "Elements"
        / "Assets"
        / "Objects",
        help="dossier de destination",
    )
    arguments = parser.parse_args()
    arguments.output.mkdir(parents=True, exist_ok=True)

    for name, build in OBJECTS.items():
        image = build()
        (arguments.output / name).write_bytes(image.to_png())
        print(f"{name} : {image.width}x{image.height}")

    print(f"\n{len(OBJECTS)} textures d'objets de test ecrites dans {arguments.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
