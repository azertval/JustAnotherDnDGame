# LOT-86 — Refonte de l'IHM sur Qt Quick {#lot-86}

> Statut : **en cours**.
> Prérequis : `LOT-38`, `LOT-68`.

## Objectif

**Rendre les interfaces du jeu modifiables par un artiste sans qu'il ouvre un fichier source.**

Pas « ranger le C++ », pas « QML est plus souple » : séparer la conception du code comme le fait
l'industrie, où ce sont deux métiers qui ne se marchent pas dessus. Tout ce que ce lot livre se juge
à ce critère, et à lui seul.

Corollaire tenu partout : **aucune surcouche**. Une couche qui n'existe que pour compenser
l'inadéquation d'une autre est le défaut qu'on corrige, pas la solution.

## Ce qui existait

**Côté code**, la couche Qt Widgets avait cessé d'être tenable :

- `MainWindow.cpp` faisait **2 472 lignes** et mêlait le poste de travail de l'éditeur, le viewport
  de rendu, la pile des écrans de jeu, l'échelle d'identité et la feuille de style.
- Le défaut du plancher de taille — `QStackedWidget::minimumSizeHint` vaut le maximum sur **toutes**
  les pages, y compris masquées — s'était produit **trois fois**. Corrigé deux fois écran par écran,
  puis masqué par un enveloppeur `ScreenPageHost`. Le guide en tirait lui-même la leçon : *« une
  règle qu'il faut se rappeler d'appliquer se reperd au premier écran ajouté »*.
- Le `LOT-85`, abandonné au profit de celui-ci, avait ajouté **1 268 lignes d'outillage** — plugin
  Qt Designer, résolveur de feuille de style, générateur de `.ui`, deux garde-fous — dont l'unique
  fonction était de rendre les `.ui` visualisables dans Qt Designer. Surcouche exemplaire : elle
  compensait le fait que le format `.ui` ne sait pas décrire ces écrans.
- Surface totale : **~23 700 lignes** (14 304 de C++ `Interface/`, 7 158 de `.ui`, 980 de feuilles
  de style, 766 d'outillage, 502 de scripts).

**Côté conception**, il n'y avait pas de séparation mais une transcription. `.design-mockups/`
portait des maquettes HTML/CSS dessinées à la main à 1280 × 720 ; la palette y était **écrite deux
fois**, en CSS et en C++, tenue par un lint dont le commentaire disait : *« Rien ne les reliait. Une
retouche de teinte d'un côté laissait l'autre en arrière sans qu'aucune Pull Request ne le
signale. »* Les libellés étaient recopiés depuis `fr.lang` **mot pour mot**, à la main.

Trois représentations du même écran, deux transcriptions manuelles, un garde-fou pour rattraper les
erreurs. L'artiste dessinait, un développeur transcrivait.

## La direction artistique, reprise des maquettes supprimées

`.design-mockups/` disparaît avec ce lot : la maquette et l'écran deviennent le même fichier. Ses
décisions sont reportées ici, seule trace qui subsiste.

### Direction A — « Ambre nuit » (retenue)

Bleu-nuit et ambre **`#ffd133`**, rendus en pixel art. Continuité totale avec l'existant : la portée
identité change de traitement sans changer de teintes, donc peu de risque de régression sur le
contraste, et le HUD en jeu reste cohérent avec les écrans sans retouche.

*Le prix, assumé* : c'est la moins mémorable des trois. Personne ne reconnaîtra le jeu à sa capture
d'écran.

### Les deux directions écartées, et pourquoi

Gardées pour ne pas refaire le débat dans six mois.

- **B — Cyan cathodique.** Noir profond, cyan froid, lignes de balayage. La plus caractérisée : on
  sait en une seconde qu'on est devant un jeu et non un logiciel. *Écartée* : les lignes de balayage
  fatiguent sur de longues sessions, et le froid va mal aux écrans de fin de niveau, qui doivent
  être accueillants.
- **C — Néon arcade.** Magenta et cyan sur violet profond, la seule qui donne une couleur signature.
  *Écartée* : deux accents, c'est deux fois plus de règles à tenir — il faudrait décider une bonne
  fois lequel dit « action » et lequel dit « information ».

### Décisions d'écran à honorer

- **Menu principal** — pas de cadre : les entrées se posent sur une scène à trois plans. La
  lisibilité tient à un dégradé sombre sur le tiers gauche plutôt qu'à un voile plein, pour que le
  décor reste visible.
- **Options** — compteur d'images par seconde en haut à droite, onglet Vidéo. Il remplace l'ancien
  sélecteur de limite d'images/s, grisé et jamais branché : afficher le chiffre est utile, imposer
  un plafond ne l'était pas. C'est un élément de HUD, il vit avec le HUD.
- **Fin de niveau** — le bilan (temps, morts, sauts) est retenu. **À savoir avant de démarrer :
  aucune de ces trois valeurs n'est comptée aujourd'hui.** Il faut les accumuler pendant la partie
  et les faire remonter, et le temps doit être mesuré en **pas de simulation** plutôt qu'en horloge
  murale pour rester comparable d'une machine à l'autre. C'est du travail de session de jeu, pas
  d'habillage.

## Critères d'acceptation

*(complétés à la livraison)*

- **Test de l'artiste** — ouvrir `Source/Ui/JadgUi.qmlproject` dans Qt Design Studio, y déplacer un
  bloc d'un écran, changer une couleur dans `Tokens.qml`, remplacer un ornement par un autre SVG ;
  relancer le jeu avec `JADG_QML_FROM_SOURCE=1` et voir les trois changements, **sans qu'aucun
  compilateur C++ n'ait été lancé**.
- **Test du développeur** — le `git diff` du test précédent ne touche que `Source/Ui/**` et
  `Source/Elements/**`. Aucun `.cpp`, aucun `.h`, aucun fichier engendré.
- Deux exécutables : `JustAnotherDnDGame` (Qt Quick, ne lie pas `Qt6::Widgets`) et `LevelEditor`.
- `scripts/check_ui_layers.py` vert, et les quatre garde-fous devenus sans objet retirés.
- Build `/W4 /WX` sans avertissement, `ctest` vert, `qmllint --strict` et `qmlformat --verify`
  propres, lint d'exigences et lint des lots verts.
