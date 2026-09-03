# TACHE-04 — Placement minimal et déploiement des assets {#lot-49-tache-04-placement-minimal}

**Lot :** [LOT-49](epic.md) · **Emplacement :** `Source/HMI/Editor`, `Source/HMI/CMakeLists.txt` · **Statut :** fait

## Contexte
Un modèle et un rendu de décors qu'on ne peut pas alimenter ne sont pas vérifiables : sans un moyen
de poser un décor, ni la superposition, ni la parallaxe, ni la traversabilité ne peuvent être
constatées dans l'application.

Cette tâche livre donc le **strict nécessaire** pour cela. L'outillage complet — déplacer,
redimensionner, pivoter, réordonner, sélectionner — est le sujet de LOT-50, et ne doit pas être
anticipé ici.

## Travail à réaliser
- **Poser et supprimer** : choisir un asset dans la bibliothèque (LOT-43), choisir une couche, poser
  au clic, supprimer au clic droit ou par la touche de suppression. Rien d'autre.
- **Passage par `LevelDraft`** : les deux opérations utilisent les mutateurs de TACHE-01 et sont donc
  annulables — c'est acquis, pas à réimplémenter.
- **Dossier `Assets/Decors/`** + copie `POST_BUILD` dans `Source/HMI/CMakeLists.txt`, sur le patron
  des copies existantes.
- **Niveau de démonstration** : ajouter ou enrichir un niveau montrant les trois couches et la
  traversabilité — un décor de premier plan devant lequel le personnage passe, un décor d'arrière-plan
  derrière les tuiles. C'est le support de la vérification manuelle du lot, et il doit rester
  cohérent avec `scripts/check_demo_sequence.py`.
- Chaînes traduites.

## Fichiers impactés
- `Source/HMI/Editor/EditorTool.h`, `ToolPanel.{h,cpp}`, `Source/Elements/UI/ToolPanel.ui`.
- `Source/HMI/Game/GameViewport.{h,cpp}` (routage des clics).
- `Source/Elements/Assets/Decors/` (nouveau dossier), `Source/HMI/CMakeLists.txt`.
- `Source/Elements/Levels/` (niveau de démonstration), `Source/Elements/Localization/*.lang`.

## Tests (obligatoires)
- Poser puis annuler retire le décor ; rétablir le repose au même endroit, sur la même couche, au
  même rang.
- **Franchissabilité** : le niveau de démonstration enrichi de décors reste franchissable — c'est le
  test qui prouve la traversabilité (`EX-NFR-021`).
- `scripts/check_demo_sequence.py` reste vert.

## Points d'attention
- **Ne pas anticiper LOT-50.** Ajouter « juste » le déplacement au glisser ferait déborder ce lot et
  viderait le suivant de sa substance ; la limite est explicite.
- Le décor posé doit l'être à la position **exacte** du clic, sans alignement sur la grille :
  c'est la démonstration de `EX-DEC-001`. L'aimantation optionnelle relève de LOT-50.
- Vérifier que le niveau de démonstration reste lisible en mode Physique (où les décors sont
  invisibles) : il doit rester franchissable sans habillage.

## Définition de fait (DoD)
- Un décor peut être posé et supprimé depuis l'éditeur, sur la couche choisie, à une position libre,
  de façon annulable ; le dossier d'assets est déployé ; un niveau de démonstration illustre les
  trois couches et reste franchissable ; `/W4 /WX` propre.

## Exigences
`EX-DEC-010` (placement de décors, partiel — complété en LOT-50), `EX-EDIT-040` (édition de décors,
partiel) ; réutilise `EX-DEC-001` (position libre), `EX-EDIT-005` (annuler/refaire), `EX-EDIT-026`
(bibliothèque d'assets), `EX-NFR-021` (test système de franchissabilité), `EX-REN-033` (traduction).
