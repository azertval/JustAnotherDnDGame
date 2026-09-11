#!/usr/bin/env python3
"""Static validation of the Qt Design Studio / runtime QML boundary.

The check is intentionally source-only: it can run before CMake/Qt are configured.
It validates every `.ui.qml` under Source/Ui for common Design Studio hazards, checks
local QML dependency cycles, verifies the tracked Jadg.Ui/qmldir, audits QML module
source paths, and rejects runtime dependencies from the design-only project.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
UI = ROOT / "Source" / "Ui"
MODULE = UI / "Jadg" / "Ui" / "qmldir"
DESIGN_ENTRY = UI / "DesignStudio" / "Main.ui.qml"
DESIGN_PROJECT = UI / "DesignStudio" / "JadgUiDesign.qmlproject"
QML_MODULE_CMAKES = [
    ROOT / "Source" / "Ui" / "Jadg" / "Ui" / "CMakeLists.txt",
    ROOT / "Source" / "HMI" / "Jadg" / "Runtime" / "CMakeLists.txt",
    ROOT / "Source" / "App" / "CMakeLists.txt",
]
ALLOWED_IMPORTS = {
    "QtQuick", "QtQuick.Window", "QtQuick.Layouts", "QtQuick.Controls",
    "QtQuick.Shapes", "QtQuick.Effects", "Jadg.Ui",
}
FORBIDDEN = [
    (re.compile(r"\bfunction\s+\w+\s*\("), "JavaScript function"),
    (re.compile(r"\bComponent\.onCompleted\b"), "Component.onCompleted"),
    (re.compile(r"^\s*Connections\s*\{", re.M), "Connections"),
    (re.compile(r"^\s*Timer\s*\{", re.M), "Timer"),
    (re.compile(r"\bconsole\.(log|warn|error|debug)\b"), "console call"),
    (re.compile(r"\bGameViewport\s*\{"), "runtime C++ GameViewport"),
]
IMPORT_RE = re.compile(r"^\s*import\s+([A-Za-z][\w.]*)", re.M)
TYPE_RE = re.compile(r"\b([A-Z][A-Za-z0-9_]*)\s*\{", re.M)
LOCAL_EXTENSIONS = (".qml", ".ui.qml")


def strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    return re.sub(r"//[^\n]*", "", text)


def rel(path: Path) -> str:
    return path.relative_to(ROOT).as_posix()


def qml_files() -> list[Path]:
    return sorted(UI.rglob("*.qml"))


def local_dependency_graph(files: list[Path]) -> dict[Path, set[Path]]:
    by_name: dict[str, list[Path]] = {}
    for path in files:
        by_name.setdefault(path.name, []).append(path)
    graph = {p: set() for p in files}
    for path in files:
        body = strip_comments(path.read_text(encoding="utf-8", errors="replace"))
        for type_name in TYPE_RE.findall(body):
            candidates = by_name.get(type_name + ".qml", []) + by_name.get(type_name + ".ui.qml", [])
            graph[path].update(c for c in candidates if c != path)
    return graph


def cycles(graph: dict[Path, set[Path]]) -> list[list[Path]]:
    state: dict[Path, int] = {}
    stack: list[Path] = []
    found: list[list[Path]] = []

    def visit(node: Path) -> None:
        state[node] = 1
        stack.append(node)
        for nxt in graph[node]:
            if state.get(nxt, 0) == 0:
                visit(nxt)
            elif state.get(nxt) == 1:
                i = stack.index(nxt)
                found.append(stack[i:] + [nxt])
        stack.pop()
        state[node] = 2

    for node in graph:
        if state.get(node, 0) == 0:
            visit(node)
    return found


def audit_qml_module_paths() -> list[str]:
    failures: list[str] = []
    for cmake in QML_MODULE_CMAKES:
        if not cmake.is_file():
            failures.append(f"CMake QML module manquant: {rel(cmake)}")
            continue
        text = cmake.read_text(encoding="utf-8", errors="replace")
        for match in re.finditer(r"\bQML_FILES\b(?P<body>.*?)(?:\n\s*\)|\n\s*[A-Z][A-Z_]+\b)", text, re.S):
            body = match.group("body")
            if "${PROJECT_SOURCE_DIR}" in body or "${CMAKE_SOURCE_DIR}" in body:
                failures.append(
                    f"{rel(cmake)}: QML_FILES ne doit pas utiliser un chemin absolu via "
                    "PROJECT_SOURCE_DIR/CMAKE_SOURCE_DIR; utiliser un chemin relatif + QT_RESOURCE_ALIAS"
                )
            for line in body.splitlines():
                stripped = line.strip()
                if re.match(r"^[A-Za-z]:[\\/]", stripped) or stripped.startswith("/"):
                    failures.append(f"{rel(cmake)}: chemin QML absolu interdit: {stripped}")
    return failures


def main() -> int:
    failures: list[str] = []
    forms = sorted(p for p in UI.rglob("*.ui.qml"))
    if not forms:
        failures.append("Aucun .ui.qml trouve sous Source/Ui")

    for path in forms:
        body = strip_comments(path.read_text(encoding="utf-8", errors="replace"))
        for module in IMPORT_RE.findall(body):
            if module not in ALLOWED_IMPORTS:
                failures.append(f"{rel(path)}: import non autorise: {module}")
        for pattern, name in FORBIDDEN:
            if pattern.search(body):
                failures.append(f"{rel(path)}: {name} interdit dans un .ui.qml")
        if path.name == "GameViewForm.ui.qml" and "GameViewport" in body:
            failures.append(f"{rel(path)}: GameViewport C++ doit rester dans GameView.qml")

    if not MODULE.is_file():
        failures.append("Source/Ui/Jadg/Ui/qmldir manquant")
    else:
        module_text = MODULE.read_text(encoding="utf-8", errors="replace")
        if not re.search(r"^module\s+Jadg\.Ui\s*$", module_text, re.M):
            failures.append("qmldir: URI module Jadg.Ui manquante")
        if not re.search(r"^singleton\s+Tokens\s+1\.0\s+", module_text, re.M):
            failures.append("qmldir: Tokens doit etre declare singleton")

    if not DESIGN_ENTRY.is_file():
        failures.append("DesignStudio/Main.ui.qml manquant")
    else:
        design_body = strip_comments(DESIGN_ENTRY.read_text(encoding="utf-8", errors="replace"))
        if "Jadg.Runtime" in design_body:
            failures.append("DesignStudio/Main.ui.qml ne doit jamais importer Jadg.Runtime")

    if not DESIGN_PROJECT.is_file():
        failures.append("DesignStudio/JadgUiDesign.qmlproject manquant")
    else:
        project_body = strip_comments(DESIGN_PROJECT.read_text(encoding="utf-8", errors="replace"))
        if "Jadg.Runtime" in project_body:
            failures.append("JadgUiDesign.qmlproject ne doit jamais referencer Jadg.Runtime")
        if 'mainFile: "Main.ui.qml"' not in project_body:
            failures.append("JadgUiDesign.qmlproject doit ouvrir Main.ui.qml")

    failures.extend(audit_qml_module_paths())

    graph = local_dependency_graph(qml_files())
    for cycle in cycles(graph):
        failures.append("dependance QML circulaire: " + " -> ".join(rel(p) for p in cycle))

    print(f"check_qml_designer_compat: {len(forms)} formulaire(s), {len(graph)} fichier(s) QML analyses")
    if failures:
        for failure in failures:
            print("ERROR:", failure, file=sys.stderr)
        return 1
    print("check_qml_designer_compat: OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
