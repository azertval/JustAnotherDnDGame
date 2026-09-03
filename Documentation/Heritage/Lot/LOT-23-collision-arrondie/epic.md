# LOT-23 — Collision arrondie {#lot-23}

> Statut : **terminé**. Une variante **courbe** de la pente de `LOT-22` (`EX-GP-004`) : un quart de
> cercle plutôt qu'un plan incliné, avec le même principe de suivi de surface.

## Objectif
`EX-GP-004` demande une tuile **arrondie** offrant le même suivi de surface que la pente
(`EX-GP-003`), avec un profil **courbe**. Ce lot est délibérément séquencé **après** `LOT-22` : il
réutilise l'infrastructure de suivi de surface (fonction de hauteur par type de tuile, passe de
résolution après le balayage classique) posée par ce dernier, plutôt que d'en construire une
seconde. Le risque physique propre à ce lot est donc **beaucoup plus faible** que celui de
`LOT-22` — la partie déjà validée (isoler la pente/l'arrondi du balayage horizontal, caler la
position verticale) ne change pas de principe, seule la **formule** de hauteur change.

## Périmètre

### Inclus
- Deux nouvelles tuiles : `RoundedUpRight` et `RoundedUpLeft` — même orientation que les pentes de
  `LOT-22`, mais un profil de **quart de cercle** plutôt que linéaire.
- Réutilisation de la passe de suivi de surface de `LOT-22` : `slopeSurfaceHeight` (ou son
  équivalent renommé) devient un point d'extension géré pour les quatre types (deux pentes, deux
  arrondis).
- Chargement/écriture JSON, palette de l'éditeur, rendu (approximation du quart de cercle en
  pixel art généré en code).

### Exclus (hors périmètre de ce lot)
- **Cercle complet ou rayon variable** — un seul rayon (celui d'une case pleine), comme pour les
  pentes à 45° de `LOT-22`.
- **Arrondi en plafond** — uniquement au sol, symétrique à l'exclusion équivalente de `LOT-22`.
- **Blocs à taille fractionnaire** (`EX-GP-005`, `LOT-24`) — lot séparé, sans lien direct avec la
  forme de la surface.

## Décisions de cadrage
- **Même architecture que `LOT-22`, seule la formule change.** La fonction de hauteur par type de
  tuile (introduite en `LOT-22-TACHE-01`) gagne deux nouveaux cas ; la passe de résolution ajoutée
  en `LOT-22-TACHE-02` n'est **pas réécrite**, seulement étendue à reconnaître ces nouveaux types
  comme « suivables ». Si ce lot demandait de retoucher la passe de résolution elle-même,
  ce serait le signe que l'abstraction posée par `LOT-22` était mal choisie — à corriger en
  priorité plutôt qu'à contourner.
- **Formule du quart de cercle** : pour une case occupant `[0, 1]` en `x` et en `y` (repère local),
  `RoundedUpRight` (haut à droite, creux en bas à gauche) a pour hauteur de surface
  `h(x) = 1 - sqrt(1 - (1 - x)²)` ; `RoundedUpLeft` est le symétrique (`h(x) = 1 - sqrt(1 - x²)`).
  Ces formules sont à vérifier par un test unitaire dédié comparant quelques points remarquables
  (bords et centre) à des valeurs calculées à la main, avant de les faire confiance en jeu.

## Exigences couvertes
- `EX-GP-004` — implémentée.

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-modele-tuile-arrondie.md) | Modèle de tuile et formule de courbe | `Core/Levels` | ✅ |
| [TACHE-02](tache-02-editeur-rendu.md) | Éditeur et rendu | `HMI/Editor`, `HMI/Graphics` | ✅ |
| [TACHE-03](tache-03-documentation-verification.md) | Documentation et vérification | `Documentation` | ✅ |

## Critères d'acceptation du lot
1. Le personnage suit une tuile arrondie en marchant dessus, avec une courbe visuellement et
   physiquement distincte d'une pente linéaire.
2. **Aucune régression** sur la physique existante, y compris les pentes de `LOT-22` (mêmes tests
   de non-régression que `LOT-22`, plus les siens propres).
3. Logique nouvelle couverte par des tests (formule de courbe, suivi en jeu). Build `/W4 /WX` sans
   avertissement, Doxygen et lint des exigences verts.

## Dépendances
- **Dépend de `LOT-22`** (réutilise entièrement son infrastructure de suivi de surface) — ne doit
  pas être commencé avant que `LOT-22` soit terminé.

## Navigation des tâches
- @subpage lot-23-tache-01-modele-tuile-arrondie
- @subpage lot-23-tache-02-editeur-rendu
- @subpage lot-23-tache-03-documentation-verification
