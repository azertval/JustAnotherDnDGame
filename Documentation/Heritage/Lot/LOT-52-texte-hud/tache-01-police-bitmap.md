# TACHE-01 — Police bitmap : atlas de glyphes et métriques {#lot-52-tache-01-police-bitmap}

**Lot :** [LOT-52](epic.md) · **Emplacement :** `Source/HMI/Graphics` · **Statut :** fait

## Contexte
Le projet a déjà eu une police bitmap : *hmi::BitmapFont*, retirée au `LOT-38` (commit `8338bc15`)
avec toute la pile d'UI « maison », Qt reprenant l'affichage du texte hors-jeu. Le raisonnement
était juste ; sa conséquence — plus aucun texte possible **dans la scène** — n'a jamais été traitée.

L'implémentation retirée reste consultable dans l'historique Git et constitue un point de départ,
à condition d'être rebranchée sur le *TextureCache* et les calques de LOT-40 plutôt que sur
l'ancien chemin.

## Travail à réaliser
- **Atlas de glyphes** : un PNG (`Assets/Fonts/`) accompagné d'un fichier de **métriques** décrivant,
  pour chaque caractère couvert, sa région dans l'atlas et son avance horizontale. Chargé par le
  *TextureCache* (LOT-40) et validé par le contrat d'asset (`EX-REN-007`).
- **Jeu de caractères** : au minimum ASCII imprimable **et** les caractères accentués utilisés par le
  catalogue français (`é è à ç ù ê î ô û`). Une police qui ne sait pas écrire « Épuisé » est
  inutilisable ici.
- **Repli procédural** : si l'atlas est absent, générer une police minimale en mémoire, sur le modèle
  de `hmi::buildProceduralAtlasImage` (`EX-REN-042`). Le jeu doit rester lisible sans aucun asset.
- **Mesure de texte** : largeur et hauteur d'une chaîne à partir des seules métriques — fonction
  **pure**, sans GPU, indispensable pour cadrer un affichage sans le dessiner.
- **Caractère absent** : substitution déterministe (par un glyphe de remplacement), jamais un
  plantage ni un trou silencieux.

## Fichiers impactés
- `Source/HMI/Graphics/BitmapFont.{h,cpp}` (nouveau, à rapprocher de la version retirée au LOT-38).
- `Source/HMI/Graphics/ProceduralFont.{h,cpp}` (nouveau) — repli.
- `Source/Elements/Assets/Fonts/` (nouveau dossier), `Source/HMI/CMakeLists.txt`.
- `Source/Test/Unit/HMI/Graphics/test_bitmap_font.cpp` (nouveau).

## Tests (obligatoires)
- **Mesure** : chaîne vide, un caractère, chaîne accentuée, chaîne contenant un caractère absent —
  largeurs attendues, fonction pure.
- Lecture des métriques : fichier valide, invalide, incohérent avec les dimensions du PNG.
- Absence d'atlas → repli procédural, sans exception.
- Sans GPU.

## Points d'attention
- **Les accents ne sont pas optionnels.** Le français est la langue par défaut du projet
  (`EX-REN-033`) ; omettre les caractères accentués rendrait le HUD illisible et la faute serait
  découverte tard.
- L'encodage des chaînes est **UTF-8** (comme les catalogues de langue) : un caractère accentué
  occupe plusieurs octets. La mesure et le rendu doivent parcourir des points de code, pas des
  octets — c'est le piège classique.
- Ne pas réintroduire l'ancien chemin de rendu du LOT-38 : la police est un asset comme un autre,
  elle passe par le *TextureCache*.

## Définition de fait (DoD)
- Un atlas de glyphes est chargé et validé, avec repli procédural ; la mesure de texte est pure et
  testée, y compris sur des chaînes accentuées ; un caractère absent est substitué ;
  `/W4 /WX` propre.

## Exigences
`EX-REN-032` (affichage de texte) ; réutilise `EX-REN-042` (assets externalisés avec repli),
`EX-REN-007` (contrat d'asset), `EX-REN-033` (traduction, français par défaut), `EX-ARCH-022`
(*nearest*), `EX-NFR-040` (repli).
