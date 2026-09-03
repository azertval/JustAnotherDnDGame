# LOT-65 — Refonte des niveaux de démonstration {#lot-65}

> Statut : **en cours** — TACHE-00 à TACHE-09 faites, TACHE-10 en cours (tout fait sauf le
> parcours manuel complet, réservé à un humain). Prérequis : [LOT-63](@ref lot-63) (mécanismes manquants),
> [LOT-64](@ref lot-64) (cadrage de caméra), [LOT-59](@ref lot-59) (séquence en donnée de contenu).
> **Dernier lot de contenu** du programme `0.1.0`.

## Objectif
Refaire le contenu livré pour qu'il **exploite** et **teste** toutes les mécaniques du moteur.

Les dix-sept tableaux actuels ont été conçus au `LOT-25`, quand le moteur en était à
`demo-salles` — le `LOT-63`, fusionné depuis, en a porté le nombre de quinze à dix-sept
(`demo-cle`, `demo-plateforme`).
Depuis, quinze lots ont ajouté : dangers directionnels, mobiles, commutés et temporisés
(`LOT-31`), fonds de niveau, décors libres avec parallaxe, calque de premier plan (`LOT-44`,
`LOT-49`), skins et raccords automatiques (`LOT-42`), textures par instance (`LOT-45`), animations
pilotées par données (`LOT-46`, `LOT-47`), ombres du plan physique (`LOT-55`) — et le programme
`0.1.0` y ajoutera clé, porte verrouillée, action « Interagir », plateforme mobile (`LOT-63`) et
trois modes de cadrage (`LOT-64`).

Le contenu n'a pas suivi. Il en résulte deux problèmes distincts :

1. **Des mécaniques livrées, testées unitairement, et jamais jouées.** Rien ne garantit qu'un type
   de tuile ou un mode de cadrage apparaisse dans un seul tableau franchi par le test système.
   Une mécanique qui n'est dans aucun niveau n'est vérifiée qu'en isolation — jamais dans une
   partie réelle.
2. **Une vitrine qui ne montre pas le jeu.** Les tableaux sont des bancs d'essai fonctionnels : nus,
   sans fond, sans décor, sans habillage. Le programme d'habillage `LOT-40` → `LOT-55` est
   entièrement livré et **invisible** dans le contenu qui accompagne le binaire.

## Périmètre

### Inclus
- **Inventaire exhaustif** des mécaniques : tous les `core::TileType`, tous les modes de cadrage,
  tous les paramètres significatifs (danger temporisé déphasé, bloc réduit, budget de mouvements…).
- **Garde-fou de couverture** (`EX-LVL-015`) : un contrôle automatique échoue si une mécanique
  livrée n'apparaît dans **aucun** niveau franchi par le test système. Même esprit que
  `scripts/check_demo_sequence.py`, appliqué au contenu plutôt qu'à l'ordre.
- **Refonte des tableaux existants** : progression de difficulté revue, une mécanique introduite à
  la fois (`niveaux.md`, section Conception), habillage complet — fond, décors, skins, animations,
  cadrage choisi.
- **Tableaux manquants** pour les mécaniques non couvertes.
- **Tableaux de synthèse** combinant plusieurs mécaniques, là où les tableaux actuels les isolent
  toutes.

### Exclus (hors périmètre de ce lot)
- Nouvelles mécaniques : ce lot **utilise** ce qui existe, il n'ajoute rien au moteur. Toute
  mécanique manquante relève du [LOT-63](@ref lot-63).
- Correction de défauts découverts : ils sont **consignés**, pas corrigés ici (cf. décisions de
  cadrage). **Deux exceptions nommées**, décidées au second temps du lot (`TACHE-06`) : la porte
  écrasante mortelle et le bloc pouvant enfoncer une plaque. Sans elles, ni le tutoriel de la clé ni
  l'énigme du tableau final ne tiennent — l'assouplissement est délibéré et borné à ces deux points.
- Création d'assets d'art originaux : l'habillage réutilise les skins, décors et fonds existants,
  complétés par les scripts de génération procédurale du dépôt.
- Campagne scénarisée, narration, progression de difficulté calibrée par des joueurs tiers.

## Décisions de cadrage
- **Base de travail remise à zéro avant le garde-fou (`TACHE-00`).** La banque d'assets était trop
  pauvre pour habiller quoi que ce soit (un seul fond, deux décors, un jeu de skins réels limité à
  trois types), et réviser les dix-sept tableaux hérités du `LOT-25` tableau par tableau aurait
  ancré chaque décision dans la géométrie existante plutôt que dans ce que `TACHE-01` exige. La
  banque est donc renforcée et les tableaux existants retirés **avant** le garde-fou, sur la
  branche dédiée du lot — la CI n'est pas verte pendant cette étape, et ce n'est pas son critère.
- **Le garde-fou d'abord, le contenu ensuite.** Écrire les tableaux puis constater la couverture,
  c'est se fier à une relecture. Le contrôle automatique est livré en `TACHE-01`, **rouge**, et
  c'est lui qui pilote le reste du lot : la liste de ce qu'il refuse est la liste du travail.
- **Ce lot est un révélateur, pas un correcteur.** Jouer réellement toutes les mécaniques
  ensemble fera émerger des défauts — c'est même son intérêt principal. Ils sont consignés et
  traités hors du lot : mélanger création de contenu et corrections de moteur rendrait les deux
  impossibles à relire, et un échec du test système inexploitable.
- **Refondre plutôt qu'ajouter.** Empiler de nouveaux tableaux sur les dix-sept existants aurait
  donné une séquence longue et incohérente, où les premiers tableaux sont nus et les derniers
  habillés — c'est pourquoi `TACHE-00` les retire tous avant que `TACHE-02`/`TACHE-03` en
  recréent, en s'appuyant sur ce que `TACHE-01` exige plutôt que sur la géométrie héritée. La
  séquence étant devenue une **donnée** (`LOT-59`), la reconstruire ne coûte plus de recompilation.
- **Une mécanique à la fois, puis des synthèses.** C'est la ligne directrice déjà écrite dans
  `niveaux.md` ; les tableaux actuels respectent la première moitié (isolement) et pas la seconde
  (combinaison), alors que c'est la combinaison qui fait un jeu — et qui trouve les défauts
  d'interaction entre mécanismes.
- **Placé en dernier parmi les lots de contenu**, parce qu'il consomme tout ce que les autres
  produisent : mécanismes du `LOT-63`, cadrage du `LOT-64`, écrans et progression du `LOT-59`, son
  du `LOT-60`, effets du `LOT-53`.

## Exigences couvertes
- Nouvelle : `EX-LVL-015` (couverture exhaustive des mécaniques par le contenu livré, vérifiée
  automatiquement).
- Réutilisées : `EX-LVL-012` (niveaux de démonstration à difficulté croissante), `EX-LVL-013`
  (séquence en donnée), `EX-LVL-006` (cadrage porté par le niveau), `EX-NFR-021` (test système de
  franchissabilité), `EX-GP-020` à `EX-GP-026` (mécanismes), `EX-GP-050` à `EX-GP-053` (dangers
  avancés), `EX-REN-044` (fond de niveau), `EX-DEC-002` (couches de décor), `EX-REN-005`
  (animations par données), `EX-REN-045` (ombres), `EX-VIS-005` (au moins trois niveaux enchaînés).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-00](tache-00-preparation-assets-niveaux.md) | Préparation : banque d'assets renforcée et remise à zéro des niveaux | `Source/Elements/Assets`, `scripts`, `Source/Elements/Levels` | ✅ |
| [TACHE-01](tache-01-inventaire-garde-fou.md) | Inventaire des mécaniques et garde-fou de couverture | `scripts`, `Source/Test` | ✅ |
| [TACHE-02](tache-02-refonte-tableaux.md) | Refonte des tableaux existants : progression et habillage | `Source/Elements/Levels` | ✅ |
| [TACHE-03](tache-03-tableaux-manquants-syntheses.md) | Tableaux manquants et tableaux de synthèse | `Source/Elements/Levels` | ✅ |
| [TACHE-04](tache-04-documentation-verification.md) | Documentation et vérification | `Source/Test`, `Documentation` | ✅ |
| [TACHE-05](tache-05-doctrine-gardes-fous-profondeur.md) | Doctrine de conception et garde-fous de profondeur | `Documentation/Specification`, `Source/Test` | ✅ |
| [TACHE-06](tache-06-croisements-mecaniques.md) | Croisements de mécaniques et corrections moteur associées | `Source/Core/Gameplay`, `Source/Test/Integration` | ✅ |
| [TACHE-07](tache-07-actes-mouvement-mecanismes.md) | Actes I et II : mouvement et mécanismes | `Source/Elements/Levels`, `Source/HMI/Game` | ✅ |
| [TACHE-08](tache-08-actes-terrain-dangers.md) | Actes III et IV : terrain, dangers et plateformes | `Source/Elements/Levels` | ✅ |
| [TACHE-09](tache-09-syntheses-final.md) | Tableaux de synthèse et final multi-salles | `Source/Elements/Levels` | ✅ |
| [TACHE-10](tache-10-documentation-verification-finale.md) | Documentation et vérification finale | `Documentation`, `Source/Elements/Levels` | 🔄 |

### Second temps du lot : de la couverture à la profondeur

Les `TACHE-00` à `TACHE-04` ont rendu le garde-fou de couverture **vert** : chaque type de tuile et
chaque mode de cadrage livrés apparaissent dans un tableau franchi par le test système. Une revue
des vingt-deux tableaux obtenus établit que cela ne suffisait pas — et que la `TACHE-01` l'avait
elle-même annoncé (« couvert ≠ franchi ») sans qu'on en tire les conséquences :

- **10 tableaux sur 22** se franchissent sans jouer : neuf en maintenant « droite », et
  `demo-plateforme` sans **aucune** entrée.
- Chaque mécanique n'existe qu'en **une seule instance** dans toute la séquence.
- Quatre tableaux posent leur sujet **hors d'atteinte** : plafonds inclinés, concaves de plafond et
  la totalité des dangers avancés, interrupteur compris — le `dangerSwitched` livré ne peut donc
  jamais être commuté.
- `dashBudget`, les `zones` de caméra (`EX-LVL-007`) et la taille de salle par niveau
  (`EX-REN-017`) ne sont employés par **aucun** tableau.
- `demo-final` combine quatre mécaniques sans qu'aucune n'en croise une autre, et n'est pas le
  dernier tableau joué.

Les critères d'acceptation n°2 et n°4 (franchissabilité, synthèses) étaient donc satisfaits à la
lettre et pas dans leur intention. Les `TACHE-05` à `TACHE-10` reprennent le lot là : une doctrine
de conception écrite, des garde-fous qui mesurent l'**usage** et non la présence, et une refonte
complète pilotée par eux. Deux corrections moteur bornées y sont assumées (porte écrasante mortelle,
bloc pouvant presser une plaque) : sans elles, ni le tutoriel de la clé ni l'énigme du final ne
tiennent — c'est un assouplissement délibéré du cadrage « ce lot n'ajoute rien au moteur ».

## Critères d'acceptation du lot
1. Le garde-fou de couverture est **vert** : chaque type de tuile et chaque mode de cadrage livrés
   apparaissent dans au moins un tableau franchi par le test système.
2. Chaque tableau de la séquence est **franchissable** de bout en bout (`EX-NFR-021`).
3. Chaque tableau est **habillé** : fond, skins, et cadrage explicitement choisi.
4. La séquence introduit une mécanique à la fois, puis les combine dans des tableaux de synthèse.
5. Retirer une mécanique de tous les tableaux fait **échouer** le garde-fou — démontré par un test
   négatif.
6. Les défauts découverts en jouant sont consignés, avec leur reproduction, sans être corrigés dans
   ce lot.
7. Build `/W4 /WX`, `ctest` à 100 %, Doxygen et lints verts.

## Dépendances
Consomme [LOT-59](@ref lot-59) (séquence en donnée, progression), [LOT-60](@ref lot-60) (son),
[LOT-53](@ref lot-53) (effets), [LOT-63](@ref lot-63) (mécanismes) et [LOT-64](@ref lot-64)
(cadrage). Bâtit sur [LOT-25](@ref lot-25), dont il reprend et étend la démarche. Précède
[LOT-66](@ref lot-66), qui clôt le programme.

## Navigation des tâches
- @subpage lot-65-tache-00-preparation-assets-niveaux
- @subpage lot-65-tache-01-inventaire-garde-fou
- @subpage lot-65-tache-02-refonte-tableaux
- @subpage lot-65-tache-03-tableaux-manquants-syntheses
- @subpage lot-65-tache-04-documentation-verification
- @subpage lot-65-tache-05-doctrine-gardes-fous-profondeur
- @subpage lot-65-tache-06-croisements-mecaniques
- @subpage lot-65-tache-07-actes-mouvement-mecanismes
- @subpage lot-65-tache-08-actes-terrain-dangers
- @subpage lot-65-tache-09-syntheses-final
- @subpage lot-65-tache-10-documentation-verification-finale
