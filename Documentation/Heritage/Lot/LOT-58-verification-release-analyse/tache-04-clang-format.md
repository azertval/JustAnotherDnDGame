# TACHE-04 — Vérification automatique du formatage {#lot-58-tache-04-clang-format}

**Lot :** [LOT-58](epic.md) · **Emplacement :** `.github/workflows` · **Statut :** fait

## Contexte
`CONTRIBUTING.md` ouvre sur « Le code doit être formaté (`clang-format`) […] avant tout commit », et
le répète dans la liste des vérifications avant PR. `conventions.md` décrit `.clang-format` comme
appliqué automatiquement par Visual Studio. Aucun contrôle ne le vérifie : la règle tient
uniquement parce que l'IDE d'un contributeur unique la respecte.

C'est la moins grave des trois vérifications manquantes de ce lot, et la moins chère à combler.

## Travail à réaliser
- **Job `format`** dans `ci.yml`, sur Linux : `clang-format --dry-run --Werror` sur les fichiers
  `.cpp`/`.h` de `Source/`, hors `External/`.
- **Épingler la version de `clang-format`**, exactement comme `DOXYGEN_VERSION` l'est déjà pour
  Doxygen, et pour la même raison : deux versions majeures ne formatent pas identiquement, et un
  contrôle qui ne prédit pas le résultat local ne sert qu'à agacer.
- **Documenter la commande locale** dans `CONTRIBUTING.md`, pour que la vérification soit
  reproductible avant de pousser.
- **Contrôle, jamais réécriture** : le job signale, il ne pousse pas de commit de reformatage.

## Fichiers impactés
- `.github/workflows/ci.yml` — nouveau job et variable de version épinglée.
- `CONTRIBUTING.md` — commande locale.
- Sources reformatées si l'état actuel s'écarte du fichier de style.

## Tests (obligatoires)
- Le job passe sur l'état de `main` après un éventuel reformatage initial.
- **Test négatif** : une indentation volontairement fausse fait échouer le job.
- La commande documentée dans `CONTRIBUTING.md`, exécutée localement avec la version épinglée,
  donne le même verdict que la CI.

## Points d'attention
- **Un reformatage initial du dépôt entier est possible** : s'il a lieu, le faire dans un commit
  **séparé et unique**, sans aucun changement de comportement, pour ne pas polluer l'historique des
  lots ni les futurs `git blame`.
- La version de `clang-format` de l'image Ubuntu change avec l'image : c'est précisément le piège
  déjà rencontré avec Doxygen (`DOXYGEN_VERSION` est épinglée pour cette raison, commentée dans
  `ci.yml`). Ne pas répéter l'erreur.
- Les fichiers générés par Qt (`moc_*`, `ui_*`, `qrc_*`) ne sont pas du code du projet : les
  exclure explicitement.

## Définition de fait (DoD)
- Le formatage est vérifié sur chaque PR avec une version épinglée, un écart est démontré refusé, la
  commande locale équivalente est documentée, et l'éventuel reformatage initial est isolé dans son
  propre commit.
- Vérifié localement (LOT-58) : `clang-format 18.1.8` (paquet PyPI, binaires officiels LLVM — le
  `clang-format` fourni par cette installation de Visual Studio est une édition preview LLVM 22.1.3
  qui ne prédit pas le verdict de la CI). 192 fichiers `Source/*.cpp,*.h` s'écartaient du style :
  reformatés dans un commit séparé et unique, `ctest --preset ninja` confirmé à 100 % (943 tests)
  après coup. Test négatif (indentation et espacement volontairement faux dans un constructeur)
  fait échouer `clang-format --dry-run --Werror`, puis retiré.

## Exigences
`EX-NFR-024` (formatage vérifié automatiquement) ; réutilise `EX-NFR-012` (conventions),
`EX-NFR-022` (CI), `EX-NFR-031` (dépendances et outils épinglés).
