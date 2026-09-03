# TACHE-03 — Nommage, renommage, avertissement d'écrasement {#lot-15-tache-03-nommage-renommage}

**Lot :** [LOT-15](epic.md) · **Emplacement :** `HMI/Editor`, `HMI/Interface` · **Statut :** à faire

## Contexte
Aujourd'hui, tout nouveau brouillon s'appelle `"Nouveau niveau"` (constante figée dans
`EditorScreen`) et `LevelDraft::setName()` n'est jamais appelé. Deux niveaux créés sans charger de
fichier existant enregistrent donc tous les deux dans `Nouveau niveau.json` et **s'écrasent
silencieusement** — le défaut le plus risqué relevé sur LOT-14 (`EX-EDIT-009`).

## Travail à réaliser
- **`TextInputField`** (nouveau, `Source/HMI/Editor/`) : widget de saisie de texte minimal et pur
  (pas de dépendance rendu), consommant `InputState::typedCharacters()` (TACHE-01) et
  `Key::Backspace`/`Key::Enter`/`Key::Escape` ; expose le texte courant, `confirmed()`/`cancelled()`
  une fois `Entrée`/`Échap` pressée. Le dessin (curseur, cadre) est délégué à `EditorScreen`, à
  l'identique du principe déjà suivi par `TilePalette`/`LevelPicker`.
- **Validation du nom** (fonction pure, testable, ex. `TextInputField::isValidLevelName` ou
  utilitaire dédié) : un nom valide est non vide (une fois les espaces de bord retirés) et ne
  contient aucun des caractères invalides pour un nom de fichier Windows (antislash, barre oblique,
  deux-points, astérisque, point d'interrogation, guillemet droit, chevrons ouvrant/fermant, barre
  verticale). Un nom invalide **n'est pas confirmé** : le champ reste actif et affiche le motif du refus (nom
  vide / caractère interdit), sur le même modèle que la validation de niveau (`EX-EDIT-007`).
- **Prompt à la création** : choisir « Nouveau niveau » dans `LevelPicker` ouvre un
  `TextInputField` demandant le nom avant d'entrer réellement en édition (au lieu du nom figé
  actuel). `Échap` à ce stade **annule la création** et revient au `LevelPicker` (aucun brouillon
  n'existe encore à ce moment — il n'y a rien à perdre, contrairement à un `Échap` en cours
  d'édition, cf. TACHE-02).
- **Renommage** : `F2` (TACHE-01) en cours d'édition ouvre le même widget pré-rempli du nom
  courant ; confirmer appelle `LevelDraft::setName` (déjà existant, jamais câblé) ; `Échap`
  **annule le renommage** et conserve le nom précédent inchangé (aucun effet sur le brouillon).
- **Avertissement d'écrasement** : `saveDraft()` détecte qu'un fichier `<nom>.json` existe déjà
  dans le dossier `Levels` **et** que ce n'est pas celui d'où le brouillon a été chargé (tracker le
  chemin d'origine, `std::optional<std::filesystem::path> _loadedFrom` dans `EditorScreen`) ; le
  cas échéant, affiche une confirmation avant d'écrire (réutilise le mécanisme générique de
  confirmation posé en TACHE-02). Ce contrôle porte sur le nom déjà validé ci-dessus — il ne
  revérifie pas les caractères.

## Fichiers impactés
- `Source/HMI/Editor/TextInputField.h`/`.cpp` (nouveau).
- `Source/HMI/Interface/EditorScreen.h`/`.cpp` (état de saisie, `_loadedFrom`, câblage `F2`,
  prompt post-picker avec retour au picker sur annulation, détection d'écrasement dans
  `saveDraft`).
- Tests unitaires (`Source/Test/Unit/HMI/Editor/test_text_input_field.cpp`), test système étendu.

## Tests (obligatoires)
- `TextInputField` : caractères tapés s'accumulent dans l'ordre, `Backspace` retire le dernier,
  `Entrée` confirme avec le texte courant (si valide), `Échap` annule sans modifier l'appelant.
- Validation du nom : chaîne vide ou composée uniquement d'espaces refusée ; chaîne contenant un
  antislash, une barre oblique, un deux-points, un astérisque, un point d'interrogation, un
  guillemet droit, un chevron ou une barre verticale refusée ; nom légitime (lettres, chiffres,
  espaces, accents, tiret, underscore) accepté.
- `Échap` pendant le prompt de création revient au `LevelPicker` sans créer de brouillon ; `Échap`
  pendant un renommage (`F2`) laisse `LevelDraft::name()` strictement inchangé.
- `F2` puis renommage valide change `LevelDraft::name()` et le nom de fichier utilisé au prochain
  enregistrement.
- Enregistrer sous un nom correspondant à un fichier existant **différent** du niveau chargé
  déclenche la confirmation ; enregistrer sous le nom du fichier **d'origine** du brouillon
  n'en déclenche pas (c'est une mise à jour normale, pas un écrasement accidentel).

## Points d'attention
- Le nom par défaut proposé au renommage (`F2`) est le nom courant, pas une chaîne vide — évite de
  perdre le nom par une confirmation trop rapide.
- `TextInputField` doit rester une classe de logique pure (comme `TilePalette`), testable sans
  fenêtre ni GPU (`EX-NFR-010`) : le rendu (cadre, curseur clignotant, message de refus) reste dans
  `EditorScreen`.
- La validation de caractères est volontairement une **liste noire** minimale (caractères
  strictement interdits par Windows), pas une liste blanche restrictive — un level designer non
  francophone doit pouvoir utiliser des accents/caractères Unicode dans le nom affiché.

## Définition de fait (DoD)
- Nommage à la création, renommage `F2`, avertissement d'écrasement opérationnels et testés
  (`ctest` vert) ; build `/W4 /WX` ; Doxygen à jour.

## Exigences
`EX-EDIT-009`.
