# LOT-73 — Refonte IHM : taille, réactivité, mode IA {#lot-73}

> Statut : **fait**.

## Objectif
Trois symptômes rapportés à l'usage — une fenêtre qui réclame plus grand que l'écran, une interface
qui gèle plusieurs secondes en changeant d'écran, un Mode IA peu lisible et incomplet dans ses
données — remontent à **deux défauts structurels** et **un écart de périmètre**. Ce lot les traite à
la racine, en invariants tenus par des tests plutôt qu'en correctifs écran par écran.

**1. Un écran dictait sa taille à la fenêtre.** `QStackedWidget::minimumSizeHint` vaut le maximum
sur *toutes* ses pages, y compris masquées : un seul écran dense fixait donc le plancher de la
fenêtre entière. Ce plancher était multiplié par le facteur d'agrandissement des écrans du jeu
(`EX-IHM-070`, jusqu'à 3×), lui-même dérivé de la hauteur de la fenêtre — une boucle sans point
fixe, que rien ne bornait à l'écran (`availableGeometry` n'apparaissait nulle part dans le dépôt).
Journal rapporté par l'utilisateur, sur un écran offrant `1920x1009` :

```
Navigation : menu principal.
QWindowsWindow::setGeometry: Unable to set geometry 1920x1476 … minimum size: 764x1476
QWindowsWindow::setGeometry: Unable to set geometry 1920x1497 … minimum size: 1030x1497
Navigation : mode IA.
QWindowsWindow::setGeometry: Unable to set geometry 2280x1560 … minimum size: 2280x1560
```

Le défaut s'était déjà produit **deux fois**, corrigé deux fois localement (une `QScrollArea` posée
dans l'écran fautif). Le commentaire de `MainWindow.cpp` le consignait sans que rien ne l'empêche de
revenir.

**2. Une préoccupation d'écran repolissait toute l'application.** Le facteur d'agrandissement ne
concerne que les écrans du jeu, mais il était substitué dans la feuille de style de
l'**application** : en changer repolissait ses 862 widgets — **cinq secondes par appel** en
configuration Debug. Le regroupement introduit auparavant rendait ce coût supportable pendant un
glisser de bordure, mais ne le supprimait pas, et le faisait atterrir **après** que la fenêtre ait
été placée et peinte : d'où un recalage visible.

**3. Le Mode IA montrait moins que ce que le moteur sait.** Neuf réglages de l'onglet Entraînement
étaient lus par l'écran puis **jetés** — `TrainingRequest` n'avait pas de champ où les recevoir. Les
régler ne changeait rien, et le `config.json` du run affirmait le contraire, de sorte que
« Reprendre les réglages de ce run » rechargeait des valeurs fausses. Trois champs ouvraient par
ailleurs sur des valeurs différentes des défauts du moteur. Enfin, la moyenne mobile et la variation
de récompense étaient calculées, écrites au CSV, puis perdues avant d'atteindre l'écran.

## Décisions de cadrage

**L'invariant vit sur le chemin d'ajout, pas dans les fichiers `.ui`.** Envelopper sept écrans un
par un aurait refermé le symptôme sans fermer la cause : la règle se reperd au premier écran ajouté,
ce qui s'est produit deux fois. `hmi::ScreenPageHost` est donc traversé par **toute** page de la
pile, et un écran futur en hérite sans que personne ait à y penser.

**Le Mode IA devient hybride : enveloppe d'identité, contenu d'outil.** Cet écran est le seul de la
portée identité à n'être pas un écran de *joueur* — c'est un poste de travail. Il garde son fond,
son titre et son cadre à bordure franche, qui le rattachent visiblement au jeu, et son contenu
adopte la typographie et la densité d'un outil : plus de police bitmap sur vingt-six lignes de
formulaire, plus de grandeurs multipliées par le facteur d'agrandissement.

**La borne du facteur vient de l'écran, pas de la fenêtre.** Une grandeur dont l'application décide
ne peut pas borner une boucle que l'application entretient. La zone d'affichage disponible, elle,
ne dépend d'aucune de ses décisions.

## Périmètre

### Inclus
- **`hmi::ScreenPageHost`** : enveloppe défilante de toute page d'écran, à contribution nulle dans
  la taille minimale de la fenêtre.
- **`hmi::pixelArtScaleForDisplay`** : facteur borné par la zone d'affichage disponible.
- **Géométrie restaurée** ramenée dans la zone utile ; garde-fou journalisé au démarrage.
- **Deux feuilles de style disjointes** (`theme-identity.qss`, `theme-editor.qss`), appliquées
  chacune à sa portée — l'identité sur la pile d'écrans, le châssis sur l'application.
- **Neuf réglages du Mode IA** câblés jusqu'au moteur, via `hmi::overridesFor` — traduction pure,
  donc testable champ par champ.
- **Données déjà produites** enfin transmises : moyenne mobile, variation de récompense, pas cumulés
  du DQN.
- **Lisibilité du Mode IA** : graphique doté d'axes, de graduations et d'une courbe lissée ; tables
  aux en-têtes dimensionnés ; contrôles à densité d'outil.

### Exclus (hors périmètre de ce lot)
- **Latence du jeu en configuration Debug** — famille de défauts distincte du gel d'interface traité
  ici, relevée pendant l'exploration et consignée en `TACHE-06` : trace émise à chaque frame dans le
  chemin de rendu, puits de journalisation synchrones, absence de préréglage `RelWithDebInfo`. À
  cadrer séparément.
- **Reprise d'un entraînement interrompu**, **entraînement multi-niveaux**, **file d'attente de
  runs** : décisions de cadrage du [LOT-ANNEXE-21](@ref lot-annexe-21), non rouvertes.
- **Exposition des comparateurs `Eval/`** (convergence, transfert inter-niveaux, robustesse au
  bruit) : sans point d'entrée en ligne de commande non plus, ils restent une API de bibliothèque.

## Exigences couvertes
Nouvelles : [`EX-IHM-080`](@ref EX-IHM-080) (aucun écran ne contraint la fenêtre),
[`EX-IHM-081`](@ref EX-IHM-081) (facteur et géométrie bornés par l'écran),
[`EX-IHM-082`](@ref EX-IHM-082) (une portée ne repolit pas plus large qu'elle),
[`EX-IHM-083`](@ref EX-IHM-083) (tout réglage exposé atteint le moteur).
Réutilisées : [`EX-IHM-070`](@ref EX-IHM-070), [`EX-IHM-050`](@ref EX-IHM-050),
[`EX-IHM-054`](@ref EX-IHM-054), [`EX-IA-022`](@ref EX-IA-022), [`EX-REN-033`](@ref EX-REN-033).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-invariant-taille.md) | Aucun écran ne contraint la fenêtre | `Source/HMI/Interface` | ✅ |
| [TACHE-02](tache-02-portees-theme.md) | Le rejeu de thème cesse d'être global | `Source/HMI/Interface`, `Source/Elements/Themes` | ✅ |
| [TACHE-03](tache-03-reglages-effectifs.md) | Tout réglage exposé atteint le moteur | `Source/HMI/Ai`, `Source/HMI/Interface` | ✅ |
| [TACHE-04](tache-04-donnees-suivi.md) | Les données que le moteur produit déjà | `Source/AiSolver/Stats`, `Source/HMI/Ai` | ✅ |
| [TACHE-05](tache-05-mode-ia-lisible.md) | Mode IA hybride et lisible | `Source/Elements`, `Source/HMI/Interface` | ✅ |
| [TACHE-06](tache-06-exigences-doc.md) | Exigences, localisation, documentation | `Documentation`, `Source/Elements/Localization` | ✅ |

## Critères d'acceptation du lot
1. Aucun écran n'élève la taille minimale de la fenêtre : la ligne `QWindowsWindow::setGeometry:
   Unable to set geometry` ne paraît plus dans le journal, sur aucun écran, sur un affichage de
   `1920x1009` utiles.
2. Le facteur d'agrandissement ne dépasse jamais celui qu'admet la zone d'affichage disponible,
   quelle que soit la hauteur de fenêtre demandée — y compris supérieure à l'écran.
3. Une géométrie persistée débordante est ramenée dans l'écran au lancement suivant.
4. Un changement de facteur ne repose que la feuille de la portée identité, sur la pile d'écrans :
   aucun rejeu de style applicatif, donc aucun gel perceptible en navigation ni en
   redimensionnement.
5. Les deux feuilles de style sont disjointes : ni jeton de châssis dans l'identité, ni jeton
   d'identité dans le châssis.
6. Tout réglage de l'onglet Entraînement atteint la configuration résolue du run, et l'écran ouvre
   sur les défauts du moteur — « Réinitialiser aux défauts » ne change aucun champ à l'ouverture.
7. Le graphique porte ses graduations, ses valeurs extrêmes, son intervalle de générations et une
   courbe de moyenne mobile ; les en-têtes des tables sont lisibles sans défilement horizontal.
8. Les deux catalogues de traduction déclarent exactement les mêmes clés.
9. Build `/W4 /WX` sans avertissement, Doxygen et lint des exigences verts.

## Dépendances
Complète [`LOT-56`](@ref lot-56) (système de design, portées de thème), `LOT-68`
(facteur d'agrandissement des écrans du jeu) et [`LOT-ANNEXE-22`](@ref lot-annexe-22) (IHM complète
du Mode IA) sans les remplacer. Aucun lot ultérieur n'en dépend à ce jour.

## Navigation des tâches
- @subpage lot-73-tache-01-invariant-taille
- @subpage lot-73-tache-02-portees-theme
- @subpage lot-73-tache-03-reglages-effectifs
- @subpage lot-73-tache-04-donnees-suivi
- @subpage lot-73-tache-05-mode-ia-lisible
- @subpage lot-73-tache-06-exigences-doc
