#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

"""Genere les assets animes des mecanismes a etat pour JustAnotherDnDGame (LOT-47).

Un asset par famille (`Door`, `Switch`, `PressurePlate`, `DangerSwitched`, `DangerBlink`,
`DangerMover`), chacun une spritesheet horizontale accompagnee de son fichier
`<asset>.anim.json` (`hmi::AnimationCatalog`) declarant les clips conventionnels documentes
dans `Source/Elements/Assets/README.md` (section « Apparence des mecanismes »).

Ces images sont volontairement schematiques (meme esprit que generate_test_skins.py et
generate_test_animations.py) : elles verifient que la correspondance etat -> clip et les
transitions se voient a l'ecran, pas des assets finaux. Le script n'a AUCUNE dependance
externe (PNG ecrit avec zlib + struct) : il tourne sur n'importe quel poste et en CI.

Usage :
    python scripts/generate_mechanism_animations.py [--output <dossier>]

Par defaut, ecrit dans Source/Elements/Assets/Skins/. Les fichiers generes ne sont PAS
assignes dans skins.json par ce script : voir Source/Elements/Assets/skins.json (jeu
"test"), deja mis a jour a la main pour les y assigner.
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
    """Bitmap RGBA simple, origine en haut a gauche (repris de generate_test_animations.py)."""

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


# --- Porte : un vantail dont la largeur visible varie de plein (fermee) a nul (ouverte). ---

FRAME_COLOR: Color = (60, 40, 24, 255)
WOOD_COLOR: Color = (140, 96, 52, 255)
HANDLE_COLOR: Color = (224, 192, 96, 255)


def door_frame(gap_fraction: float) -> Image:
    """gap_fraction 0 = vantail plein (fermee), 1 = passage entierement degage (ouverte)."""
    frame = Image(TILE, TILE, TRANSPARENT)
    frame.fill_rect(0, 0, TILE, TILE, FRAME_COLOR)  # dormant, toujours visible
    leaf_width = round((TILE - 2) * (1.0 - gap_fraction))
    frame.fill_rect(1, 1, leaf_width, TILE - 2, WOOD_COLOR)
    if leaf_width > 3:
        frame.fill_rect(leaf_width - 3, TILE // 2, 2, 2, HANDLE_COLOR)
    return frame


def door_sheet() -> Image:
    return horizontal_sheet(
        [
            door_frame(0.0),  # 0 : closed
            door_frame(0.35),  # 1 : opening (1/2)
            door_frame(0.7),  # 2 : opening (2/2)
            door_frame(1.0),  # 3 : open
            door_frame(0.7),  # 4 : closing (1/2)
            door_frame(0.35),  # 5 : closing (2/2)
        ]
    )


DOOR_JSON = {
    "version": 1,
    "frameWidth": TILE,
    "frameHeight": TILE,
    "clips": {
        "closed": {"frames": [0], "loop": True},
        "opening": {"frames": [1, 2], "frameDuration": 0.08, "loop": False, "next": "open"},
        "open": {"frames": [3], "loop": True},
        "closing": {"frames": [4, 5], "frameDuration": 0.08, "loop": False, "next": "closed"},
    },
}


# --- Interrupteur : levier bascule, plus lumineux quand actionne. ---

PLATE_COLOR: Color = (96, 96, 104, 255)
LEVER_OFF: Color = (72, 72, 80, 255)
LEVER_ON: Color = (232, 200, 64, 255)


def switch_frame(active: bool) -> Image:
    frame = Image(TILE, TILE, TRANSPARENT)
    frame.fill_rect(3, 11, 10, 3, PLATE_COLOR)  # socle
    lever = LEVER_ON if active else LEVER_OFF
    if active:
        frame.fill_rect(9, 3, 2, 9, lever)  # leve, vertical
    else:
        frame.fill_rect(4, 8, 8, 2, lever)  # couche, horizontal
    return frame


SWITCH_JSON = {
    "version": 1,
    "frameWidth": TILE,
    "frameHeight": TILE,
    "clips": {
        "inactive": {"frames": [0], "loop": True},
        "active": {"frames": [1], "loop": True},
    },
}


# --- Plaque de pression : enfoncee = plus basse et plus sombre. ---

PLATE_BASE: Color = (80, 80, 88, 255)
PLATE_TOP_RELEASED: Color = (150, 150, 162, 255)
PLATE_TOP_PRESSED: Color = (110, 110, 120, 255)


def plate_frame(pressed: bool) -> Image:
    frame = Image(TILE, TILE, TRANSPARENT)
    frame.fill_rect(1, 12, 14, 3, PLATE_BASE)  # socle, fixe
    top_height = 2 if pressed else 4
    top_y = 12 - top_height
    frame.fill_rect(2, top_y, 12, top_height, PLATE_TOP_PRESSED if pressed else PLATE_TOP_RELEASED)
    return frame


PLATE_JSON = {
    "version": 1,
    "frameWidth": TILE,
    "frameHeight": TILE,
    "clips": {
        "released": {"frames": [0], "loop": True},
        "pressed": {"frames": [1], "loop": True},
    },
}


# --- Danger commute : pics dormants (gris) / actifs (rouge vif). ---

SPIKE_DORMANT: Color = (120, 120, 128, 255)
SPIKE_ACTIVE: Color = (216, 32, 32, 255)


def danger_switched_frame(active: bool) -> Image:
    frame = Image(TILE, TILE, TRANSPARENT)
    color = SPIKE_ACTIVE if active else SPIKE_DORMANT
    for spike in range(4):
        x = spike * 4
        for row in range(4):
            width = 4 - row
            frame.fill_rect(x + row // 2, 12 - row * 3, max(1, width - row), 3, color)
    return frame


DANGER_SWITCHED_JSON = {
    "version": 1,
    "frameWidth": TILE,
    "frameHeight": TILE,
    "clips": {
        "inactive": {"frames": [0], "loop": True},
        "active": {"frames": [1], "loop": True},
    },
}


# --- Danger temporise : inoffensif (fondu) / mortel (plein, contour marque). ---

BLINK_HARMLESS: Color = (216, 32, 32, 90)
BLINK_LETHAL: Color = (248, 64, 32, 255)


def danger_blink_frame(lethal: bool) -> Image:
    frame = Image(TILE, TILE, TRANSPARENT)
    color = BLINK_LETHAL if lethal else BLINK_HARMLESS
    frame.fill_rect(2, 2, 12, 12, color)
    if lethal:
        frame.fill_rect(0, 0, TILE, 1, BLINK_LETHAL)
        frame.fill_rect(0, TILE - 1, TILE, 1, BLINK_LETHAL)
    return frame


DANGER_BLINK_JSON = {
    "version": 1,
    "frameWidth": TILE,
    "frameHeight": TILE,
    "clips": {
        "harmless": {"frames": [0], "loop": True},
        "lethal": {"frames": [1], "loop": True},
    },
}


# --- Danger mobile : une seule "scie" dont les dents tournent, en boucle continue. ---

SAW_BODY: Color = (140, 140, 150, 255)
SAW_TOOTH: Color = (216, 216, 224, 255)


def danger_mover_frame(phase: int) -> Image:
    frame = Image(TILE, TILE, TRANSPARENT)
    frame.fill_rect(4, 4, 8, 8, SAW_BODY)
    offsets = [(0, -1), (1, 0), (0, 1), (-1, 0)]
    dx, dy = offsets[phase % len(offsets)]
    frame.fill_rect(7 + dx * 5, 7 + dy * 5, 2, 2, SAW_TOOTH)
    return frame


DANGER_MOVER_JSON = {
    "version": 1,
    "frameWidth": TILE,
    "frameHeight": TILE,
    "clips": {"idle": {"frames": [0, 1, 2, 3], "frameDuration": 0.12, "loop": True}},
}


ASSETS = {
    "door.png": (door_sheet, DOOR_JSON),
    "switch.png": (
        lambda: horizontal_sheet([switch_frame(False), switch_frame(True)]),
        SWITCH_JSON,
    ),
    "plate.png": (
        lambda: horizontal_sheet([plate_frame(False), plate_frame(True)]),
        PLATE_JSON,
    ),
    "danger_switched.png": (
        lambda: horizontal_sheet([danger_switched_frame(False), danger_switched_frame(True)]),
        DANGER_SWITCHED_JSON,
    ),
    "danger_blink.png": (
        lambda: horizontal_sheet([danger_blink_frame(False), danger_blink_frame(True)]),
        DANGER_BLINK_JSON,
    ),
    "danger_mover.png": (
        lambda: horizontal_sheet([danger_mover_frame(i) for i in range(4)]),
        DANGER_MOVER_JSON,
    ),
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

    for name, (build_image, description) in ASSETS.items():
        image = build_image()
        (arguments.output / name).write_bytes(image.to_png())
        descriptor_name = pathlib.Path(name).with_suffix(".anim.json").name
        (arguments.output / descriptor_name).write_text(
            json.dumps(description, indent=2, ensure_ascii=False) + "\n", encoding="utf-8"
        )
        print(f"{name} : {image.width}x{image.height} + {descriptor_name}")

    print(f"\n{len(ASSETS)} assets de mecanismes ecrits dans {arguments.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
