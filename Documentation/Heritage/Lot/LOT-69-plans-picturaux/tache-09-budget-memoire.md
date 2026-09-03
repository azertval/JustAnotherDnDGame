# TACHE-09 — Budget de mémoire de texture {#lot-69-tache-09-budget-memoire}

**Lot :** [LOT-69](epic.md) · **Emplacement :** `Source/HMI/Graphics`, `Source/Test` ·
**Statut :** livré

## Contexte
Le budget de rendu du `LOT-62` est un plafond de **primitives par niveau livré**
(`test_render_budget.cpp`, compteurs *considérées* et *soumises*). Les plans picturaux le prennent à
contre-pied : ils ajoutent **un seul quad** chacun, mais occupent une texture à l'échelle du niveau.

Chiffré sur le plus grand niveau du dépôt (50 × 26 tuiles) :

| Densité | Dimensions | Par plan | 8 plans |
|---|---|---:|---:|
| 16 px/unité | 800 × 416 | 1,33 Mo | 10,6 Mo |
| 8 px/unité | 400 × 208 | 0,33 Mo | 2,7 Mo |
| 4 px/unité | 200 × 104 | 0,08 Mo | 0,7 Mo |

Négligeable **ici**. Mais un niveau 200 × 100 à densité native coûterait **20,5 Mo par plan** — et
seize plans, plus de 300 Mo. C'est là que ça casse, et c'est pourquoi la densité est réglable par
plan. Un plafond exprimé en primitives laisserait passer exactement cette régression.

## Travail à réaliser
- Exposer la **mémoire de texture** d'un niveau à côté des compteurs de primitives existants, dans
  la même structure de statistiques.
- Étendre `test_render_budget.cpp` d'un **second axe** : plafond de mémoire par niveau livré, au
  même endroit et selon le même patron que celui des primitives.
- Ajouter le cas de coût des plans : `N` plans → `+N` primitives soumises, `+N` passes, **0** écartée
  par le culling, et surtout **invariant en taille de niveau** — propriété qui distingue les plans
  des tuiles et mérite d'être figée.
- Alimenter l'affichage du poids en barre d'état (`TACHE-08`).

## Fichiers impactés
`Source/HMI/Graphics/{PlaneVisuals,ComposedScene}.{h,cpp}`,
`Source/Test/Unit/HMI/Graphics/test_render_budget.cpp`.

## Tests (obligatoires)
- Plafond de mémoire de texture respecté par **chaque** niveau livré.
- **Un plan supplémentaire à densité native sur un niveau au plafond fait échouer le test.** C'est
  le sens qui compte : un garde-fou qu'on n'a jamais vu refuser quoi que ce soit ne prouve rien.
- Coût des plans constant quand la taille du niveau double.

## Points d'attention
La tâche vient **après** tous ses consommateurs, comme le `LOT-62` venait après les émetteurs
d'effets dans le programme `0.1.0` : mesurer un budget avant que le contenu n'existe fige un chiffre
faux.

Le coût dominant n'est pas la mémoire mais le **batch** — chaque plan est une texture distincte,
donc une passe de plus par image, jamais écartée par le culling. Les deux axes méritent d'être
suivis ; ne pas se laisser rassurer par le tableau ci-dessus, calculé sur un petit niveau.

## Definition de fait (DoD)
Deux axes de budget testés par niveau livré, garde-fou vérifié dans le sens du refus, poids visible
dans l'éditeur. `ctest` à 100 %.

## Exigences
`EX-NFR-043`, `EX-NFR-005`, `EX-DEC-041`, `EX-DEC-044`.
