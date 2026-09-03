# TACHE-05 — Mode IA hybride et lisible {#lot-73-tache-05-mode-ia-lisible}

**Lot :** [LOT-73](epic.md) · **Emplacement :** `Source/Elements`, `Source/HMI/Interface` ·
**Statut :** fait

## Contexte
`#AiModeScreen` figurait parmi les sept surfaces de la **portée identité** du thème, aux côtés du
menu principal et de l'écran de pause. Il en héritait la police bitmap et les grandeurs multipliées
par le facteur d'agrandissement — jusqu'à 3×.

Or cet écran est le seul de cette portée à n'être pas un écran de **joueur**. C'est un poste de
travail : vingt-six lignes de formulaire, une table à huit colonnes, un graphique, trois onglets.
Habillé comme un menu, il réclamait une hauteur minimale de plus de deux mille pixels et rendait des
données denses dans une police conçue pour sept mots à l'écran.

Trois défauts de lisibilité s'y ajoutaient :
- **le graphique n'avait ni axes, ni graduations, ni valeurs, ni axe des abscisses** : une courbe
  qui monte n'y apprenait que le signe d'une progression, jamais son ampleur, et deux runs ne s'y
  comparaient pas ;
- **il était écrasé sur ses 140 pixels minimaux** — sans politique de taille déclarée, un `QWidget`
  vaut `Preferred` avec une taille idéale invalide, tandis que la table voisine est `Expanding` par
  défaut et prenait donc toute la hauteur ;
- **aucune des deux tables n'avait d'en-tête dimensionné** : huit libellés français longs restaient
  à la largeur par défaut, donc tronqués, avec un défilement horizontal permanent pour lire ce qui
  aurait tenu, et un en-tête vertical qui répétait la colonne « Génération ».

## Travail réalisé
- **Habillage hybride** (`theme-identity.qss`) : l'écran garde son **enveloppe** — fond, titre en
  police de titre, cadre à bordure franche, bouton de retour en entrée de menu — et son **contenu**
  passe en densité d'outil. Concrètement, la règle de l'écran ne nomme plus de `font-family` ni de
  `font-size` : ne pas en nommer laisse la police par défaut de l'application s'appliquer aux
  descendants. Les rembourrages du contenu viennent de `tokens.spacing.*`, **jamais multipliés**, là
  où `identity.size.*` et `identity.space.*` le sont.
- **Couleurs du jeu conservées** pour le contenu. Les emprunter au châssis l'aurait fait basculer en
  clair au milieu d'un écran sombre ; l'identité étant invariante, elle reste cohérente avec le cadre
  qui l'entoure.
- **Graphique** (`TrainingChartWidget`) : trois graduations horizontales avec leurs valeurs, les
  bornes de l'intervalle de générations sous la zone de tracé, une quatrième courbe de **moyenne
  mobile**, et une politique de taille qui lui donne un facteur d'étirement `3` contre `4` pour la
  table — la table reste la plus grande, le graphique cesse d'être une bande.
- **Tables** : `ResizeToContents` sur les colonnes, dernière section étirée, en-tête vertical masqué
  — appliqué aux deux tables de l'écran.
- **Ordre du formulaire** : « Dossier des runs » était déclaré en ligne 4, entre la topologie et le
  budget de pas, alors que le reste suivait l'ordre d'écriture. Il passe en dernier, ce qu'il est —
  un réglage de **sortie**, pas un réglage de ce que le run fait. L'ordre des lignes du fichier
  décrit de nouveau l'ordre à l'écran, seule chose qu'un fichier de description d'interface ait à
  faire.
- **« Voir en jeu »** : l'état d'activation et l'infobulle qui l'explique sont posés **ensemble**
  (`setPreviewAvailable`). Un contrôle grisé dont rien ne dit pourquoi se lit comme une panne, alors
  que l'attente du premier aperçu est le déroulement normal d'un run.

## Vérification
- Les tests de thème existants restent verts, la séparation des portées comprise.
- Vérification à l'IHM : contenu lisible à densité d'outil sous un cadre resté celui du jeu ;
  graphique gradué et partageant la hauteur avec la table ; en-têtes complets sans défilement
  horizontal.

## Écart assumé
Les couleurs des quatre courbes restent des constantes nommées dans `TrainingChartWidget.cpp`, et
non des jetons de `hmi::DesignTokens` comme le cadrage l'envisageait. Les jetons décrivent des
**rôles de surface** (fond, bordure, accent, texte) ; quatre courbes superposées posent un autre
problème — rester distinguables **entre elles** — auquel un rôle de surface ne répond pas. Prendre
`accent` pour deux d'entre elles les rendrait indiscernables, et aucun jeton ne couvre les deux
autres. Une palette catégorielle serait à introduire dans les jetons comme un rôle à part entière,
ce qui dépasse le périmètre de ce lot ; la raison est consignée à l'endroit du choix.
