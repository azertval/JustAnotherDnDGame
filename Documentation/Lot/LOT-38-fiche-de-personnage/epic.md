# LOT-38 — Fiche de personnage : maquette et interface {#lot-38}

> Statut : **fait** (vérification automatisée : build `/W4 /WX` sans avertissement, `ctest` à
> 1028/1028, `clang-format`, les huit lints, cahier de test et Doxygen verts ; vérification
> manuelle : la fiche ouverte, remplie et parcourue dans l'application).
>
> **Repris** depuis : la feuille n'est plus recomposée en widgets, elle est la **planche gravée** du
> livre, relettrée dans la langue active (voir « Reprise » plus bas).
> Prérequis : [LOT-13](@ref lot-13) (fiche de personnage), [LOT-68](@ref lot-68) (châssis des
> écrans), [LOT-76](@ref lot-76) (ornements tracés).
>
> Aucune exigence ajoutée : `EX-IHM-090` et `EX-IHM-091` du [LOT-68](@ref lot-68) couvrent ce lot,
> et c'est leur premier vrai emploi.

## Objectif

Le [LOT-68](@ref lot-68) a livré neuf écrans vides. Celui-ci en remplit un : la **fiche de
personnage**, relevée sur la maquette du corpus, et alimentée par un personnage réel.

## La maquette : cinq planches, et ce qu'on relève dessus

La source est **`VTT/Blank Sheets RPG (1).pdf`** — les cinq feuilles Tanares **vierges**, arrivées
avec les ressources de table virtuelle. Vierges, et non remplies : un exemple rempli cache ses
propres cadres, et une mise en page se relève sur ce qui la montre. `Character_Sheets_Tanares.pdf`,
déjà au corpus, porte les mêmes feuilles remplies ; `VTT/RPG Sheets - BW print (2).pdf` en donne la
structure à plat.

Les cinq planches sont dans le périmètre :

| Planche | Où elle atterrit |
|---|---|
| 1 — identité, caractéristiques, compétences, sauvegardes, combat, attaques, personnalité | Fiche de personnage |
| 2 — apparence, équipement, langues, biographie | Fiche de personnage |
| 3 — dons, traits et pouvoirs | Fiche de personnage |
| 4 — incantation et emplacements de sorts | Fiche de personnage |
| 5 — **feuille d'équipe** : blason, quartier général, mécénat | Un **neuvième écran** |

**La cinquième planche n'est pas la fiche d'un personnage.** C'est celle de son équipe — renommée,
blason, installations, dessein caché. La ranger dans la fiche aurait mêlé deux sujets sur un même
écran ; elle a donc son écran. Et cet écran est la **preuve** de ce que le `LOT-68` affirmait : il
s'ajoute par une entrée de table et ses clés de traduction, sans qu'aucun des huit autres, ni la
feuille de style, ni le châssis, n'aient été touchés (`EX-IHM-090`).

## Un champ affiché, ou écrit comme non alimenté

Le critère d'acceptation demandait que chaque champ de la maquette soit **affiché** ou **inscrit
dans une liste explicite** de champs hors périmètre — « un champ simplement oublié n'est pas un
arbitrage ». Cette liste n'est pas un document à côté : elle est **dans la table**.

Chaque ligne de l'ossature porte un identifiant de valeur, ou une chaîne vide. Vide veut dire :
*ce champ existe sur la feuille, il est à l'écran, et rien ne l'alimente encore*. Il reste au tiret
cadratin. La colonne des identifiants vides **est** le périmètre restant, lisible d'un coup d'œil,
et impossible à laisser dériver — un champ ajouté sans source se voit à la lecture de la table.

Ce que rien n'alimente aujourd'hui, et pourquoi :

| Champs | En attente de |
|---|---|
| Alignement, nom du joueur | rien ne les porte : ce sont des champs de table, pas d'état de jeu |
| Inspiration | une ressource que rien n'accorde ni ne dépense |
| Points de vie temporaires, jets contre la mort | l'agonie du `LOT-72` |
| Apparence (âge, taille, poids, yeux, peau, cheveux) | un portrait de personnage, `LOT-39` |
| Traits, idéaux, liens, défauts, biographie | le contenu narratif du `LOT-27` |
| Emplacements d'équipement, attaques | l'inventaire du `LOT-14` |
| Dons, traits et pouvoirs | le socle de classe du `LOT-47` |
| Incantation et emplacements de sorts | les sorts du `LOT-35` |
| Toute la feuille d'équipe | la Guilde du `LOT-45` et sa progression, `LOT-83` |

**Aucune de ces cases n'affiche zéro.** Un « 0 » se lirait comme un état du jeu — un personnage
sans emplacement de sort disponible, une CA nulle — et mentirait. Le tiret dit « on ne sait pas
encore », ce qui est la vérité.

## Reprise : la planche gravée, et non son imitation

La première version de cet écran **recomposait** la feuille en widgets Qt — cartouches peints, roue
des caractéristiques tracée au pinceau, encadrements de parchemin. Elle en avait l'allure sans en
venir : l'arc des six cadrans était réglé à la main, les bandeaux étaient des rectangles, et les
rinceaux, la rose des vents et les cabochons n'existaient pas. C'était le mieux qu'on pouvait faire
d'une maquette qu'on ne pouvait que **regarder**.

La planche 1 a depuis été **vectorisée** (`Character_Sheets_Tanares.svg`, corpus). L'écran ne la
réinvente donc plus : il la **dessine**, et se contente d'écrire dessus. Ce qui était approché est
exact. `hmi::AbilityWheel` a disparu — la roue est gravée, elle n'est plus peinte — et
`CharacterSheetPage.ui` avec elle : une gravure se repère en **coordonnées de maquette**, pas en
dispositions imbriquées.

### Le lettrage est à nous, l'ornement est au livre

La gravure porte tout, y compris **son lettrage, en anglais**. La livrer telle quelle aurait figé
« Acrobatics » et « BACKGROUND » dans l'image, donc dans les deux langues à la fois — et l'écran
serait redevenu anglais le jour même où le lexique du [LOT-30](@ref lot-30) venait de le traduire.

`scripts/build_character_sheet_plate.py` retire donc le lettrage du tracé. Le fichier est un
**calque d'encre** : un unique chemin noir de 21 906 sous-chemins, sans balise `<text>` à supprimer
ni calque à masquer — une lettre y est un sous-chemin comme un filet de cadre. Ce qui les distingue
est leur **étendue** : une lettre tient dans le rectangle de son intitulé, un filet de cadre le
traverse de part en part. Le script retire les sous-chemins **entièrement contenus** dans l'un des
rectangles de la table, et garde tous les autres — 2 565 retirés, 19 341 conservés.

### Une seule table pour les deux moitiés

Les rectangles ne sont écrits ni dans le script ni dans le C++ : ils vivent dans
`character-sheet-plate.json`, que le script lit pour **effacer** et que l'écran lit pour **reposer**.
Une seule source, donc, et une propriété qui en découle : il est impossible d'effacer un intitulé
sans le reposer, ni d'en reposer un sur une gravure restée en place. Un rectangle déplacé déplace
les deux.

C'est la même idée que la colonne des identifiants vides — le périmètre est **dans la table**, pas
dans un document tenu à côté.

### Deux échappatoires, et pourquoi elles existent

Deux cas ne se laissent pas découper au rectangle, et ils sont nommés plutôt que rattrapés en
silence :

- **Un lettrage évidé dans son propre fond.** Sur une poignée de cartouches — les jets de
  sauvegarde, la perception passive — les lettres sont un trou *dans* le fond du cartouche : les
  retirer le **bouche** au lieu de les effacer. Ces cartouches portent donc un rectangle `cover`,
  que l'écran repeint en parchemin avant d'écrire. Le fond y étant plat, la reprise est invisible.
- **Un onglet plus haut que large.** Les six onglets de sauvegarde portent leur abréviation tournée
  d'un quart de tour ; treize unités de large ne reçoivent rien d'horizontal. Une étiquette Qt ne
  tourne pas : ces trois lettres sont **peintes**, et c'est le seul texte de la planche qui le soit.

### La planche est un masque d'encre, et non un SVG

Ce n'est pas un choix de confort : **Qt ne sait pas rendre ce tracé.** `QSvgHandler` rejette tout
`<path>` de plus de 32 768 éléments — il ne le tronque pas malgré son message, il le jette, et la
planche s'affiche entièrement vide. Ce tracé en demande 540 094, seize fois et demie la limite.

Le découper en plusieurs chemins ne marche pas davantage, et c'est le point intéressant : un
remplissage — non-nul comme pair-impair — se calcule sur l'**ensemble** des contours d'un même
chemin. Le blanc d'un cartouche, le trou d'un anneau, l'intérieur du cadre de page n'existent que
parce qu'un autre contour du même chemin y annule le premier. Trois découpes ont été essayées —
dans l'ordre du fichier, par arbre quaternaire sur le centre, puis en mariant chaque forme aux
contours qu'elle contient — et les trois rendent la même image fausse. La raison est visible dans
la donnée : le contour du **cadre de page** fait à lui seul 24 535 points, 75 % du budget d'un
chemin, et il enveloppe toute la feuille. Aucune coupe ne peut le laisser du même côté que tout ce
qu'il contient.

La planche est donc rendue **une fois** en PNG, à trois fois la taille de la maquette. Elle n'y perd
rien : elle est monochrome, son alpha est intact, et l'écran la teinte au jeton d'encre exactement
comme il aurait teinté le SVG — la reprise suit donc le thème, comme le reste des écrans du jeu.

## Ce que la fiche calcule, et où

`hmi::characterSheetValues` est une fonction **pure** : d'une `core::CharacterSheet` et des
catalogues, elle rend une table `identifiant → texte`. Aucun widget, aucun fichier. C'est ce qui
permet de vérifier par test qu'un modificateur s'affiche `+3` et non `3`, qu'une sauvegarde
maîtrisée compte son bonus, et que la Perception passive vaut 10 + le modificateur — sans ouvrir de
fenêtre.

Trois décisions d'affichage y sont prises, et chacune répond à une ambiguïté de lecture :

- **le signe des modificateurs** : `3` nu se lit comme une valeur de caractéristique, et les deux se
  côtoient sur la même ligne ;
- **les points de vie contre leur maximum** (`27 / 32`) : `27` seul ne dit pas si le personnage va
  bien ;
- **la maîtrise se voit** (`+4 •`) : c'est ce que porte la pastille cochée de la feuille, et le seul
  moyen de la rendre dans une ligne de texte.

Le lien entre les deux côtés — ce que la fonction produit, ce que l'ossature attend — est le seul
contrat, et un test le vérifie dans les deux sens. Une faute de frappe d'un côté ne se verrait
sinon qu'à l'écran, sous la forme d'un champ resté au tiret au milieu de champs remplis,
c'est-à-dire pas du tout.

## Le personnage affiché : une donnée, et un échafaudage assumé

Un écran de fiche qui n'affiche aucune fiche ne se relit pas. Il n'y a pourtant ni groupe
([LOT-29](@ref lot-29)) ni sauvegarde ([LOT-17](@ref lot-17)) d'où tirer un personnage réel.

Ce lot livre donc **`Rpg/characters/demonstration-brenna.json`** : une demi-elfe de niveau 3,
cartographe, avec son schéma (`character.schema.json`) et sa validation en intégration continue.

Le fichier ne porte que des **choix** — espèce, classe, historique, valeurs de caractéristique
avant augmentation, niveau. Rien de dérivé : ni points de vie, ni classe d'armure, ni valeurs
finales. Le moteur les calcule (`core::buildCharacterSheet`, `LOT-13`), et le niveau s'atteint par
**gain d'expérience**, le chemin qu'une partie empruntera. Écrire ces valeurs dans le fichier en
aurait fait une seconde source, qui aurait différé de la première au premier ajustement de règle —
et personne n'aurait su laquelle croire.

Le personnage est **déclaré provisoire**, avec son critère de retrait (`EX-CNT-032`) : il disparaît
le jour où une partie en fournit un vrai. C'est aussi ce qui l'autorise à référencer l'une des
quatre classes provisoires du [LOT-36](@ref lot-36) — les seules qui existent — sans casser le
garde-fou qui interdit aux données **définitives** de le faire. Ce garde-fou a été précisé, pas
contourné : une donnée provisoire porte son propre critère de retrait, et disparaît donc avec ce
qu'elle référence.

## Le lexique tient maintenant les compétences et les caractéristiques

Les dix-huit compétences et les six caractéristiques s'affichent, donc s'écrivent dans le catalogue
de traduction. Or ce sont des **termes de règle** — « Escamotage », pas « Tour de main » — et le
lexique du [LOT-30](@ref lot-30) les porte déjà. Les réécrire sans lien avec lui était exactement le
défaut que `check_glossary.py` existe pour empêcher : ses deux nouveaux espaces de noms
(`rpg.skill.`, `rpg.ability.`) portent le contrôle de 0 à **24 clés de règle**.

## Ce que le lot ne fait pas

**Il n'édite rien.** La fiche se lit ; créer ou modifier un personnage est un autre sujet, et il
n'a pas de lot à ce jour.

**Il ne traduit pas les catalogues.** Espèce, classe et historique s'affichent avec le nom que
porte leur donnée — « Brawler », « Cartographer » — parce que les catalogues Tanares sont en
anglais. C'est le sujet du lexique et des lots de contenu, pas de l'écran qui les affiche.

**Il ne remplit pas les huit autres écrans.** Ils gardent leurs tirets, et leurs lots sont nommés
ci-dessus.

## Critères d'acceptation

- **Chaque champ de la maquette est affiché, ou déclaré sans source** — dans la table, à sa ligne,
  et non dans une liste tenue à part. ✔
- **Les cinq planches sont couvertes** : quatre dans la fiche, la cinquième dans un neuvième écran
  qui n'a demandé de toucher à aucun des huit. ✔
- **Les valeurs affichées sont calculées par la règle**, pas recopiées : modificateurs, maîtrises,
  points de vie, Perception passive, bonus de maîtrise. ✔
- **Le personnage affiché est une donnée validée** contre son schéma, provisoire et datée de son
  critère de retrait. ✔
- `ctest` : **1028/1028**. ✔
- **La planche affichée est la gravure du livre**, débarrassée de son lettrage anglais et
  relettrée dans la langue active — et la table qui efface est celle qui repose. ✔
