# TACHE-07 — Palettes persistées et mode contraint {#lot-54-tache-07-palettes}

**Lot :** [LOT-54](epic.md) · **Emplacement :** `Source/HMI/Editor`, `Source/Elements` · **Statut :** fait

## Contexte
Une palette de couleurs choisie à la main dans un sélecteur, perdue à la fermeture de la fenêtre, ne
sert à rien au-delà d'une séance : la cohérence chromatique d'un jeu de tuiles se construit sur des
semaines, entre plusieurs assets. C'est la raison pour laquelle la palette du cadrage initial — une
simple liste de couleurs courantes dans le canevas — était le maillon faible du périmètre.

Une palette **persistée** répond au problème réel : les tuiles d'un même jeu de skins doivent partager
leurs couleurs, faute de quoi l'assemblage se voit. Et une fois la palette écrite quelque part, la
contrainte devient possible : interdire de poser une couleur qui n'y figure pas est la discipline
habituelle du pixel art, et c'est bien plus efficace que de la respecter à l'œil.

Le projet a déjà tout ce qu'il faut pour la partie fichier : les assets sont décrits par des fichiers
JSON à côté d'eux (`skins.json`, `*.anim.json`), lus avec la même bibliothèque et la même politique
d'erreurs récupérables.

## Travail à réaliser
- **Palette de projet** : un fichier JSON dans le dossier d'assets, listant des couleurs nommées.
  Lecture et écriture selon la politique du module — **aucune exception**, résultat porteur d'un code
  exploitable (`EX-NFR-040`), fichier absent traité comme une palette vide et non comme une erreur.
- **Édition de la palette** depuis l'atelier : ajouter la couleur courante, retirer une couleur,
  renommer une entrée, réordonner.
- **Extraction depuis un asset ouvert** : produire la liste des couleurs effectivement présentes dans
  l'image, dans un ordre **déterministe**, avec leur nombre d'occurrences — pour amorcer une palette
  à partir d'un asset existant plutôt que de la saisir.
- **Mode « contraindre à la palette »** : quand il est actif, toute couleur posée est ramenée à
  l'entrée la plus proche de la palette. La distance et la règle de départage sont **déterministes**
  et documentées ; l'alpha nul reste toujours accessible (c'est la gomme, pas une couleur).
- **Indication à l'écran** : l'état du mode contraint et la couleur courante figurent dans la barre
  d'état, via le contexte d'atelier de la TACHE-04.
- **Couleurs de l'interface de palette issues des jetons** ; seules les pastilles montrent les
  couleurs de la palette elle-même.
- **Traduction** de tous les libellés dans les deux catalogues.

## Fichiers impactés
- `Source/HMI/Editor/PixelPalette.{h,cpp}` (nouveau) — modèle de palette, extraction, plus proche
  couleur, chargement et enregistrement.
- `Source/HMI/Editor/PixelPalettePanel.{h,cpp}` (nouveau) — panneau d'édition de la palette.
- `Source/Elements/Assets/palettes.json` (nouveau) — palette de projet livrée par défaut.
- `Source/HMI/Editor/EditorStatus.{h,cpp}` (créé en `LOT-57`) — couleur courante et mode contraint.
- `Source/Elements/Localization/fr.lang`, `en.lang`.
- `Source/Test/Unit/HMI/Editor/test_pixel_palette.cpp` (nouveau).

## Tests (obligatoires)
- **Aller-retour JSON** : écrire puis relire une palette restitue les mêmes couleurs, les mêmes noms
  et le même ordre, alpha compris.
- **Fichier absent ou illisible** : résultat exploitable, palette vide, aucune exception.
- **Extraction déterministe** : deux extractions du même asset produisent la même liste dans le même
  ordre ; les occurrences comptées sont exactes ; une image entièrement transparente produit une
  liste vide.
- **Plus proche couleur** : une couleur déjà présente dans la palette est renvoyée telle quelle ;
  l'égalité de distance est départagée de façon stable ; une palette vide laisse la couleur
  inchangée plutôt que de produire une couleur indéfinie.
- **Alpha préservé** : la contrainte ne transforme jamais un pixel transparent en pixel opaque.
- Logique **pure**, testée sans Qt ni GPU.

## Points d'attention
- **Le départage doit être stable**, sinon deux exécutions du même geste sur la même image donnent
  des résultats différents et l'aller-retour d'un asset devient non reproductible.
- Une palette est une donnée d'**auteur**, pas un réglage d'affichage : elle vit dans le dossier
  d'assets avec le contenu qu'elle décrit, et non dans le fichier de configuration du poste.
- Ne pas confondre cette palette avec les **jetons** de [LOT-56](@ref lot-56) : les jetons habillent
  l'application, cette palette habille le contenu du jeu. Les deux ne se mélangent jamais, et c'est
  la même frontière que celle entre l'identité du jeu et le châssis d'édition.
- Le mode contraint doit rester **désactivable à tout moment** : imposé de force, il empêcherait
  d'introduire délibérément une couleur nouvelle, qui est le début de toute palette.

## Définition de fait (DoD)
- Une palette de projet est lue, éditée et enregistrée sans exception ; l'extraction depuis un asset
  est déterministe ; le mode contraint ramène toute couleur posée à la plus proche entrée de façon
  stable et préserve la transparence ; la couleur courante et le mode figurent dans la barre d'état ;
  chaînes traduites ; `/W4 /WX` propre.

## Exigences
`EX-EDIT-045` (outil de dessin pixel art) ; réutilise `EX-REN-042` (assets externalisés),
`EX-IHM-060` (état de travail affiché en permanence), `EX-IHM-051` (source unique des grandeurs),
`EX-REN-033` (traduction), `EX-NFR-040` (erreur récupérable), `EX-NFR-010` (testable sans GPU).
