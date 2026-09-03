# TACHE-02 — Rendu des liens + création avec retour visuel immédiat dans le viewport {#lot-37-tache-02-rendu-liens-creation}

**Lot :** [LOT-37](epic.md) · **Emplacement :** `Source/HMI/Editor`, `Source/HMI/Game` · **Statut :** fait

> Réalisé dans `Source/HMI/Graphics/DraftRenderer.cpp` (`drawLinks`, appelé depuis `render`),
> `Source/HMI/Editor/LinkGesture.{h,cpp}` (machine à état pure du geste) et
> `Source/HMI/Game/GameViewport.{h,cpp}` (`handleLinkClick`). Pas d'`EditorController` séparé (code
> legacy retiré au `LOT-38`) : le geste vit directement dans `GameViewport`, comme Pinceau/
> Rectangle/Sélection. **Décision de cadrage révisée** : pas de `Maj+clic` disponible quel que soit
> l'outil — un **outil dédié** `EditorTool::Link` (panneau Outils), plus simple côté utilisateur.

## Contexte
Avec la primitive et la géométrie de la TACHE-01, cette tâche **dessine les liaisons** du niveau
comme des flèches explicites dans le viewport, et donne un **retour visuel immédiat** pendant la
création d'un lien (trait provisoire). Elle réutilise **intégralement** le modèle existant
(`LevelDraft` + `Mechanism`/`DangerLink`), remplaçant l'indication par teinte de case.

## Travail à réaliser
- **Rendu des liens** : pour chaque `Mechanism` (interrupteur/plaque → porte) et `DangerLink`
  (déclencheur → danger commuté) du brouillon, dessiner une **flèche** du centre du déclencheur au
  centre de la cible (via `LinkGeometry` + primitive TACHE-01), par-dessus la grille de tuiles.
  - Distinguer les types (couleur : mécanisme vs danger commuté) ; **surbrillance** au survol d'un
    déclencheur/cible (mettre en avant les flèches incidentes).
  - Retirer (ou réduire à un rôle d'appoint) l'ancien code de teinte de case `LINK_TINTS`.
- **Création interactive** : conserver le geste existant (sélectionner un déclencheur puis une cible ;
  ordre indifférent ; refaire la paire = délier), désormais avec :
  - un **trait provisoire** du premier point choisi vers la souris tant que la contrepartie n'est pas
    posée (case en attente signalée) ;
  - appel à `LevelDraft::linkMechanism` / `unlinkMechanism` à la validation (dispatch `Door` vs
    `DangerSwitched` déjà géré par `LevelDraft`).
- Réutiliser les prédicats existants `isTriggerTile` (`Switch`/`PressurePlate`) et `isLinkTargetTile`
  (`Door`/`DangerSwitched`).

## Fichiers impactés
- `Source/Editor/GameViewport.{h,cpp}` (rendu des liens + trait provisoire en mode édition).
- `Source/Editor/EditorController.{h,cpp}` (geste de liaison → `LevelDraft`), si ce contrôleur existe
  (LOT-35 TACHE-03).

## Tests (obligatoires)
- **Logique de geste testable** (sans GPU) : la machine « case en attente → paire → link/unlink »
  produit les bons appels `LevelDraft` (création, bascule/suppression, cas cible sans déclencheur
  valide ignoré). Réutilise `LinkGeometry` (TACHE-01) pour le placement.
- **Non-régression** : `LevelDraft::linkMechanism`/`unlinkMechanism` restent testés côté `Core` ; la
  sérialisation d'un niveau avec liens est inchangée.
- **Vérification manuelle** : créer une liaison (trait provisoire visible), la voir dessinée, la
  supprimer par bascule ; plusieurs liens lisibles simultanément.

## Points d'attention
- **Alignement au viewport** : les flèches suivent le zoom/pan de la caméra (dessin dans le viewport,
  pas overlay Qt).
- **Le modèle ne change pas** : une cible = un déclencheur ; plusieurs cibles par déclencheur possible
  (plusieurs flèches partant d'un même déclencheur — gérer la lisibilité, cf. décalage TACHE-01).
- **Dangers avancés** : la liaison déclencheur→danger commuté est couverte ; les **paramètres**
  mover/blink ne sont pas l'objet de ce lot.

## Définition de fait (DoD)
- Liaisons rendues par flèches explicites, création avec trait provisoire, suppression par bascule ;
  logique de geste **testée** ; teinte de case retirée/réduite ; `/W4 /WX` propre ; vérification
  manuelle OK.

## Exigences
`EX-IHM-030` (liens par traits/flèches) ; reconduit `EX-EDIT-003` (liaison déclencheur→cible) ;
`EX-EDIT-010` (pas de duplication de logique), `EX-NFR-010`.
