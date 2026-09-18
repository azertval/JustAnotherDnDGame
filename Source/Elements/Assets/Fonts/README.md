# Elements/Assets/Fonts/

Polices TTF de l'application et leurs licences, déployées à côté de l'exécutable. Aucune police
n'est dessinée dans la scène rendue : tout le texte passe par Qt (widgets de l'éditeur, écrans
Qt Quick du jeu).

## Police de l'interface hors-jeu

`Inter-Regular.ttf` et `Inter-Bold.ttf` sont la police **embarquée de l'IHM Qt** (menus, panneaux,
boîtes de dialogue), enregistrées auprès de `QFontDatabase` au démarrage (`hmi::applyEditorTheme`) ;
famille couvrant les caractères accentués français du catalogue de traduction (`EX-REN-033`) et
lisible aux petites tailles des libellés de panneaux.

**Repli** : si l'un des deux fichiers est absent ou refusé par Qt, l'application retombe sur une
famille générique demandée à Qt (jamais un second nom de police codé en dur), en journalisant un
avertissement (`EX-NFR-040`).

**Licence** : [Inter](https://github.com/rsms/inter) est distribuée sous licence
[SIL Open Font License 1.1](https://scripts.sil.org/OFL), redistribuable avec l'application ;
texte complet dans `Inter-LICENSE.txt`, à côté des fichiers.

## Polices de l'interface Qt

Elles sont chargees par `hmi::applyFont` (`HMI/Interface/ApplicationTheme.cpp`)
depuis ce dossier, deploye a cote de l'executable.

| Fichier | Role | Employe par |
|---|---|---|
| `Inter-{Regular,Bold}.ttf` | `FontRole::Ui` | Chassis d'edition : panneaux, tables, arbres, boites de dialogue. Police **par defaut** de l'application. |
| `PixelifySans-{Regular,Bold}.ttf` | `FontRole::Identity` | Charte v1 (`LOT-68`, `EX-IHM-070`) : encore enregistree par `applyFont`, lue par aucun ecran depuis la charte v2 (`LOT-87`). |
| `PressStart2P-Regular.ttf` | `FontRole::Identity` | Charte v1, titres d'ecran : meme etat que `PixelifySans`. |

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

Enregistrees par `registerIdentityFonts()` (`App/Game/Main.cpp`) : ce sont les polices des ecrans
du jeu. `Tokens.qml` designe les familles par leur nom
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
