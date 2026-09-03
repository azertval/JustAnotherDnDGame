# LOT-57 — Architecture de l'information de l'éditeur {#lot-57}

> Statut : **fait**. Prérequis : [LOT-51](@ref lot-51) (visibilité des calques),
> [LOT-56](@ref lot-56) (jetons, thème, actions et barre d'outils).

## Objectif
Rendre l'éditeur **lisible** : afficher l'état dont l'auteur d'un niveau a besoin en permanence, et
cesser d'afficher tout le reste tout le temps.

L'éditeur a gagné un panneau ou un onglet à presque chaque lot du programme d'habillage — jeux de skins
au `LOT-42`, bibliothèque d'assets au `LOT-43`, fond au `LOT-44`, surcharges d'objets au `LOT-45`,
animations au `LOT-46`, décors au `LOT-50`, calques au `LOT-51`. Chaque ajout était justifié
isolément ; la répartition d'ensemble n'a jamais été revue. Le résultat se compte : **environ 106
contrôles simultanément à l'écran**, sur cinq panneaux tous affichés en même temps quel que soit
l'outil actif, sans aucun regroupement — ni séparateur, ni groupe, ni inspecteur contextuel. Le seul
panneau structuré, celui des textures, l'est par un jeu de six onglets hétérogènes qui concentre à lui
seul un tiers des contrôles.

Plusieurs états sont pilotables depuis **deux endroits distincts** : la couche d'un décor se règle dans
le panneau Outils et dans l'onglet Décors ; le mode d'affichage Physique/Texture bascule à la fois par
sa touche et par une case à cocher de l'onglet Calques ; deux listes déroulantes portent le nom « jeu de
skins » sans désigner la même chose (celui de la session d'édition, celui du niveau). Deux contrôles
pour une valeur, c'est une divergence possible et une question sans réponse : lequel fait autorité.

Le paradoxe est que cet éditeur saturé de **contrôles** n'affiche presque aucune **information**. Il ne
montre nulle part quel niveau est ouvert, s'il comporte des modifications non enregistrées — l'état
existe pourtant dans le viewport —, quel outil est actif, quelle case est survolée, ni à quel zoom on
travaille. La barre d'état ne porte qu'une ligne unique de sept raccourcis concaténés, identique quel
que soit l'outil, et **définitivement effacée** dès qu'un message transitoire l'a remplacée : rien ne la
restaure ensuite, hors changement de langue.

Ce lot ne retire aucune capacité. Il redistribue : ce qui informe devient permanent, ce qui commande
devient unique, et ce qui ne sert qu'à un outil s'efface quand cet outil n'est pas actif.

## Périmètre

### Inclus
- **Barre d'état structurée** : zones permanentes d'état (niveau ouvert, modifications non
  enregistrées, outil actif, case survolée, zoom) et zone de message transitoire qui **restaure**
  l'aide à son expiration ; aide **contextuelle à l'outil actif**.
- **Regroupement des panneaux** en onglets, avec mise en avant du panneau pertinent selon l'outil
  actif — sans jamais masquer un panneau ouvert explicitement.
- **Recentrage du panneau Textures** sur ce qui **définit l'apparence** ; sortie du mode d'inspection
  par calque vers le menu Affichage, où vit déjà le reste des bascules de vue.
- **Déduplication des commandes et des états**, en s'appuyant sur les actions livrées en
  [LOT-56](@ref lot-56) : un état, un contrôle ; une commande, une définition.
- **Raccourcis d'éditeur** : branchement des actions remappables définies mais jamais lues, et écran de
  remappage correspondant — les clés de traduction existent déjà et ne sont référencées nulle part.
- **Aperçu des raccourcis à l'écran**, qui concrétise la seconde moitié d'`EX-EDIT-015`.

### Exclus (hors périmètre de ce lot)
- **Refonte de la disposition dockable** : les panneaux restent déplaçables, redimensionnables,
  détachables et persistés (`EX-IHM-010`, `EX-IHM-011`). Ce lot change leur **regroupement par
  défaut**, pas le mécanisme.
- **Apparence** : couleurs, typographie, icônes et thème relèvent de [LOT-56](@ref lot-56).
- **Découpage de *GameViewport* et de *TexturePanel*** en tant que refactoring. Ces deux fichiers sont
  les plus volumineux du projet, mais leur restructuration est un sujet distinct : ce lot ne déplace du
  code que là où l'ergonomie l'exige.
- **Nouveaux outils ou nouvelles capacités d'édition** : aucune fonction nouvelle, uniquement une
  redistribution de l'existant.
- **HUD du jeu** ([LOT-52](@ref lot-52)) : l'information d'édition s'affiche dans les widgets Qt, pas
  dans la scène rendue.

## Décisions de cadrage
- **Redistribuer, pas retirer.** Chaque contrôle présent a été ajouté pour une raison ; le problème est
  sa présence *permanente*, pas son existence. Le lot ne supprime une capacité que lorsqu'elle est
  strictement dupliquée.
- **Un raccourci clavier n'est pas un doublon.** Deux contrôles pilotant le même état sont un défaut ;
  une commande accessible à la fois par un bouton et par une touche est un confort — à condition que la
  touche soit **affichée par la commande** plutôt que redéclarée ailleurs. C'est exactement ce que
  permettent les actions du `LOT-56`.
- **La mise en avant d'un panneau est une suggestion, jamais une contrainte.** Masquer automatiquement
  un panneau que l'utilisateur vient d'ouvrir est plus irritant que le problème qu'on corrige. La règle
  est donc : suivre l'outil actif tant que l'utilisateur n'a rien imposé, se taire ensuite.
- **Le mode d'inspection par calque n'appartient pas au panneau Textures.** Décomposer le rendu par
  calque est une manière de *regarder* le niveau, pas une propriété de texture. Sa présence dans ce
  panneau oblige aujourd'hui à afficher un avertissement expliquant qu'il n'affecte pas le jeu, et le
  place à côté d'une case qui double une touche existante — deux symptômes du même mauvais rangement.
- **L'information d'état se calcule à part.** Le choix de ce qu'affiche la barre d'état est une
  décision, pas un rendu : elle est exposée en fonction pure et testée sans Qt, sur le patron déjà
  retenu pour le HUD au `LOT-52`.
- **Étend [`interface-ihm.md`](../../Specification/interface-ihm.md)** d'une section 7 : la
  spécification cadrait l'existence des panneaux, jamais la répartition de l'information entre eux.

## Exigences couvertes
- Nouvelles : `EX-IHM-060` (état de travail affiché en permanence, aide contextuelle), `EX-IHM-061`
  (panneaux groupés, suivant l'outil actif sans masquer un panneau ouvert), `EX-IHM-062` (un état ou une
  commande à un seul endroit).
- Concrétise la partie « aperçu des raccourcis clavier » d'`EX-EDIT-015`, restée non implémentée depuis
  le `LOT-15`.
- Réutilisées : `EX-IHM-010`/`EX-IHM-011` (panneaux dockables et disposition persistée), `EX-IHM-055`
  (actions, `LOT-56`), `EX-EDIT-044` (visibilité des calques, `LOT-51`), `EX-CTRL-012` (raccourcis
  d'éditeur remappables), `EX-REN-033` (traduction).

## Amendement post-essai manuel (2026-08-09)
Après livraison des quatre tâches, premier essai manuel réel de l'éditeur reconstruit : deux retours
directs sur la disposition, tranchés et implémentés dans la foulée (branche encore ouverte, non
mergée — pas un nouveau lot) :
- **Les « palettes de texture » devaient redevenir un panneau à part**, sur le modèle
  Outils/Palette. Le panneau « Outils » (`ToolPanel`), qui ne portait déjà plus que le strict
  nécessaire de l'outil Décor depuis `LOT-56` TACHE-04 (le choix d'outil s'était déplacé vers la
  barre d'outils), est renommé « Décors » (`DecorsPanel`) et regroupe désormais aussi l'inspecteur
  des décors posés (déplacé de l'onglet « Décors » du panneau Textures, `LOT-50`). Le panneau
  Textures sort du regroupement en onglets de TACHE-02 (Niveaux/Liens restent tabifiés, Textures
  redevient indépendant, comme Palette/Décors).
- **La barre d'outils reste hors du panneau Décors**, à son emplacement d'avant l'amendement (barre
  globale de la fenêtre principale, `addToolBar()`, même `EditorActions::populateToolBar`) : un
  premier essai l'avait embarquée verticalement dans le panneau Décors, corrigé après retour
  immédiat — elle n'a pas vocation à dépendre d'un panneau ou d'un mode particulier.
- Les deux sélecteurs de couche de décor (placement du prochain décor, couche du décor sélectionné)
  vivent désormais dans le même panneau, renommés explicitement (`decorPlacementLayerCombo`/
  `decorSelectedLayerCombo`) plutôt que de porter le même nom générique dans le même `.ui`.
- `LAYOUT_VERSION` 4 → 5 (dock renommé).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-barre-etat.md) | Barre d'état structurée : état permanent et aide contextuelle à l'outil | `Source/HMI/Interface`, `Source/HMI/Editor` | ✅ |
| [TACHE-02](tache-02-panneaux-groupes.md) | Regroupement des panneaux en onglets, suivant l'outil actif | `Source/HMI/Interface` | ✅ |
| [TACHE-03](tache-03-panneau-textures.md) | Recentrage du panneau Textures ; calques déplacés vers le menu Affichage | `Source/HMI/Editor`, `Source/Elements/UI` | ✅ |
| [TACHE-04](tache-04-deduplication-raccourcis.md) | Déduplication des commandes, raccourcis d'éditeur branchés et remappables, aperçu à l'écran | `Source/HMI/Game`, `Source/HMI/Input`, `Source/HMI/Interface` | ✅ |

## Critères d'acceptation du lot
1. Le niveau ouvert, la présence de modifications non enregistrées, l'outil actif, la case survolée et
   le zoom sont visibles en permanence pendant l'édition.
2. Un message transitoire ne laisse jamais la barre d'état vide : l'aide contextuelle revient à son
   expiration.
3. L'aide affichée change avec l'outil actif.
4. Les panneaux de droite sont regroupés en onglets et celui qui correspond à l'outil actif est mis en
   avant ; un panneau ouvert explicitement n'est jamais masqué automatiquement.
5. Aucun état n'est pilotable depuis deux contrôles distincts ; chaque commande affiche son raccourci
   sans qu'il soit défini deux fois.
6. Les raccourcis d'éditeur remappables sont tous effectifs et réglables depuis l'écran de remappage ;
   aucune clé de traduction d'action d'éditeur ne reste orpheline.
7. Un aperçu des raccourcis est atteignable depuis l'application.
8. La disposition sauvegardée d'une session précédente se restaure sans panneau égaré ; libellés dans
   les deux langues ; build `/W4 /WX`, Doxygen, lint verts.

## Dépendances
Bâtit sur [LOT-56](@ref lot-56), dont les actions rendent la déduplication structurelle plutôt que
cosmétique, et sur [LOT-51](@ref lot-51), dont le mode d'inspection par calque est déplacé. Utilise le
patron de fonction pure de décision d'affichage établi par [LOT-52](@ref lot-52). Aucune dépendance sur
les autres lots du programme d'habillage.

[LOT-54](@ref lot-54) s'exécute derrière celui-ci et s'y branche : l'atelier pixel art étendra le
modèle pur de la barre d'état d'un contexte d'édition d'asset, entrera dans le regroupement des
panneaux et dans la table de mise en avant, prendra son point d'entrée dans le panneau Textures
recentré, et donnera une seconde cible aux actions Annuler et Refaire dédupliquées ici. Quatre des
tâches de ce lot sont donc à écrire en prévoyant ce second contexte d'édition, plutôt qu'en figeant
leurs signatures sur le seul contexte niveau.

## Navigation des tâches
- @subpage lot-57-tache-01-barre-etat
- @subpage lot-57-tache-02-panneaux-groupes
- @subpage lot-57-tache-03-panneau-textures
- @subpage lot-57-tache-04-deduplication-raccourcis
