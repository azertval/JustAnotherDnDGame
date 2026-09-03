# TACHE-06 — Panneau « Proprietes » {#lot-67-tache-06-panneau-proprietes}

**Lot :** [LOT-67](epic.md) · **Emplacement :** `Source/HMI/Editor`, `Source/Elements/UI` · **Statut :** fait

## Contexte
Les parametres de temporisation (vitesse, dephasage, periode, duree active) ne se manipulent pas au
canevas : ce sont des nombres. Aucun panneau ne les exposait, et les regles du tableau non plus.

Le projet n'a **aucun** inspecteur de proprietes generique, ni reflexion : chaque propriete est
cablee a la main, le panneau emettant un signal type que le viewport applique au brouillon.

## Travail a realiser
- Creer un **nouveau dock** « Proprietes » plutot qu'un onglet du panneau Textures : celui-ci porte
  l'apparence, et le `LOT-57` l'a precisement debarrasse de ce qui n'en relevait pas.
- Mise en page **integralement dans le `.ui`** (Qt Designer, accessible a un non-developpeur) ; le
  C++ ne branche que le fonctionnel.
- Une pile de pages pilotee par la selection : plateforme, danger mobile, danger temporise, ou page
  d'attente.
- Deux groupes de regles de tableau, **explicitement libelles** « budget consommable » et
  « capacites rechargees a l'atterrissage » : c'est le libelle qui previent la confusion.
- Les champs numeriques emettent sur `editingFinished`, **jamais** `valueChanged` : taper « 120 »
  produirait sinon trois mutations, donc trois pas d'annulation pour un seul geste.
- Les signaux portent la **case** de l'element, jamais un rang de vecteur : un rang se perime des
  qu'une configuration est ajoutee ou retiree, une case non.

## Fichiers impactes
`Source/HMI/Editor/PropertiesPanel.{h,cpp}` (nouveaux), `Source/Elements/UI/PropertiesPanel.ui`
(nouveau), `MainWindow.ui`, `Source/HMI/Interface/MainWindow.{h,cpp}`,
`Source/HMI/Game/GameViewport.{h,cpp}`, `Source/Elements/Localization/*.lang`.

## Tests (obligatoires)
Les mutations sous-jacentes sont couvertes par `test_level_draft.cpp` (mutateurs granulaires et
annulation des quatre regles de tableau) ; la traduction, par le test de completude des catalogues.
Le panneau lui-meme est un widget Qt : son comportement d'affichage releve de la verification IHM
manuelle, comme les autres panneaux du depot.

## Points d'attention
La garde de reentrance (`_updating`) est indispensable : sans elle, repeupler les champs depuis le
brouillon reemettrait des mutations en boucle.

## Definition de fait (DoD)
Les quatre regles et tous les parametres de temporisation sont editables et annulables ; les deux
notions de regles sont visiblement distinctes.

## Exigences
`EX-EDIT-033`, `EX-EDIT-005`, `EX-GP-024`, `EX-GP-053`, `EX-GP-055`, `EX-IHM-062`.
