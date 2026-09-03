# LOT-47 — États visuels des mécanismes {#lot-47}

> Statut : **fait**. Prérequis : [LOT-45](@ref lot-45) (texture par instance),
> [LOT-46](@ref lot-46) (moteur d'animation).

## Objectif
Faire en sorte qu'une porte s'ouvre, qu'un interrupteur bascule, qu'une plaque s'enfonce et qu'un
piège s'arme **visiblement**, au lieu de changer d'opacité.

Aujourd'hui, le retour d'état des mécanismes passe entièrement par la teinte : `GameSession`
applique `tint.a = 0.25f` à une porte ouverte, `INACTIVE_ALPHA = 0.35f` / `ACTIVE_ALPHA = 1.0f` à un
danger commuté ou clignotant. C'était un placeholder assumé du rendu en couleurs plates. Habiller le
jeu de vraies textures sans corriger cela produirait des portes texturées à demi transparentes — un
résultat pire que l'état actuel, parce que l'incohérence deviendrait visible.

## Périmètre

### Inclus
- **Correspondance état logique → clip**, lue **sans muter** la simulation (`EX-ARCH-012`), à partir
  des accesseurs existants : `core::MechanismController::isDoorOpen`, `isDangerActive`, l'état des
  interrupteurs et des plaques de pression, `core::DangerController::isBlinkActive` et la position
  de `DangerMover`.
- **Transitions jouées une fois** : `closed → opening → open` et `open → closing → closed`, en
  s'appuyant sur les clips `OneShot` de LOT-46. Une porte qui s'ouvre passe par son animation
  d'ouverture avant d'atteindre son état ouvert.
- **Suppression des modulations d'opacité** dans `GameSession` : `refreshDoorVisuals`,
  `refreshDangerStateVisuals` et la part visuelle de `refreshBlockVisuals` sont remplacées par le
  choix d'un clip. La teinte redevient ce qu'elle doit être — un ajustement colorimétrique, pas un
  canal d'information.
- **Repli explicite** : un mécanisme dont l'asset ne déclare pas le clip attendu conserve un rendu
  **lisible** (image fixe du clip disponible) et journalise l'état et le clip manquants, plutôt que
  de disparaître ou de rester bloqué sur une image arbitraire.
- **Section « Animations » du panneau « Textures »** (LOT-42) : associer, pour chaque famille de
  mécanisme, l'asset animé utilisé par défaut. Pas de panneau séparé.

### Exclus (hors périmètre de ce lot)
- Le personnage (LOT-48) et les décors (LOT-49).
- Toute modification du comportement des mécanismes : ce lot est **strictement visuel**. La durée
  d'une animation d'ouverture n'affecte **pas** le moment où la porte cesse d'être solide — le
  contraire introduirait une divergence entre ce que le joueur voit et ce que la simulation applique.
  Cette divergence est assumée et documentée.
- Effets sonores.
- Édition des images d'animation (LOT-54).

## Décisions de cadrage
- **Le visuel suit la simulation, jamais l'inverse.** La transition d'ouverture est purement
  décorative ; la collision bascule à l'instant où `MechanismController` le décide. Synchroniser la
  physique sur la durée d'une animation reviendrait à laisser un fichier d'asset modifier le
  gameplay — inacceptable au regard de `EX-ARCH-012` et du déterminisme (`EX-NFR-002`).
- **Correspondance côté `HMI`, pas dans `Core`** : `Core` expose des états ; le choix du clip est une
  décision de présentation. `core::Level` ne gagne aucun champ.
- **Repli lisible plutôt que repli visible** : à la différence d'une texture manquante (damier
  magenta, LOT-40), un **clip** manquant ne justifie pas de masquer l'objet — la case existe et a un
  rôle de gameplay. On dégrade vers l'image fixe et on journalise.
- **Assets par défaut par famille de mécanisme**, surchargeables par case grâce aux overrides de
  LOT-45 : la hiérarchie de résolution du programme reste unique.

## Exigences couvertes
- Nouvelle : `EX-REN-006` (apparence des mécanismes pilotée par leur état logique, transitions
  comprises).
- Réutilisées : `EX-REN-005` (animations par données), `EX-EDIT-043` (texture par instance),
  `EX-REN-046` (bascule), `EX-ARCH-012` (rendu sans effet sur la simulation), `EX-GP-020`/`EX-GP-021`
  (mécanismes), `EX-GP-052`/`EX-GP-053` (dangers commutés et temporisés), `EX-NFR-040` (repli).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-correspondance-etat-clip.md) | Correspondance état logique → clip (fonction pure, testée sans GPU) | `Source/HMI/Graphics` | ✅ |
| [TACHE-02](tache-02-transitions.md) | Transitions jouées une fois (ouverture/fermeture) + repli sur clip manquant | `Source/HMI/Graphics`, `Source/HMI/Game` | ✅ |
| [TACHE-03](tache-03-retrait-modulation-opacite.md) | Retrait des modulations d'opacité de `GameSession` | `Source/HMI/Game` | ✅ |
| [TACHE-04](tache-04-section-animations.md) | Section « Animations » du panneau « Textures » + assets par défaut | `Source/HMI/Editor` | ✅ |

## Critères d'acceptation du lot
1. Une porte, un interrupteur, une plaque de pression, un danger commuté et un danger temporisé
   changent d'apparence selon leur état, en mode Texture.
2. Une porte qui s'ouvre joue son animation d'ouverture puis reste sur son état ouvert.
3. Plus aucune modulation d'opacité ne code un état dans `GameSession`.
4. Le comportement de la simulation est **inchangé** : les tests de gameplay existants passent sans
   modification, et la collision d'une porte bascule au même pas fixe qu'avant.
5. Un asset ne déclarant pas le clip attendu reste lisible et journalise l'état et le clip manquants.
6. La correspondance état → clip est testée sans GPU ; build `/W4 /WX`, Doxygen, lint verts.

## Dépendances
Bâtit sur [LOT-46](@ref lot-46) (clips par données, clips joués une fois) et [LOT-45](@ref lot-45)
(assets par instance). Lit les contrôleurs de [LOT-12](@ref lot-12), [LOT-19](@ref lot-19) et
[LOT-31](@ref lot-31) sans les modifier. Ne modifie pas `Core`.

## Navigation des tâches
- @subpage lot-47-tache-01-correspondance-etat-clip
- @subpage lot-47-tache-02-transitions
- @subpage lot-47-tache-03-retrait-modulation-opacite
- @subpage lot-47-tache-04-section-animations
