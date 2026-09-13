# Elements/Assets/Fonts/

Police bitmap utilisée pour le texte affiché **dans la scène rendue** (HUD, `LOT-52`,
`EX-REN-032`) : `hmi::BitmapFont` cherche ici, au démarrage, un atlas de glyphes accompagné de
ses métriques.

## Fichiers attendus

- `font.png` — atlas de glyphes (`hmi::AssetFamily::Font`, dimensions libres, découpées par les
  métriques plutôt que par une grille de cases).
- `font.json` — métriques : la région (en pixels) et l'avance horizontale de chaque caractère
  couvert.

```json
{
  "version": 1,
  "lineHeight": 10,
  "replacement": "?",
  "glyphs": [
    { "char": "A", "x": 0, "y": 0, "width": 6, "height": 10, "advance": 6 }
  ]
}
```

`"replacement"` est optionnel (`?` par défaut) : c'est le glyphe substitué à un caractère non
couvert (`EX-NFR-040`), jamais un trou silencieux ni un plantage. Chaque entrée de `"glyphs"`
désigne exactement **un** caractère (`"char"`), en UTF-8 — les lettres accentuées du français
(`é è à ç ù ê î ô û`, minuscules et majuscules) doivent être couvertes, faute de quoi le HUD en
français serait illisible.

## Absence d'asset : repli procédural

Aucun fichier n'est déposé dans ce dossier pour l'instant : `hmi::BitmapFont` retombe sans
plantage sur une police procédurale minimale (`hmi::buildProceduralFont`, glyphes 5×7 pixels,
ASCII imprimable + accents français), sur le modèle de `atlas.png`/`hmi::buildProceduralAtlasImage`
(`LOT-39`). Le jeu reste lisible même sans aucun asset de police (`EX-NFR-040`).

Un artiste pourra déposer `font.png` + `font.json` ici sans toucher au code : dès que les deux
fichiers sont présents, valides (contrat d'asset, `EX-REN-007`) et cohérents entre eux, ils
remplacent le repli procédural au démarrage suivant.

## Police de l'interface hors-jeu (`LOT-56`)

`Inter-Regular.ttf` et `Inter-Bold.ttf` sont la police **embarquée de l'IHM Qt** (menus, panneaux,
boîtes de dialogue) — sans rapport avec la police bitmap du HUD ci-dessus, décrite plus haut : deux
mécanismes distincts (`hmi::ApplicationTheme` pour l'un, `hmi::BitmapFont`/`hmi::ProceduralFont`
pour l'autre), deux jeux de fichiers distincts. Enregistrées auprès de `QFontDatabase` au démarrage
(`hmi::ApplicationTheme::applyEditorTheme`) ; famille couvrant les caractères accentués français du
catalogue de traduction (`EX-REN-033`) et lisible aux petites tailles des libellés de panneaux.

**Repli** : si l'un des deux fichiers est absent ou refusé par Qt, l'application retombe sur une
famille générique demandée à Qt (jamais un second nom de police codé en dur), en journalisant un
avertissement — même garantie que le repli procédural du HUD (`EX-NFR-040`).

**Licence** : [Inter](https://github.com/rsms/inter) est distribuée sous licence
[SIL Open Font License 1.1](https://scripts.sil.org/OFL), redistribuable avec l'application ;
texte complet dans `Inter-LICENSE.txt`, à côté des fichiers.

## Polices de l'interface Qt

Distinctes de l'atlas ci-dessus : celui-ci sert au texte rendu **dans la scene**, celles-ci au texte
des **widgets**. Elles sont chargees par `hmi::applyFont` (`HMI/Interface/ApplicationTheme.cpp`)
depuis ce dossier, deploye a cote de l'executable.

| Fichier | Role | Employe par |
|---|---|---|
| `Inter-{Regular,Bold}.ttf` | `FontRole::Ui` | Chassis d'edition : panneaux, tables, arbres, boites de dialogue. Police **par defaut** de l'application. |
| `PixelifySans-{Regular,Bold}.ttf` | `FontRole::Identity` | Charte v1 : corps et entrees de menu des ecrans du jeu (`LOT-68`, `EX-IHM-070`), le temps de la phase 3 du `LOT-87`. |
| `PressStart2P-Regular.ttf` | `FontRole::Identity` | Charte v1, **titres d'ecran uniquement** : trop typee pour du corps de texte. |

Les deux familles d'identite sont posees par la feuille de style, cadrees par `objectName`
(`#MainMenu`, `#OptionsPage`, ...). C'est ce cadrage, et lui seul, qui empeche la police pixel de
se repandre dans les tables denses de l'editeur, ou elle serait illisible.

**Repli.** Chaque famille est enregistree independamment : si un fichier manque ou est refuse par
Qt, la feuille de style tombe sur un mot-cle CSS **generique** pour cette famille-la — jamais un
second nom de police en dur (`EX-IHM-052`), et jamais la famille d'un autre role. Une police
d'ecran manquante ne doit pas faire retomber le jeu sur la police de l'editeur.

**Licences.** Les trois sont sous SIL Open Font License 1.1, donc redistribuables avec le jeu ;
chaque `*-LICENSE.txt` accompagne sa famille et doit le rester.

**Accents.** Les trois couvrent `E A E C U OE` accentues et la ponctuation employee par les
catalogues de traduction. C'est le point de rupture d'une police pixel : beaucoup s'arretent a
l'ASCII, et le francais devient illisible sans que rien ne le signale.

## Polices des ecrans du jeu, charte v2 (`LOT-87`, `T2.3`)

Enregistrees par `registerIdentityFonts()` (`App/Game/Main.cpp`), a cote de la charte v1
ci-dessus et non a sa place : les quatorze ecrans existants lisent encore `PixelifySans`/
`PressStart2P` jusqu'a ce que la phase 3 du `LOT-87` les ait tous transcrits en v2 ; T5.2 retire
alors les deux polices pixel de ce dossier. `Tokens.qml` designe les familles par leur nom
(`bodyFamily`, `titleFamily`, `loreFamily`) : Design Studio les voit via `FontFiles` du
`.qmlproject`, qui pointe sur ce dossier entier — aucune liste de fichiers a tenir a jour ailleurs.

| Fichier | Famille rapportee a Qt | Sert a |
|---|---|---|
| `Cinzel-Regular.ttf` | `Cinzel`, style `Regular` | Titres, plaques et bandeaux (`titleFamily`). |
| `Cinzel-SemiBold.ttf` | `Cinzel SemiBold`, style `Regular` | Graisse intermediaire ; famille legacy distincte faute d'entree `STAT` a 600 dans la police source, nom typographique prefere `Cinzel`/`SemiBold` conserve pour les moteurs qui le lisent. |
| `Cinzel-Bold.ttf` | `Cinzel`, style `Bold` | Emphase de titre. |
| `IMFellEnglish-Regular.ttf` | `IM Fell English`, style `Regular` | Corps de texte (`bodyFamily`) et citations (`loreFamily`). |
| `IMFellEnglish-Italic.ttf` | `IM Fell English`, style `Italic` | Citations et texte d'ambiance (`font.italic: true` sur `loreFamily`). |

**Provenance.** Extraites de [google/fonts](https://github.com/google/fonts) (`ofl/cinzel`,
`ofl/imfellenglish`), SIL Open Font License 1.1 — `Cinzel-LICENSE.txt` et
`IMFellEnglish-LICENSE.txt` accompagnent leurs familles. `Cinzel` n'existe en amont qu'en police
variable (axe `wght`, 400 a 900) : les trois graisses deposees ici sont des instances statiques
figees par `fonttools varLib.instancer`, sans modification de dessin — seul le nom de la graisse
600 a ete pose a la main, faute d'instance nommee correspondante en amont. Aucun nom n'est reserve
par les deux licences (pas de `Reserved Font Name` apres la mention de copyright) : cette
derivation reste couverte par l'OFL.

**Repli.** Meme garantie que la charte v1 : un fichier absent ou refuse par Qt journalise un
avertissement (`EX-NFR-040`) et Qt Quick retombe sur la famille demandee telle quelle, resolue par
le systeme — `Tokens.qml` ne connait pas de mot-cle CSS generique, a la difference de la feuille de
style du chassis d'edition.
