# TACHE-03 — Convention d'assets + (option) aperçu ; documentation & vérification {#lot-39-tache-03-convention-assets-doc}

**Lot :** [LOT-39](epic.md) · **Emplacement :** `Source/Elements`, `Source/Editor`, `Documentation` · **Statut :** fait

## Contexte
Finalise le pipeline de textures fichiers : **convention d'assets** claire (où poser les fichiers,
comment les recharger), **aperçu** optionnel dans l'éditeur, puis **documentation et vérification** de
clôture du lot (et du programme de refonte).

## Travail à réaliser
- **Convention d'assets** : formaliser le dossier `Assets/` (structure, nommage, format PNG), sa copie
  à côté de l'exécutable (POST_BUILD, comme `Levels`/`Localization`) et son déploiement en release.
- **Rechargement à chaud** (optionnel) : une action éditeur « Recharger les textures » qui re-décode
  l'atlas sans relancer l'appli (utile au level design / itération d'assets).
- **Aperçu d'assets** (optionnel selon coût) : un dock affichant les textures chargées (vignettes).
- **Documentation** :
  - `Documentation/Guide/guide-rendu.md` : section « pipeline de textures depuis fichiers » (loader,
    atlas, repli, nearest, où déposer les assets, rechargement).
  - Déclarer `EX-REN-041`/`EX-REN-042` dans `Documentation/Specification/rendu-technique.md`.
  - Regénérer le cahier de test si nécessaire (`generate_cahier_test.py --check` vert).

## Fichiers impactés
- `Source/Elements/Assets/` (assets de base + README de convention).
- `Source/Editor/` (rechargement / aperçu optionnels).
- `Documentation/Guide/guide-rendu.md`, `Documentation/Specification/rendu-technique.md`, cahier de
  test généré.

## Tests (obligatoires)
- **Non-régression** : mapping de régions (TACHE-02) et résolution d'assets (TACHE-01) restent testés
  et verts.
- **`--check` du cahier de test** et **lint des exigences** verts.
- **Vérification manuelle** : déposer/remplacer un fichier d'asset et constater l'effet (au relancement
  et, si implémenté, au rechargement à chaud) ; asset manquant → repli sans crash.

## Points d'attention
- **Éditable hors code** : remplacer le fichier d'asset doit suffire à changer l'apparence (objectif
  du demandeur) — le documenter explicitement.
- **Rester dans le périmètre** : pas de DDS/mipmaps/packing auto, pas de décors riches (`EX-DEC-*`) —
  hors périmètre.
- **Doxygen 1.9.8** : générer la doc localement avant push.

## Définition de fait (DoD)
- Convention d'assets documentée et opérationnelle (copie + déploiement), aperçu/rechargement
  optionnels livrés si retenus ; guide rendu et exigences à jour ; tests verts (dont cahier `--check`) ;
  `/W4 /WX`, Doxygen, lint verts ; critères du [LOT-39](epic.md) satisfaits (vérification manuelle).

## Exigences
`EX-REN-041`/`EX-REN-042` ; `EX-NFR-040` (repli), `EX-NFR-010`.

## Réalisé
Convention documentée dans `Source/Elements/Assets/README.md` (l'ancien dossier réservé
`Textures/`, jamais peuplé, est plié dans `Assets/` — voir `Source/Elements/README.md`). Pas de
rechargement à chaud ni d'aperçu dock (optionnels, non retenus à ce stade — hors périmètre minimal
du lot, le besoin premier du demandeur étant de pouvoir remplacer le fichier). À la place, un outil
de développement headless régénère l'atlas de référence depuis la génération procédurale :
`JustAnotherDnDGame.exe --export-atlas=<chemin>.png` (`Source/HMI/main.cpp`), documenté dans
`Documentation/Guide/guide-rendu.md` (section « Le pipeline de textures depuis fichiers, et son
repli procédural »). `scripts/lint_exigences.py` et `scripts/generate_cahier_test.py --check`
verts ; build `/W4 /WX` propre (`JustAnotherDnDGame`, `UnitTests`, `IntegrationTests`, `SystemTests`) ;
385+81+3 tests verts ; chargement fichier et repli procédural vérifiés manuellement (device D3D11
WARP headless, sans fenêtre).
