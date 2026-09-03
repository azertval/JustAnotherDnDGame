# TACHE-02 — Regroupement des panneaux, suivant l'outil actif {#lot-57-tache-02-panneaux-groupes}

**Lot :** [LOT-57](epic.md) · **Emplacement :** `Source/HMI/Interface` · **Statut :** fait

## Contexte
Les cinq panneaux de l'éditeur — Palette, Outils, Niveaux, Liens, Textures — sont affichés
**simultanément** dès l'entrée en mode éditeur, et le restent quel que soit l'outil actif. Ils sont
déclarés côte à côte, deux à gauche et trois à droite, sans regroupement.

Or leur utilité est très inégale selon le moment : le panneau Liens ne sert qu'avec l'outil Lien ou pour
auditer les liaisons ; la Palette, avec ses trente entrées dépliées en permanence, ne sert qu'aux outils
de peinture ; le panneau Textures définit l'apparence, opération distincte de la construction du niveau.
Les trois panneaux de droite occupent en permanence une largeur qui manque au canevas.

Le bon réflexe existe pourtant déjà, à un seul endroit : dans le panneau Outils, le bloc de sélection
des décors est masqué quand l'outil Décor n'est pas actif. Cette tâche généralise ce principe, sans le
durcir : suivre l'outil actif tant que l'utilisateur n'a rien imposé, et se taire ensuite.

## Travail à réaliser
- **Regroupement en onglets** des panneaux de droite (Niveaux, Liens, Textures) dans une disposition par
  défaut, chacun restant individuellement déplaçable, détachable et refermable (`EX-IHM-010`).
- **Mise en avant selon l'outil actif** : changer d'outil met au premier plan l'onglet correspondant
  lorsqu'il en existe un — Lien vers le panneau Liens, Texture vers le panneau Textures.
- **Respect du choix explicite de l'utilisateur** : dès qu'il sélectionne lui-même un onglet ou
  réorganise les panneaux, la mise en avant automatique cesse pour la session. Aucun panneau ouvert
  explicitement n'est jamais masqué.
- **Réglage de la mise en avant** dans le menu Affichage, à côté des bascules de panneaux existantes,
  persisté entre deux sessions.
- **Incrémentation de la version de disposition** : elle n'a pas bougé depuis l'ajout du panneau
  Textures, si bien qu'une disposition sauvegardée par une version antérieure se restaure déjà mal.
  Cette tâche change la disposition par défaut et doit donc invalider les dispositions enregistrées.
- **Disposition par défaut reconstruite**, et action « réinitialiser la disposition » vérifiée contre
  elle.
- **Table extensible** : la correspondance outil → panneau accueillera les outils et les panneaux de
  l'atelier pixel art ([LOT-54](@ref lot-54)) — canevas, palette, historique. La table doit donc être
  une donnée que l'on complète, pas une suite de conditions écrites en dur sur les six outils actuels.

## Fichiers impactés
- `Source/HMI/Interface/MainWindow.{h,cpp}`, `Source/Elements/UI/MainWindow.ui`.
- `Source/HMI/Editor/PanelFocus.{h,cpp}` (nouveau) — correspondance pure outil → panneau.
- `Source/Elements/Localization/fr.lang`, `en.lang`.
- `Source/Test/Unit/HMI/Editor/test_panel_focus.cpp` (nouveau).

## Tests (obligatoires)
- **Correspondance outil → panneau** : chaque outil produit le panneau à mettre en avant, ou aucun.
  Fonction **pure**, testée sans Qt.
- **Choix explicite prioritaire** : après une sélection manuelle d'onglet, un changement d'outil ne
  déplace plus la mise en avant.
- **Réglage désactivé** : aucune mise en avant, quel que soit l'outil.
- **Persistance** : le réglage est relu au démarrage suivant.

## Points d'attention
- **Ne jamais masquer un panneau que l'utilisateur a ouvert.** C'est la ligne rouge de cette tâche :
  une mise en avant est une suggestion, un masquage est une confiscation.
- **Conserver le parcours dynamique des panneaux** dans la logique qui les affiche ou les cache selon le
  mode de l'application. Ce parcours a remplacé une liste écrite à la main précisément parce que
  celle-ci avait laissé le panneau Textures s'afficher par-dessus le menu principal lors de son ajout :
  toute liste en dur réintroduirait ce défaut au panneau suivant.
- **Une version de disposition non incrémentée est un piège silencieux** : la disposition restaurée est
  incohérente sans qu'aucun message ne l'indique. L'incrémenter fait partie de la tâche, pas d'un
  correctif ultérieur.
- Vérifier le comportement au passage éditeur → jeu → éditeur : les panneaux sont masqués puis
  réaffichés, et le regroupement doit survivre à l'aller-retour.
- Un onglet mis en avant ne doit pas voler le focus clavier au canevas : l'utilisateur peint à la souris
  tout en utilisant les raccourcis.

## Définition de fait (DoD)
- Les panneaux de droite sont regroupés en onglets par défaut, l'onglet pertinent suit l'outil actif
  tant que l'utilisateur n'a rien imposé, et aucun panneau ouvert explicitement n'est masqué ; la mise
  en avant est réglable et persistée ; la version de disposition est incrémentée et la réinitialisation
  vérifiée ; la correspondance est pure et testée ; `/W4 /WX` propre.

## Exigences
`EX-IHM-061` (panneaux groupés, suivant l'outil actif) ; réutilise `EX-IHM-010` (panneaux dockables),
`EX-IHM-011` (disposition persistée et réinitialisable), `EX-REN-033` (traduction).
