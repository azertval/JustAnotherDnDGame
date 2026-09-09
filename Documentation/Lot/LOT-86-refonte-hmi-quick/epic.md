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

### Attention : le texte des maquettes était périmé

Les planches nommaient la direction retenue « **Ambre nuit** » et la décrivaient comme « votre
bleu-nuit et votre ambre `#ffd133`, rendus en pixel art ». **Ce n'est plus l'identité du jeu.** Ce
texte datait du `LOT-68` ; les `LOT-66` et `LOT-76` ont depuis remplacé la portée identité par le
**parchemin de Tanares**, dont chaque teinte est *relevée* sur les feuilles de personnage de la
source — parchemin vieilli `#d0c0a0`, encre sépia `#302000`, or des filets `#c0a060`, grenat des
cabochons `#701010`. `#ffd133` n'est plus que l'accent du **châssis d'édition**.

C'est une illustration de ce que ce lot corrige : la maquette avait cessé de décrire le jeu, et le
seul contrôle qui les reliait comparait les couleurs — pas les mots. `Source/Ui/Theme/Tokens.qml`
porte désormais ces valeurs, à un seul endroit, et il n'y a plus de second texte à laisser périmer.

*Le prix, assumé et toujours vrai* : le parchemin est chaleureux et lisible, mais peu mémorable.
Personne ne reconnaîtra le jeu à sa capture d'écran.

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

## Exigences couvertes

Nouvelles, toutes vérifiées par `scripts/check_ui_layers.py` — c'est la condition pour qu'elles
soient des règles et non des intentions :

- [`EX-IHM-100`](@ref EX-IHM-100) — une modification purement visuelle, sans toucher au C++ ;
- [`EX-IHM-101`](@ref EX-IHM-101) — la présentation ne connaît ni Qt Quick ni Qt Widgets ;
- [`EX-IHM-102`](@ref EX-IHM-102) — le jeu ne lie pas `Qt6::Widgets` ;
- [`EX-IHM-103`](@ref EX-IHM-103) — écrans et contrôles en `.ui.qml`, sans code impératif ;
- [`EX-IHM-104`](@ref EX-IHM-104) — imports connus à la fois de Qt et de Qt Design Studio ;
- [`EX-IHM-105`](@ref EX-IHM-105) — aucun littéral d'apparence hors du thème.

Réutilisées et **verrouillées** par le même contrôle, sans être redéclarées :
[`EX-ARCH-001`](@ref EX-ARCH-001) et [`EX-NFR-010`](@ref EX-NFR-010) — `Core` sans un seul en-tête
Qt, vrai depuis le `LOT-01` et qu'un seul `QString` suffirait à rendre faux.

## Où en est le lot

**Fait et vérifié.** Deux binaires séparés — le jeu en Qt Quick (`QGuiApplication`, ne lie pas
`Qt6::Widgets`), l'éditeur inchangé en Qt Widgets. `Source/Ui` avec son projet Qt Design Studio et
`Tokens.qml` écrit à la main. L'édition d'un écran **sans reconstruction**, prouvée de bout en bout.
`hmi::SceneResources`, la grappe de ressources QRhi commune aux deux surfaces. Une tranche verticale
complète — vue-modèle, formulaire `.ui.qml`, écran affichant les vraies données du personnage de
démonstration. Et les six garde-fous, dont chacun a été vérifié **en mordant**.

**Reste à faire.**

- Les douze autres écrans (inventaire, journal, carte, dialogue, marchand, guilde, HUD de combat,
  compagnie, menu, options, pause, crédits). Le motif est établi et éprouvé ; ce qui suit est de la
  répétition, pas de la conception.
- Le portage du viewport sur `QQuickRhiItem`. Reporté sciemment : c'est le morceau le plus risqué —
  `QQuickRhiItem` rend sur un **fil séparé** là où `QRhiWidget` est mono-fil, et la simulation doit
  rester sur le fil graphique en passant sa `ComposedScene` au renderer, ce qui suppose de séparer
  la composition de la soumission, aujourd'hui faites en une passe.
- La localisation sur `.ts`/`.qm`, et la suppression de `Source/HMI/Interface`.
- Les guides (`guide-ihm-qt`, `guide-design-ihm`, `guide-conception-qds`).

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
