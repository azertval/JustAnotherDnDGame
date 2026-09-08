# .design-mockups/rpg/

Maquettes des **écrans du RPG**, dans la direction « papiers de mercenaire » : chaque écran de
consultation se lit comme un feuillet détaché d'un registre — souche perforée, en-tête de bureau,
matricule, portrait à coins de montage, tampon encré, contreseing, cachet de cire.

Elles complètent `.design-mockups/` (les écrans de menu, `LOT-66`/`LOT-68`) sans le remplacer : la
palette, la typographie et la recette d'encadrement sont **les mêmes**, celles de
`hmi::identityTokens()`.

## Ce qui est source, ce qui est généré

| Fichier | Rôle |
|---|---|
| `Main.dc.html` | Fiche de personnage — le feuillet d'enrôlement |
| `Dotation.dc.html` | Inventaire et équipement — le bordereau de dotation |
| `Combat.dc.html` | ATH de combat — le seul écran qui ne suspend pas le jeu, donc des bandeaux posés sur la carte tactique |
| `Systeme.dc.html` | La grammaire : palette, deux familles, les pièces du document, les cinq contenants, trois règles |
| `Assets.dc.html` | L'état des lieux des illustrations : 308 clés attendues, six familles à ajouter, ce qui reste tracé |
| `canvas.json` | Mise en page du canevas : deux pages, cinq planches, deux annotations |

`papiers-de-mercenaire.html` est **généré** et n'est pas versionné (≈ 2,5 Mo : l'éditeur de canevas
y est embarqué). Il se réassemble depuis les sources ci-dessus :

```powershell
& "C:\Program Files\nodejs\node.exe" "<skill>\seed-canvas.mjs" `
  --template "<skill>\payload.template.html" `
  --out papiers-de-mercenaire.html --title "Papiers de mercenaire" `
  --artboard Main.dc.html --artboard Dotation.dc.html --artboard Combat.dc.html `
  --artboard Systeme.dc.html --artboard Assets.dc.html --canvas canvas.json
```

Node ou Bun est nécessaire — c'est ce qui manquait au poste quand `.design-mockups/README.md` a
écarté le canevas éditable ; l'obstacle est levé.

## Conventions tenues

- Les **libellés** sont ceux de `Source/Elements/Localization/fr.lang`, mot pour mot.
- Les **valeurs** sont dérivées des données réelles de `demonstration-brenna.json` et des
  catalogues (demi-elfe : Charisme +2 ; *Brawler* : dé de vie d12 → 32 points de vie au niveau 3 ;
  cuir clouté + bouclier + Dextérité → classe d'armure 15).
- Ce que **rien n'alimente** reste au tiret cadratin, jamais à zéro — la convention du `LOT-38`.
- Les **teintes** sont les neuf rôles attestés du `LOT-66`, sans exception.

## Ce que ces planches ne sont pas

Elles ne décrivent pas l'ossature en données de `hmi::rpgScreens()` : elles proposent ce qu'elle
deviendrait. Les blocs, leurs genres et leur ordre y sont respectés, mais un feuillet d'identité
demande des *widgets* que le châssis ne peint pas encore — tuile d'aptitude, jauge, jeton, écu.
C'est la table qui reste le contrat ; ce sont les peintres qui changeraient.
