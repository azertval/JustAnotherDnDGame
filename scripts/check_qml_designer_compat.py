#!/usr/bin/env python3
"""Static validation of the Qt Design Studio / runtime QML boundary.

The check is intentionally source-only: it can run before CMake/Qt are configured.
It validates every `.ui.qml` under Source/Ui for common Design Studio hazards, checks
local QML dependency cycles, verifies the tracked Jadg.Ui/qmldir, and rejects the
runtime-only C++ GameViewport type from designer forms.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
UI = ROOT / "Source" / "Ui"
MODULE = UI / "Jadg" / "Ui" / "qmldir"
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
