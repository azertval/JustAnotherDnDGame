# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later
"""Garde-fou : la separation conception / code ne doit pas rederiver (LOT-86).

POURQUOI CE CONTROLE EXISTE
---------------------------
Le LOT-86 separe ce qu'un artiste modifie (Source/Ui, en QML declaratif) de ce qu'un developpeur
ecrit (Source/HMI/Presentation, en C++). Cette separation ne tient pas toute seule : elle se perd
par de petits gestes raisonnables, chacun defendable pris isolement -- un include de Quick pour
« juste » lire une propriete, une couleur ecrite en dur « le temps d'essayer », un gestionnaire
imperatif glisse dans un formulaire.

Le depot a deja paye ce prix. Le defaut du plancher de taille des ecrans s'est produit TROIS FOIS,
et le guide en a tire la lecon lui-meme : « une regle qu'il faut se rappeler d'appliquer se reperd
au premier ecran ajoute ». La palette d'identite, elle, a ete ecrite DEUX FOIS pendant des mois,
sans que rien ne relie les copies.

Une regle qui n'est pas verifiee n'est pas une regle : c'est une intention.

CE QUI EST VERIFIE
------------------
1. Presentation ne connait ni Qt Quick ni Qt Widgets.
2. Core ne connait pas Qt du tout.
3. La cible du JEU ne lie pas Qt6::Widgets.
4. Les ecrans et controles sont des `.ui.qml`, et n'y contiennent aucun code imperatif.
5. Les `.ui.qml` n'importent que des modules connus A LA FOIS de Qt et de Qt Design Studio.
6. Aucune couleur, police ou taille en dur hors de Source/Ui/Theme.

Controle purement textuel : aucun binaire a construire, aucune dependance. Meme motif que
`check_qt_version_pin.py`. Il s'auto-verifie contre la vacuite -- un releve vide est un ECHEC, et
non un succes : c'est ainsi que le LOT-78 avait cru un controle vert alors qu'il ne lisait rien.

Usage :
    python scripts/check_ui_layers.py     # code de sortie non nul si une regle est enfreinte
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

PRESENTATION = ROOT / "Source" / "HMI" / "Presentation"
CORE = ROOT / "Source" / "Core"
UI = ROOT / "Source" / "Ui"
UI_THEME = UI / "Theme"
HMI_CMAKE = ROOT / "Source" / "HMI" / "CMakeLists.txt"

# Repertoires dont TOUT fichier visuel appartient a la conception.
DESIGN_DIRECTORIES = (UI / "Screens", UI / "Controls")

# Modules importables depuis un formulaire. L'intersection est la contrainte : un module present
# dans Qt Design Studio mais absent de l'installation Qt (QtQuick.Studio.Components, livre avec le
# designer) s'ouvrirait chez l'artiste et casserait le jeu -- a la construction pour un module
# inconnu, a l'execution pour un type absent.
ALLOWED_QML_IMPORTS = {
    "QtQuick",
    "QtQuick.Window",
    "QtQuick.Layouts",
    "QtQuick.Controls",
    "QtQuick.Shapes",
    "QtQuick.Effects",
    "Jadg.Ui",
}

# Constructions imperatives interdites dans un `.ui.qml`. Qt Design Studio relit et REENREGISTRE
# ces fichiers : ce qu'il ne comprend pas, il le perd -- sans avertir. La liste ne vise donc pas
# le style, mais ce que l'outil detruirait.
IMPERATIVE_PATTERNS = (
    (re.compile(r"\bfunction\b"), "une fonction JavaScript"),
    (re.compile(r"\bComponent\.onCompleted\b"), "un Component.onCompleted"),
    (re.compile(r"^\s*Connections\s*\{", re.MULTILINE), "un bloc Connections"),
    (re.compile(r"\bconsole\.(log|warn|error|debug)\b"), "un appel a console"),
    (re.compile(r"\bQt\.callLater\b"), "un Qt.callLater"),
    (re.compile(r"^\s*Timer\s*\{", re.MULTILINE), "un Timer"),
)

# Litteraux d'apparence : ils doivent venir de Tokens.qml, jamais d'un ecran. Une couleur ecrite
# ici survit a un changement de palette et devient un mensonge silencieux (EX-IHM-051).
COLOR_LITERAL = re.compile(r':\s*"#[0-9a-fA-F]{3,8}"')
PIXEL_SIZE_LITERAL = re.compile(r"\bfont\.pixelSize\s*:\s*\d")
FONT_FAMILY_LITERAL = re.compile(r'\bfont\.family\s*:\s*"')

IMPORT_LINE = re.compile(r"^\s*import\s+([A-Za-z][\w.]*)", re.MULTILINE)
# Un commentaire de fin de ligne ne doit pas declencher les regles : on les retire avant analyse.
LINE_COMMENT = re.compile(r"//[^\n]*")
BLOCK_COMMENT = re.compile(r"/\*.*?\*/", re.DOTALL)


def strip_comments(text: str) -> str:
    """Retire commentaires de bloc et de ligne : leur contenu explique souvent la regle elle-meme,
    et le controle se declencherait sur sa propre justification."""
    return LINE_COMMENT.sub("", BLOCK_COMMENT.sub("", text))


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="replace")


def sources(directory: Path, suffixes: tuple[str, ...]) -> list[Path]:
    if not directory.is_dir():
        return []
    return sorted(p for p in directory.rglob("*") if p.is_file() and p.suffix in suffixes)


def relative(path: Path) -> str:
    return path.relative_to(ROOT).as_posix()


def check_presentation_has_no_ui_toolkit(failures: list[str]) -> int:
    """Regle 1 : Presentation transforme l'etat du jeu en donnees affichables. Elle ne dessine
    rien. Un include de Quick ou de Widgets y signale que la logique de vue a commence a
    redescendre dans la couche de donnees -- exactement ce que le lot separe."""
    files = sources(PRESENTATION, (".h", ".cpp"))
    forbidden = re.compile(r'#\s*include\s*[<"](QtQuick|QtWidgets|QQuick|QWidget)')
    for path in files:
        for number, line in enumerate(read(path).splitlines(), start=1):
            if forbidden.search(line):
                failures.append(
                    f"{relative(path)}:{number} : Presentation inclut une bibliotheque d'IHM "
                    f"({line.strip()}). Elle ne doit connaitre ni Qt Quick ni Qt Widgets."
                )
    return len(files)


def check_core_has_no_qt(failures: list[str]) -> int:
    """Regle 2 : `EX-ARCH-001` et `EX-NFR-010` exigent que Core soit testable sans fenetre ni GPU.
    C'est vrai aujourd'hui -- ce controle le VERROUILLE, parce qu'un seul QString suffirait a le
    rendre faux sans que rien d'autre ne le signale."""
    files = sources(CORE, (".h", ".cpp"))
    forbidden = re.compile(r'#\s*include\s*[<"]Q[A-Za-z]')
    for path in files:
        for number, line in enumerate(read(path).splitlines(), start=1):
            if forbidden.search(line):
                failures.append(
                    f"{relative(path)}:{number} : Core inclut un en-tete Qt ({line.strip()}). "
                    f"Core doit rester testable sans fenetre ni GPU (EX-ARCH-001, EX-NFR-010)."
                )
    return len(files)


def check_game_does_not_link_widgets(failures: list[str]) -> int:
    """Regle 3 : la garantie la plus forte du lot, parce qu'elle n'est pas une convention -- si le
    jeu liait Qt6::Widgets, un widget pourrait y reapparaitre. Ne pas le lier rend la chose
    impossible : l'edition de liens echouerait."""
    if not HMI_CMAKE.is_file():
        failures.append(f"{relative(HMI_CMAKE)} introuvable : la regle 3 ne peut pas etre verifiee.")
        return 0
    text = read(HMI_CMAKE)
    match = re.search(
        r"target_link_libraries\(JustAnotherDnDGame\s+PRIVATE(.*?)\)", text, re.DOTALL
    )
    if match is None:
        failures.append(
            f"{relative(HMI_CMAKE)} : bloc target_link_libraries(JustAnotherDnDGame PRIVATE …) "
            f"introuvable. La regle 3 ne peut pas etre verifiee."
        )
        return 0
    if "Qt6::Widgets" in strip_comments(match.group(1)):
        failures.append(
            f"{relative(HMI_CMAKE)} : la cible du JEU lie Qt6::Widgets. Le jeu est en Qt Quick ; "
            f"les widgets n'appartiennent qu'a LevelEditor (EX-IHM-041)."
        )
    return 1


def check_design_files_are_forms(failures: list[str]) -> int:
    """Regle 4 : un ecran qui n'est pas un `.ui.qml` n'est pas ouvrable dans Qt Design Studio, et
    un `.ui.qml` qui contient du code imperatif s'y fait DETRUIRE en silence a l'enregistrement.
    Les deux moities de la regle protegent la meme chose : que l'artiste puisse travailler."""
    checked = 0
    for directory in DESIGN_DIRECTORIES:
        for path in sources(directory, (".qml",)):
            checked += 1
            name = path.name
            is_form = name.endswith(".ui.qml")
            # Un ecran a le droit d'avoir un jumeau de cablage `X.qml` a cote de `XForm.ui.qml` :
            # c'est la convention de Design Studio, et c'est la que le code doit vivre.
            has_twin_form = (path.parent / f"{path.stem}Form.ui.qml").is_file()
            if not is_form and not has_twin_form:
                failures.append(
                    f"{relative(path)} : ni formulaire `.ui.qml`, ni jumeau de cablage d'un "
                    f"`{path.stem}Form.ui.qml`. La conception ne pourra pas l'ouvrir."
                )
                continue
            if not is_form:
                continue
            body = strip_comments(read(path))
            for pattern, what in IMPERATIVE_PATTERNS:
                if pattern.search(body):
                    failures.append(
                        f"{relative(path)} : un formulaire contient {what}. Qt Design Studio le "
                        f"perdrait a l'enregistrement -- la logique va dans le jumeau `.qml`."
                    )
    return checked


def check_form_imports(failures: list[str]) -> int:
    """Regle 5 : Qt Design Studio livre ses propres modules (QtQuick.Studio.*), absents d'une
    installation Qt ordinaire. Un formulaire qui en importerait s'ouvrirait parfaitement chez
    l'artiste et casserait le jeu -- le pire des deux mondes, decouvert le plus tard possible."""
    checked = 0
    for directory in DESIGN_DIRECTORIES:
        for path in sources(directory, (".qml",)):
            if not path.name.endswith(".ui.qml"):
                continue
            checked += 1
            for module in IMPORT_LINE.findall(strip_comments(read(path))):
                if module not in ALLOWED_QML_IMPORTS:
                    failures.append(
                        f"{relative(path)} : import de « {module} », hors de la liste connue a la "
                        f"fois de Qt et de Qt Design Studio. Autorises : "
                        f"{', '.join(sorted(ALLOWED_QML_IMPORTS))}."
                    )
    return checked


def check_no_appearance_literals(failures: list[str]) -> int:
    """Regle 6 : ce qui donne son sens a Tokens.qml. Une couleur, une police ou une taille ecrite
    dans un ecran survit a un changement de palette : elle ne suit plus rien, et personne ne
    remarque qu'un seul ecran a cesse de ressembler aux autres (EX-IHM-051)."""
    checked = 0
    for path in sources(UI, (".qml",)):
        if UI_THEME in path.parents:
            continue  # Le theme EST l'endroit ou ces valeurs s'ecrivent.
        checked += 1
        for number, line in enumerate(strip_comments(read(path)).splitlines(), start=1):
            if COLOR_LITERAL.search(line):
                failures.append(
                    f"{relative(path)}:{number} : couleur ecrite en dur ({line.strip()}). "
                    f"Les couleurs viennent de Source/Ui/Theme/Tokens.qml (EX-IHM-051)."
                )
            if PIXEL_SIZE_LITERAL.search(line):
                failures.append(
                    f"{relative(path)}:{number} : taille de police ecrite en dur "
                    f"({line.strip()}). Elle vient de l'echelle de Tokens.qml."
                )
            if FONT_FAMILY_LITERAL.search(line):
                failures.append(
                    f"{relative(path)}:{number} : famille de police ecrite en dur "
                    f"({line.strip()}). Elle vient de Tokens.qml."
                )
    return checked


def main() -> int:
    failures: list[str] = []

    counts = {
        "1. Presentation sans Quick ni Widgets": check_presentation_has_no_ui_toolkit(failures),
        "2. Core sans Qt": check_core_has_no_qt(failures),
        "3. le jeu ne lie pas Widgets": check_game_does_not_link_widgets(failures),
        "4. ecrans et controles en .ui.qml, sans imperatif": check_design_files_are_forms(failures),
        "5. imports connus de Qt et de Design Studio": check_form_imports(failures),
        "6. aucun litteral d'apparence hors du theme": check_no_appearance_literals(failures),
    }

    # Auto-verification contre la vacuite. Un controle qui ne lit RIEN passe au vert et ne prouve
    # rien : c'est la panne du LOT-78, et elle ne se voit pas dans un journal de CI.
    empty = [name for name, count in counts.items() if count == 0]
    if empty:
        print("check_ui_layers : ECHEC -- ces regles n'ont rien eu a lire :", file=sys.stderr)
        for name in empty:
            print(f"  - {name}", file=sys.stderr)
        print(
            "  Un controle vert par vacuite ne prouve rien. Verifier les chemins ci-dessus.",
            file=sys.stderr,
        )
        return 1

    if failures:
        print(f"check_ui_layers : {len(failures)} manquement(s).\n", file=sys.stderr)
        for failure in failures:
            print(f"  {failure}", file=sys.stderr)
        return 1

    total = sum(counts.values())
    print(f"check_ui_layers : OK ({total} element(s) verifie(s), 6 regles).")
    for name, count in counts.items():
        print(f"  {name} : {count}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
