# HMI/Input/

Acquisition des entrées et traduction en **actions logiques**.

- `InputState` : état clavier/souris **et manette** par frame, avec fronts **pressée / maintenue /
  relâchée** (`EX-CTRL-011`), échantillonné une fois par frame (`EX-CTRL-021`). Indépendant
  de toute fenêtre (aucun `<Windows.h>`), donc testable en isolation ; clavier et manette y sont
  **fusionnés** en lecture seule sur les mêmes `Key` (`EX-CTRL-002`).
- `GamepadPoller` : sondage XInput de la manette, fusionné dans un `InputState`.
- `GamepadButton` : bouton (ou direction) manette logique, indépendant de toute touche clavier.
- `ButtonRepeat` : répétition d'un bouton tenu — un pas à l'appui, puis un pas régulier après un
  délai.
- `EditorKeyBindings` : association remappable action d'éditeur → touche, avec persistance JSON
  (`EX-CTRL-012`).
- `QtKeyMap` : traduction d'un code `Qt::Key` en `hmi::Key` (code virtuel Win32), et l'inverse.

Réf. specs : `EX-CTRL-001`…`EX-CTRL-021`.
