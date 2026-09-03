#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

"""Genere des skins de tuiles ANIMES de demonstration pour JustAnotherDnDGame (LOT-46).

Chaque asset est une spritesheet horizontale (les images se suivent de gauche a droite,
un seul rang de TILE pixels de haut) accompagnee de son fichier `<asset>.anim.json`
(`hmi::AnimationCatalog`, LOT-46 TACHE-03). Ces images sont volontairement schematiques :
elles servent a verifier que le moteur d'animation fonctionne (chargement, cache,
progression au pas fixe, mise en phase entre tuiles), pas a habiller le jeu.

Le script n'a AUCUNE dependance externe (PNG ecrit avec zlib + struct, comme
generate_test_skins.py) : il tourne sur n'importe quel poste et en CI, sans rien installer.

Usage :
    python scripts/generate_test_animations.py [--output <dossier>]

Par defaut, ecrit dans Source/Elements/Assets/Skins/. Les fichiers generes ne sont PAS
assignes dans skins.json : les assigner depuis le panneau "Textures" de l'editeur (ou a la
main) est un choix de contenu, pas une consequence du moteur.
"""

from __future__ import annotations

import argparse
import json
import pathlib
import struct
import zlib

TILE = 16
"""Cote d'une image, en pixels (hmi::TextureAtlas::TILE_SIZE)."""

Color = tuple[int, int, int, int]

TRANSPARENT: Color = (0, 0, 0, 0)


class Image:
    """Bitmap RGBA simple, origine en haut a gauche (repris de generate_test_skins.py)."""

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

    def blit(self, source: "Image", x: int, y: int) -> None:
        for row in range(source.height):
            for column in range(source.width):
                self.set(x + column, y + row, source.pixels[row * source.width + column])

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


def horizontal_sheet(frames: list[Image]) -> Image:
    """Assemble des images de meme taille en une spritesheet horizontale (un seul rang)."""
    sheet = Image(TILE * len(frames), TILE)
    for index, frame in enumerate(frames):
        sheet.blit(frame, index * TILE, 0)
    return sheet


def water_frame(phase: int) -> Image:
    """Nappe d'eau : bandes ondulantes qui se decalent d'une image a l'autre."""
    deep: Color = (28, 74, 168, 255)
    mid: Color = (58, 120, 214, 255)
    foam: Color = (176, 214, 240, 255)

    frame = Image(TILE, TILE, deep)
    for y in range(TILE):
        # Bande claire diagonale, decalee selon la phase : lecture immediate du mouvement.
        for x in range(TILE):
            if (x + y + phase * 3) % TILE < 3:
                frame.set(x, y, mid)
            if (x + y + phase * 3) % TILE == 0:
                frame.set(x, y, foam)
    return frame


def lava_frame(phase: int) -> Image:
    """Lave : coulee sombre ponctuee de points incandescents qui se deplacent."""
    base: Color = (120, 24, 8, 255)
    glow: Color = (232, 108, 24, 255)
    hot: Color = (250, 200, 60, 255)

    frame = Image(TILE, TILE, base)
    for i in range(6):
        x = (i * 5 + phase * 2) % TILE
        y = (i * 3 + phase) % TILE
        frame.set(x, y, hot)
        frame.fill_rect(max(0, x - 1), y, 1, 1, glow)
    return frame


def torch_frame(phase: int) -> Image:
    """Flamme de torche : socle fixe, flamme dont la hauteur/couleur flottent."""
    wood: Color = (92, 58, 28, 255)
    flame_outer: Color = (232, 108, 24, 255)
    flame_inner: Color = (250, 220, 90, 255)

    frame = Image(TILE, TILE)
    frame.fill_rect(6, 11, 4, 5, wood)  # socle du support, fixe d'une image a l'autre

    heights = [7, 9, 8]
    height = heights[phase % len(heights)]
    top = 11 - height
    frame.fill_rect(5, top, 6, height, flame_outer)
    frame.fill_rect(6, top + 1, 4, max(1, height - 2), flame_inner)
    return frame


def clip_json(name: str, frame_count: int, frame_duration: float) -> dict:
    return {
        "version": 1,
        "frameWidth": TILE,
        "frameHeight": TILE,
        "clips": {name: {"frames": list(range(frame_count)), "frameDuration": frame_duration, "loop": True}},
    }


ANIMATIONS = {
    "water.png": (lambda: horizontal_sheet([water_frame(i) for i in range(4)]), clip_json("wave", 4, 0.15)),
    "lava.png": (lambda: horizontal_sheet([lava_frame(i) for i in range(4)]), clip_json("bubble", 4, 0.2)),
    "torch.png": (lambda: horizontal_sheet([torch_frame(i) for i in range(3)]), clip_json("flicker", 3, 0.1)),
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
        / "Skins",
        help="dossier de destination",
    )
    arguments = parser.parse_args()
    arguments.output.mkdir(parents=True, exist_ok=True)

    for name, (build_image, description) in ANIMATIONS.items():
        image = build_image()
        (arguments.output / name).write_bytes(image.to_png())
        descriptor_name = pathlib.Path(name).with_suffix(".anim.json").name
        (arguments.output / descriptor_name).write_text(
            json.dumps(description, indent=2, ensure_ascii=False) + "\n", encoding="utf-8"
        )
        print(f"{name} : {image.width}x{image.height} + {descriptor_name}")

    print(f"\n{len(ANIMATIONS)} skins animes de demonstration ecrits dans {arguments.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
