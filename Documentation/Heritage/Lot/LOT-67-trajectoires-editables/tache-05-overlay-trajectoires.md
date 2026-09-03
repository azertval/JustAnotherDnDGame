# TACHE-05 — Overlay d'edition des trajectoires {#lot-67-tache-05-overlay-trajectoires}

**Lot :** [LOT-67](epic.md) · **Emplacement :** `Source/HMI/Graphics` · **Statut :** fait

## Contexte
`composeMovingPlatformPaths` tracait un unique segment entre deux points, avec une pointe de
fleche. La route multi-points le perime, et rien ne materialisait la course d'un danger mobile ni
les poignees manipulables.

## Travail a realiser
- Generaliser le trace au polyligne, une pointe par segment : elle donne le **sens** de parcours,
  seul moyen de distinguer un circuit ferme a l'oeil.
- Composer les poignees du parcours **selectionne uniquement** — les afficher sur tous saturerait le
  canevas — en reutilisant le double ton sombre/clair des poignees de decors, lisible sur tout fond.
- Appliquer l'apercu du geste sur une **copie locale** de la configuration : le brouillon n'est
  touche qu'au relachement.
- Ajouter `composeDangerMoverPaths` : trait rouge-orange (famille danger, distincte de l'azur des
  plateformes et du bleu/orange des liens), avec une pointe a **chaque** bout — la course est un
  aller-retour, pas un sens unique.

## Fichiers impactes
`Source/HMI/Graphics/DraftRenderer.{h,cpp}`, `Source/HMI/Game/GameViewport.cpp`.

## Tests (obligatoires)
La geometrie composee est testee **en amont**, dans `test_platform_path.cpp` et
`test_path_geometry.cpp` : ce sont les primitives pures dont l'overlay se contente de tracer le
resultat.

`hmi::DraftRenderer` exige un `ID3D11Device` (via `SpriteBatch` et `TextureAtlas`) : il n'est
instancie par **aucun** test du depot, et ne peut pas l'etre sans GPU. Le rendu lui-meme releve donc
de la verification IHM manuelle, conformement a la convention du projet pour les lots de rendu.

## Points d'attention
L'ordre de dessin des poignees de parcours reutilise celui des poignees de decors : les deux outils
ne sont jamais actifs en meme temps, aucune superposition n'est possible.

## Definition de fait (DoD)
Le trajet affiche correspond au trajet parcouru (meme primitive). Verification manuelle : poignees
visibles a la selection, apercu suivant le curseur, trajet de danger mobile distinct.

## Exigences
`EX-EDIT-032`, `EX-EDIT-030`, `EX-GP-051`, `EX-GP-054`.
