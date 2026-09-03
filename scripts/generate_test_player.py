#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

"""Genere une spritesheet de personnage de TEST pour JustAnotherDnDGame (LOT-48, livree en LOT-65).

`Player/` ne contenait jusqu'ici aucun fichier : le personnage retombait systematiquement sur la
silhouette procedurale historique (`atlas.png`) faute d'un seul asset livre, alors que le contrat
(`hmi::AssetFamily::CharacterSheet`, `Documentation/Lot/LOT-48-personnage-texture/`) est complet
depuis longtemps. Ce script comble ce trou avec un personnage schematique, dans le meme esprit que
les autres assets de test du depot (generes par script, reproductibles, sans dependance externe).

Une seule spritesheet horizontale, une image par case de `TILE` pixels, accompagnee de
`player.anim.json` (`hmi::AnimationCatalog`) declarant les sept clips attendus
(`idle`, `run`, `jump`, `fall`, `land`, `wallslide`, `dash`) -- meme patron que
generate_mechanism_animations.py pour les clips multiples dans un seul fichier. Le personnage est
dessine dans le seul sens attendu (vers la DROITE) : le moteur retourne l'image pour l'autre sens.

Usage :
    python scripts/generate_test_player.py [--output <dossier>]

Par defaut, ecrit dans Source/Elements/Assets/Player/.
"""

from __future__ import annotations

import argparse
import json
import pathlib
import struct
import zlib

TILE = 16
"""Cote d'une case, en pixels (hmi::TextureAtlas::TILE_SIZE) -- une image par case, comme les
autres skins animes de single case (LOT-46/LOT-47), plutot qu'un format dedie au personnage."""

Color = tuple[int, int, int, int]

TRANSPARENT: Color = (0, 0, 0, 0)

SKIN: Color = (240, 198, 158, 255)
SKIN_SHADOW: Color = (206, 158, 120, 255)
HAIR: Color = (96, 58, 36, 255)
HAIR_LIGHT: Color = (140, 90, 54, 255)
EYE: Color = (36, 30, 34, 255)
TUNIC: Color = (86, 138, 210, 255)
TUNIC_DARK: Color = (52, 92, 154, 255)
TUNIC_LIGHT: Color = (128, 178, 238, 255)
BELT: Color = (120, 78, 44, 255)
LEG: Color = (72, 82, 112, 255)
LEG_DARK: Color = (46, 54, 78, 255)
BOOT: Color = (128, 84, 48, 255)
BOOT_DARK: Color = (92, 58, 32, 255)
OUTLINE: Color = (26, 24, 34, 255)
TRAIL: Color = (206, 226, 250, 150)
WALL: Color = (58, 56, 66, 255)
SPARK: Color = (250, 232, 170, 220)


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


def outline(image: Image, color: Color = OUTLINE) -> None:
    """Cerne la silhouette d'un liseré sombre d'un pixel.

    Un aplat sans contour se confond avec le decor des qu'il passe devant une tuile de teinte
    voisine ; c'est ce qui rendait le personnage precedent illisible. Le cerne est calcule depuis
    la silhouette plutot que dessine a la main, de sorte qu'il reste juste quelle que soit la pose.
    """
    opaque = [pixel[3] > 0 for pixel in image.pixels]
    for y in range(image.height):
        for x in range(image.width):
            if opaque[y * image.width + x]:
                continue
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                nx, ny = x + dx, y + dy
                if 0 <= nx < image.width and 0 <= ny < image.height and opaque[ny * image.width + nx]:
                    image.set(x, y, color)
                    break


def head(image: Image, x: int, y: int, *, blink: bool = False) -> None:
    """Tete 6x5 : chevelure, visage ombre du cote oppose a la lumiere, oeil oriente a DROITE."""
    image.fill_rect(x, y + 1, 6, 4, SKIN)
    image.fill_rect(x, y + 1, 1, 4, SKIN_SHADOW)   # cote a l'ombre
    image.fill_rect(x, y, 6, 2, HAIR)              # chevelure
    image.fill_rect(x + 1, y, 4, 1, HAIR_LIGHT)    # reflet
    image.fill_rect(x + 5, y + 2, 1, 1, HAIR)      # meche devant l'oreille
    if blink:
        image.fill_rect(x + 3, y + 3, 2, 1, SKIN_SHADOW)
    else:
        image.fill_rect(x + 4, y + 2, 1, 2, EYE)
    image.fill_rect(x + 1, y + 4, 3, 1, SKIN_SHADOW)  # menton


def torso(image: Image, x: int, y: int, height: int = 5) -> None:
    """Buste : tunique eclairee a droite, ombree a gauche, ceinture en bas."""
    image.fill_rect(x, y, 7, height, TUNIC)
    image.fill_rect(x, y, 2, height, TUNIC_DARK)
    image.fill_rect(x + 5, y, 2, height, TUNIC_LIGHT)
    image.fill_rect(x, y + height - 1, 7, 1, BELT)


def arm(image: Image, x: int, y: int, height: int, *, back: bool = False) -> None:
    """Bras : deux pixels de large, plus sombre quand il s'agit du bras arriere."""
    image.fill_rect(x, y, 2, height, TUNIC_DARK if back else TUNIC)
    image.fill_rect(x, y + height - 1, 2, 1, SKIN)  # main


def leg(image: Image, x: int, y: int, height: int, *, back: bool = False) -> None:
    """Jambe + botte : la jambe arriere est assombrie pour separer les deux a la lecture."""
    image.fill_rect(x, y, 2, height, LEG_DARK if back else LEG)
    image.fill_rect(x, y + height - 1, 3, 1, BOOT_DARK if back else BOOT)


def idle_frame(phase: int) -> Image:
    """Repos : respiration d'un pixel, et un clignement sur la troisieme image."""
    bob = 1 if phase == 1 else 0
    image = Image(TILE, TILE)
    head(image, 5, 1 + bob, blink=(phase == 2))
    torso(image, 5, 6 + bob)
    arm(image, 3, 6 + bob, 5, back=True)
    arm(image, 11, 6 + bob, 5)
    leg(image, 5, 11 + bob, 4 - bob)
    leg(image, 9, 11 + bob, 4 - bob, back=True)
    outline(image)
    return image


def run_frame(phase: int) -> Image:
    """Course : six phases distinctes (contact, passage, envol) plutot que quatre dont deux
    identiques -- l'ancien cycle rendait la foulee saccadee, deux images sur quatre ne bougeant
    pas. Le buste penche vers l'avant et monte d'un pixel a l'envol."""
    #            (jambe avant, jambe arriere, hauteur du buste, bras avant)
    poses = [
        (2, -2, 0, 1),
        (3, -1, -1, 2),
        (1, -3, 0, 0),
        (-2, 2, 0, -1),
        (-1, 3, -1, -2),
        (-3, 1, 0, 0),
    ]
    front, back, lift, swing = poses[phase % len(poses)]
    image = Image(TILE, TILE)
    head(image, 5, 1 + lift)
    torso(image, 4, 6 + lift)
    # Les bras balancent en HAUTEUR, jamais en largeur : le buste occupe x=4..10, et un bras
    # translate horizontalement s'y noyait -- trois des six images se retrouvaient sans bras.
    arm(image, 2, 6 + lift + (1 if swing > 0 else 0), 4, back=True)
    arm(image, 11, 6 + lift + (0 if swing > 0 else 1), 4)
    leg(image, 6 + front, 11 + lift, 4)
    leg(image, 6 + back, 11 + lift, 4, back=True)
    outline(image)
    return image


def jump_frame() -> Image:
    """Envol : silhouette ETIREE (squash & stretch), bras leve, jambes repliees."""
    image = Image(TILE, TILE)
    head(image, 5, 0)
    torso(image, 5, 5, height=6)
    arm(image, 11, 2, 5)          # bras leve vers le haut
    arm(image, 3, 7, 4, back=True)
    leg(image, 5, 11, 3)
    leg(image, 9, 10, 3, back=True)
    outline(image)
    return image


def fall_frame() -> Image:
    """Chute : bras ecartes vers le haut, jambes ouvertes -- silhouette en X, lisible d'un coup."""
    image = Image(TILE, TILE)
    head(image, 5, 2)
    torso(image, 5, 7, height=4)
    arm(image, 2, 4, 4, back=True)
    arm(image, 12, 4, 4)
    leg(image, 3, 11, 3)
    leg(image, 10, 11, 3, back=True)
    outline(image)
    return image


def land_frame(phase: int) -> Image:
    """Reception : silhouette ECRASEE (phase 0), puis retour au repos (phase 1)."""
    if phase != 0:
        return idle_frame(0)
    image = Image(TILE, TILE)
    head(image, 5, 4)
    torso(image, 4, 9, height=4)
    arm(image, 2, 9, 3, back=True)
    arm(image, 11, 9, 3)
    image.fill_rect(4, 13, 3, 2, LEG)      # jambes pliees, tres courtes
    image.fill_rect(9, 13, 3, 2, LEG_DARK)
    image.fill_rect(3, 15, 4, 1, BOOT)
    image.fill_rect(9, 15, 4, 1, BOOT_DARK)
    outline(image)
    return image


def wallslide_frame() -> Image:
    """Glissade murale : dos plaque contre la paroi de droite, bras tendu, jambes flechies."""
    image = Image(TILE, TILE)
    head(image, 6, 2)
    torso(image, 6, 7, height=5)
    arm(image, 11, 6, 3)                   # bras plaque contre la paroi
    arm(image, 4, 8, 4, back=True)
    leg(image, 6, 12, 3)
    leg(image, 9, 11, 4, back=True)
    outline(image)
    image.fill_rect(14, 2, 2, 13, WALL)    # paroi : repere, dessine APRES le cerne
    for y in range(3, 14, 3):              # etincelles de frottement
        image.fill_rect(13, y, 1, 1, SPARK)
    return image


def dash_frame() -> Image:
    """Ruee : corps horizontal, tendu vers l'avant, avec une trainee derriere lui."""
    image = Image(TILE, TILE)
    head(image, 8, 5)
    image.fill_rect(3, 6, 6, 4, TUNIC)     # buste allonge dans l'axe du deplacement
    image.fill_rect(3, 6, 6, 1, TUNIC_LIGHT)
    image.fill_rect(3, 9, 6, 1, TUNIC_DARK)
    image.fill_rect(11, 7, 3, 2, SKIN)     # bras tendu devant
    image.fill_rect(2, 8, 3, 2, LEG)       # jambes tendues derriere
    image.fill_rect(2, 6, 3, 2, LEG_DARK)
    outline(image)
    for index, length in enumerate((7, 5, 3)):  # trainee, apres le cerne
        image.fill_rect(0, 5 + index * 2, length, 1, TRAIL)
    return image


def player_sheet() -> Image:
    return horizontal_sheet(
        [
            idle_frame(0),  # 0 : repos
            idle_frame(1),  # 1 : repos, respiration
            idle_frame(2),  # 2 : repos, clignement
            run_frame(0),  # 3 a 8 : course, six phases distinctes
            run_frame(1),
            run_frame(2),
            run_frame(3),
            run_frame(4),
            run_frame(5),
            jump_frame(),  # 9 : saut
            fall_frame(),  # 10 : chute
            land_frame(0),  # 11 : reception, ecrasee
            land_frame(1),  # 12 : reception, redressee
            wallslide_frame(),  # 13 : glissade murale
            dash_frame(),  # 14 : ruee
        ]
    )


PLAYER_JSON = {
    "version": 1,
    "frameWidth": TILE,
    "frameHeight": TILE,
    "clips": {
        # Repos : la respiration est lente et le clignement bref -- obtenu en revenant sur l'image
        # immobile entre les deux, plutot qu'en compliquant le format de clip d'une duree par image.
        "idle": {"frames": [0, 1, 0, 2], "frameDuration": 0.45, "loop": True},
        "run": {"frames": [3, 4, 5, 6, 7, 8], "frameDuration": 0.08, "loop": True},
        "jump": {"frames": [9], "loop": True},
        "fall": {"frames": [10], "loop": True},
        "land": {"frames": [11, 12], "frameDuration": 0.08, "loop": False, "next": "idle"},
        "wallslide": {"frames": [13], "loop": True},
        "dash": {"frames": [14], "loop": True},
    },
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
        / "Player",
        help="dossier de destination",
    )
    arguments = parser.parse_args()
    arguments.output.mkdir(parents=True, exist_ok=True)

    image = player_sheet()
    (arguments.output / "player.png").write_bytes(image.to_png())
    (arguments.output / "player.anim.json").write_text(
        json.dumps(PLAYER_JSON, indent=2, ensure_ascii=False) + "\n", encoding="utf-8"
    )
    print(f"player.png : {image.width}x{image.height} + player.anim.json")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
