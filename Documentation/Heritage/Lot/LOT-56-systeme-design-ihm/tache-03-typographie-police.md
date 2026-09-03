# TACHE-03 — Police embarquée et typographie à source unique {#lot-56-tache-03-typographie-police}

**Lot :** [LOT-56](epic.md) · **Emplacement :** `Source/Elements`, `Source/HMI/Interface` · **Statut :** fait

## Contexte
La typographie de l'interface a **deux sources de vérité** qui se contredisent. Les fichiers
`Source/Elements/UI/MainMenu.ui` et `Source/Elements/UI/OptionsPage.ui` figent des tailles de police et
des marges dans leurs propriétés ; la feuille de style redéfinit ensuite une famille et une taille pour
les mêmes contrôles. Selon la propriété considérée, c'est l'un ou l'autre qui l'emporte — comportement
que personne ne peut prédire à la lecture.

Plus gênant : la famille imposée par la feuille de style n'est disponible **que sous Windows**.
L'interface dépend donc d'une police installée sur le système hôte, sans repli déclaré. Le dossier
`Source/Elements/Assets/Fonts/` existe pourtant déjà — créé au `LOT-52` — et ne contient qu'un fichier
d'explication.

Le projet a par ailleurs un précédent net sur la question du repli : `hmi::ProceduralFont` (`LOT-52`)
garantit que le jeu reste lisible **sans aucun asset de police** (`EX-NFR-040`). L'interface hors-jeu
doit offrir la même garantie, par le mécanisme propre à Qt.

## Travail à réaliser
- **Échelle typographique dans les jetons** (TACHE-01) : un petit nombre de niveaux nommés par rôle
  (titre d'écran, titre de section, corps, libellé secondaire, monospace), chacun avec sa taille et sa
  graisse. Pas de taille ponctuelle en dehors de l'échelle.
- **Police embarquée** : fichier de police déposé dans `Source/Elements/Assets/Fonts/`, enregistré au
  démarrage auprès de la base de polices de Qt, et employé par le thème.
- **Repli explicite** : si le fichier est absent ou refusé par Qt, l'application retombe sur une
  famille générique demandée à Qt (et non sur un nom de police codé en dur), en journalisant un
  avertissement. L'interface reste lisible et correctement dimensionnée dans ce cas.
- **Retrait des propriétés de police et de marge figées** dans `MainMenu.ui` et `OptionsPage.ui`, au
  profit de l'échelle typographique et de l'échelle d'espacement des jetons.
- **Vérification du choix de police** : la famille retenue doit couvrir les caractères accentués
  français employés par le catalogue de traduction, et rester lisible aux petites tailles des libellés
  de panneaux.
- **Licence** : la police retenue doit être redistribuable avec l'application ; sa licence est déposée
  à côté du fichier et mentionnée dans le fichier d'explication du dossier.

## Fichiers impactés
- `Source/Elements/Assets/Fonts/` — fichier de police, sa licence, mise à jour du fichier
  d'explication.
- `Source/HMI/Interface/DesignTokens.h` — échelle typographique.
- `Source/HMI/Interface/ApplicationTheme.{h,cpp}` — enregistrement de la police, résolution du repli.
- `Source/Elements/UI/MainMenu.ui`, `Source/Elements/UI/OptionsPage.ui` — retrait des polices et marges
  figées.
- `Source/Elements/Themes/theme.qss` — famille et tailles issues des jetons.
- `Source/Test/Unit/HMI/Interface/test_application_theme.cpp` — complété.

## Tests (obligatoires)
- **Résolution de la famille** : police disponible → la famille embarquée est retenue ; police absente
  → une famille générique est demandée et un avertissement est journalisé. Décision exposée en fonction
  **pure** (entrée : la police a-t-elle pu être enregistrée ; sortie : la famille à employer), testée
  sans instance d'application.
- **Échelle typographique** : chaque niveau produit une taille strictement positive, et les niveaux sont
  ordonnés du plus grand au plus petit — un test qui échoue si une modification casse la hiérarchie
  visuelle.
- **Aucune taille de police résiduelle dans les fichiers d'interface** : test lisant les deux fichiers
  `.ui` et échouant si une propriété de police y réapparaît.

## Points d'attention
- **Ne pas coder en dur un second nom de police comme repli.** Nommer une police de repli reproduit le
  défaut qu'on corrige : c'est à Qt qu'il faut demander une famille générique.
- Vérifier le rendu **sans la police embarquée** avant de conclure la tâche : c'est le chemin qui
  s'exécutera sur toute machine où le fichier n'aura pas été déployé.
- La police du menu principal est aujourd'hui monospace, ce qui lui donne son caractère. Si l'échelle
  retenue abandonne le monospace, c'est un **choix esthétique délibéré**, à assumer — pas un effet de
  bord du nettoyage.
- Retirer les marges des fichiers `.ui` change la composition des deux écrans : les revoir visuellement
  fait partie de la tâche.
- La police de l'interface n'a **aucun rapport** avec la police bitmap du HUD (`LOT-52`), qui vit dans
  le pipeline Direct3D 11. Deux mécanismes distincts, deux fichiers distincts.

## Définition de fait (DoD)
- L'interface s'affiche correctement sans aucune police installée sur le système hôte ; la typographie
  a une source de vérité unique et aucune taille ne subsiste dans les fichiers `.ui` ; le repli est
  testé et journalisé ; la licence de la police est déposée ; `/W4 /WX` propre.

## Exigences
`EX-IHM-052` (typographie unique, police embarquée avec repli) ; réutilise `EX-IHM-050` (thème),
`EX-REN-033` (traduction — couverture des caractères accentués), `EX-NFR-040` (repli en l'absence
d'asset).
