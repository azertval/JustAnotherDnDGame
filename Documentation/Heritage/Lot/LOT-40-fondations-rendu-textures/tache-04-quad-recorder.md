# TACHE-04 — *QuadRecorder* : capture des primitives soumises {#lot-40-tache-04-quad-recorder}

**Lot :** [LOT-40](epic.md) · **Emplacement :** `Source/HMI/Graphics`, `Source/Test` · **Statut :** fait

## Contexte
Le programme d'habillage empile, lot après lot, six couches de rendu dont l'**ordre**, la
**priorité de résolution** (surcharge par case > skin > repli) et l'**isolement** en mode aperçu
sont autant de règles qui cassent silencieusement. Or tous les critères d'acceptation rédigés
jusqu'ici — « rendu pixel-identique », « identique bit à bit », « ordre de calque attendu » — sont
formulés comme des **vérifications manuelles**, dans un projet qui compte par ailleurs plus de
quatre cents cas de test et une exigence explicite de testabilité hors GPU (`EX-NFR-010`).

Tout le rendu passe par un unique goulot : `hmi::SpriteBatch::draw(SpriteQuad)` et
`draw(LineQuad)`. Capturer ce flux suffit à rendre ces règles **assertables** sans GPU, sans
capture d'écran et sans comparaison d'images.

## Travail à réaliser
- ***QuadRecorder*** (`Source/HMI/Graphics/QuadRecorder.{h,cpp}`) : collecte ordonnée des primitives
  soumises pour une image, chaque entrée conservant au minimum le *RenderLayer*, l'identité de la
  texture liée, et la primitive elle-même (`SpriteQuad`/`LineQuad`).
- **Point d'insertion** : le flux de composition de `SpriteRenderer`/`DraftRenderer` doit pouvoir
  produire sa liste de primitives **sans** device D3D11. Séparer la **composition** (calcul des
  quads, tri, regroupement — logique pure) de la **soumission** (`SpriteBatch`, GPU), la composition
  devenant la partie testable. C'est la même séparation que celle déjà pratiquée entre
  *ProceduralAtlas* (pur, testé) et `hmi::TextureAtlas` (GPU).
- **Fonctions d'assertion réutilisables** côté test : ordre des calques respecté, regroupement par
  texture contigu, absence/présence d'une primitive à une position donnée, nombre de primitives par
  calque.

## Fichiers impactés
- `Source/HMI/Graphics/QuadRecorder.{h,cpp}` (nouveau).
- `Source/HMI/Graphics/SpriteRenderer.{h,cpp}`, `Source/HMI/Graphics/DraftRenderer.{h,cpp}`
  (séparation composition / soumission).
- `Source/Test/Unit/HMI/Graphics/test_quad_recorder.cpp` (nouveau).
- `Source/Test/CMakeLists.txt` (ajout des `.cpp` purs recompilés par les tests).

## Tests (obligatoires)
- Scène de référence : ordre des calques et regroupement par texture assertés sur la liste capturée.
- Scène à deux textures et trois calques : vérifier qu'aucun quad d'un calque supérieur n'est soumis
  avant un quad d'un calque inférieur, quel que soit l'ordre d'insertion.
- Non-régression du rendu existant : la liste capturée pour une scène donnée est celle attendue
  avant modification (test de référence, à conserver pour tous les lots suivants).

## Points d'attention
- **Aucun coût en production** : le *QuadRecorder* est un outil de test et de diagnostic, pas un
  détour permanent du rendu. Il ne doit ni allouer ni copier quoi que ce soit sur le chemin normal.
- Ne pas transformer cette tâche en moteur de comparaison d'images : on assertionne des **listes de
  primitives**, pas des pixels — c'est précisément ce qui la rend possible sans GPU.
- La séparation composition/soumission ne doit modifier **aucune** signature publique de
  `SpriteBatch` (critère du lot).

## Définition de fait (DoD)
- La composition du rendu est testable sans GPU ; les critères « ordre de calque » et
  « non-régression » de ce lot et des suivants sont des assertions, plus des vérifications à l'œil ;
  `/W4 /WX` propre.

## Exigences
`EX-NFR-004` (rendu vérifiable sans GPU) ; réutilise `EX-NFR-010` (testabilité), `EX-REN-043`
(calques et multi-textures).
