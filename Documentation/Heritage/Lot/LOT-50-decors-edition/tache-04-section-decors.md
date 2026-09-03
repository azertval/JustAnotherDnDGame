# TACHE-04 — Section « Décors » du panneau « Textures » {#lot-50-tache-04-section-decors}

**Lot :** [LOT-50](epic.md) · **Emplacement :** `Source/HMI/Editor` · **Statut :** fait

## Contexte
Manipuler des décors uniquement au canevas devient vite impraticable : un décor caché derrière un
autre, ou situé hors du cadrage courant, n'est pas atteignable à la souris. Il faut une **vue en
liste** qui donne accès à tout ce que contient le niveau.

C'est la cinquième et dernière section du panneau « Textures » ouvert en LOT-42, et le dernier test
de son extensibilité.

## Travail à réaliser
- **Section « Décors »** : liste des décors du niveau courant, **groupés par couche** et affichés
  dans leur **ordre de superposition** — c'est ce qui rend le réordonnancement compréhensible.
- **Sélection croisée** : sélectionner dans la liste met en évidence le décor dans le canevas, et
  inversement. Les deux vues partagent une seule sélection.
- **Actions depuis la liste** : réordonner (avancer, reculer), changer de couche, supprimer,
  centrer la caméra sur le décor — c'est la voie d'accès aux décors non atteignables à la souris.
- **Vignette et nom d'asset** par entrée, via le widget de LOT-43.
- **Signalement des assets manquants** : un décor dont l'asset est introuvable est mis en évidence
  dans la liste. Le damier magenta le signale déjà dans le canevas, mais la liste permet de tous les
  repérer d'un coup d'œil.
- Chaînes traduites.

## Fichiers impactés
- `Source/HMI/Editor/TexturePanel.{h,cpp}`, `Source/Elements/UI/TexturePanel.ui`.
- `Source/HMI/Editor/DecorGesture.{h,cpp}` (sélection partagée).
- `Source/Elements/Localization/fr.lang`, `en.lang`.

## Tests (obligatoires)
- **Construction du modèle de liste** : groupement par couche, ordre de superposition, marquage des
  assets manquants — fonction pure, testée sans Qt.
- La sélection est **unique** : sélectionner dans la liste puis dans le canevas ne laisse jamais deux
  décors sélectionnés.
- Les actions de la liste passent par les mêmes mutateurs que le canevas (TACHE-01) et sont
  annulables.

## Points d'attention
- **Ne pas dupliquer l'état de sélection.** Une sélection stockée à la fois dans le panneau et dans
  le geste divergera ; une seule source, les deux vues l'observent.
- Un niveau peut contenir beaucoup de décors : prévoir le filtrage et éviter de reconstruire toute la
  liste à chaque modification (un déplacement au glisser émet de nombreuses mises à jour).
- La liste doit refléter l'**annulation** : annuler un ajout doit retirer l'entrée correspondante.

## Définition de fait (DoD)
- La section liste les décors par couche et dans l'ordre de superposition, permet de les
  sélectionner, réordonner, déplacer de couche, supprimer et centrer, signale les assets manquants,
  partage une sélection unique avec le canevas, et suit l'annulation ; modèle testé sans Qt ;
  `/W4 /WX` propre.

## Exigences
`EX-DEC-010` (superposer, supprimer), `EX-EDIT-040` (édition de décors) ; réutilise `EX-DEC-002`
(couches), `EX-EDIT-026` (bibliothèque d'assets), `EX-EDIT-005` (annuler/refaire), `EX-IHM-010`
(fenêtre à panneaux), `EX-REN-033` (traduction), `EX-NFR-040` (asset manquant signalé).
