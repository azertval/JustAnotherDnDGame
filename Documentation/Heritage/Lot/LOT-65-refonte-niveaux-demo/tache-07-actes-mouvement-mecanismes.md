# TACHE-07 — Actes I et II : mouvement et mécanismes {#lot-65-tache-07-actes-mouvement-mecanismes}

**Lot :** [LOT-65](epic.md) · **Emplacement :** `Source/Elements/Levels`, `Source/HMI/Game` · **Statut :** non commencé

## Contexte
Première moitié de la refonte pilotée par les garde-fous rouges de la `TACHE-05`. Les douze
premiers tableaux forment le tutoriel implicite : ils apprennent le vocabulaire du jeu, chacun sur
une mécanique, avant que les actes suivants ne les croisent.

Le problème n'est pas que ces tableaux soient trop faciles — c'est qu'ils sont **facultatifs** :
l'interrupteur de `demo-interrupteur` est posé sur le trajet direct vers la porte, le bloc de
`demo-bloc` est contournable au double saut, et `demo-bloc-quart` est un couloir entièrement plat où
le bloc n'obstrue rien. La doctrine de la `TACHE-05` inverse la règle : la mécanique devient la
seule issue.

## Travail à réaliser

### Acte I — Mouvement
| # | Tableau | Refonte | Budgets |
|---|---|---|---|
| 1 | `demo-deplacement` | Marche, chute, atterrissage, **sans mort possible** (bas du niveau plein). Seule exception nommée du garde-fou anti-couloir. | — |
| 2 | `demo-saut` | Trois vides de largeur croissante ; le `danger` de base introduit **en fond de fosse**, visible et évitable, de sorte que la première façon de mourir du jeu cesse d'être une surprise. | — |
| 3 | `demo-double-saut` | Trois paliers hors de portée d'un saut simple (≈ 2,4 tuiles). | `dashBudget: 0` |
| 4 | `demo-wall-jump` | Deux puits, large puis étroit. | `dashBudget: 0` |
| 5 | `demo-dash` | Trois couloirs bas (saut impossible), le dernier au-dessus d'un `danger`. Géométrie héritée conservée comme modèle : c'est le seul tableau du lot précédent qui force déjà sa mécanique. | `jumpBudget: 0` |
| 6 | `demo-mouvement` **(nouveau)** | Synthèse d'acte : dash, wall jump et double saut enchaînés. | serrés au strict nécessaire |

### Acte II — Mécanismes
| # | Tableau | Refonte | Budgets |
|---|---|---|---|
| 7 | `demo-interrupteur` | Porte vue **fermée** d'abord ; interrupteur dans une alcôve **hors du trajet** ; second couple démontrant la **bascule** (repasser dessus referme). | — |
| 8 | `demo-plaque-pression` | Contraste explicite plaque (continue) / interrupteur (bascule) côte à côte, puis l'idiome débloqué par la `TACHE-06` : **poser un bloc sur la plaque** pour tenir la porte ouverte et partir. Ne repose plus sur un saut qui prend la porte de vitesse. | — |
| 9 | `demo-cle` | Clé dans un **cul-de-sac obligatoire**, seconde paire clé/porte pour confirmer la leçon, plus l'invite « Interagir » ci-dessous. | — |
| 10 | `demo-bloc` | Corniche inatteignable sans la marche que forme le bloc. | `jumpBudget: 1` |
| 11 | `demo-bloc-reduit` | Fosse de **deux** cases (insautable), comblée seulement par le bloc ×0,5. | borné |
| 12 | `demo-bloc-quart` | Passage étroit où seul le ×0,25 passe et où le ×0,5 reste coincé — rend visible une différence de gabarit aujourd'hui invisible. | borné |

### Invite « Interagir »
`EX-GP-023` exige le contact **et** l'action « Interagir » pour ramasser une clé. C'est la seule
entrée du jeu qu'aucun tableau ne demande ailleurs, et `niveaux.md` interdit le texte d'indice : un
joueur qui l'ignore reste **bloqué sans retour**. Le level design seul ne suffit pas à garantir la
découverte ; une invite contextuelle est donc ajoutée.

- `core::MechanismController` : accesseur public `isKey(std::size_t index)`, **symétrique de
  `isContinuous`** déjà présent (`_isKey` existe en privé) — aucune logique nouvelle.
- `hmi::gameHudLines` : paramètre supplémentaire indiquant que le personnage chevauche une clé non
  ramassée, calculé par `hmi::GameSession` qui possède déjà la boîte et le contrôleur. La fonction
  reste **pure**, donc assertable sans GPU.
- Clé de traduction `hud.interact_prompt` dans `fr.lang` et `en.lang`, même mécanisme que
  `hud.jumps_remaining` — aucune chaîne en dur (`EX-REN-033`).

## Fichiers impactés
- `Source/Elements/Levels/demo-{deplacement,saut,double-saut,wall-jump,dash,mouvement,interrupteur,plaque-pression,cle,bloc,bloc-reduit,bloc-quart}.json`
- `Source/Elements/Levels/sequence-demo.json` — insertion de `demo-mouvement.json`.
- `Source/Test/Systeme/test_parcours_complet.cpp` — un script d'entrées par tableau.
- `Source/Core/Gameplay/MechanismController.h`, `Source/HMI/Game/GameHud.{h,cpp}`,
  `Source/HMI/Game/GameSession.cpp`, `Source/Elements/Localization/{fr,en}.lang`,
  `Source/Test/Unit/HMI/Game/test_game_hud.cpp`.

## Tests (obligatoires)
- Chaque tableau est franchi par un script d'entrées **déterministe** qui emploie réellement sa
  mécanique — un script réduit à « droite » doit échouer, et le garde-fou anti-couloir l'exige.
- L'invite « Interagir » apparaît quand le personnage chevauche une clé non ramassée, disparaît
  ensuite, et n'apparaît jamais sur un tableau sans clé. Bloc `\castest{}` écrit avec le test.
- `scripts/check_demo_sequence.py` vert après insertion de `demo-mouvement.json`.

## Points d'attention
- **Un budget serré rend le tableau injouable si la géométrie n'est pas exacte.** Itérer sur la
  géométrie jusqu'au franchissement, comme la `TACHE-02` l'avait fait, plutôt que de le supposer.
- **Ne pas restaurer d'ancienne géométrie.** Redessiner à partir d'une page blanche ; recréer un
  tracé par `git show` reconduirait le biais qui a produit ces tableaux.
- Le budget de sauts se consomme aussi par le saut **aérien** gratuit : tenir « saut » en continu
  épuise un budget serré. Les scripts d'entrées doivent déclencher un saut par atterrissage.
- Le seul canal textuel du jeu reste le champ `name`, affiché par `hmi::gameHudLines` : le choisir
  comme un intitulé de leçon, pas comme un nom de fichier.

## Définition de fait (DoD)
- Les douze tableaux sont redessinés, habillés, franchissables par un script déterministe qui
  emploie leur mécanique, et **aucun** n'est franchissable en maintenant « droite » ; l'invite
  « Interagir » est livrée et testée ; garde-fous de la `TACHE-05` verts sur ce périmètre.

## Exigences
`EX-LVL-012` (progression), `EX-GP-014` à `EX-GP-017` (saut, double saut, wall jump, dash),
`EX-GP-020` à `EX-GP-025` (mécanismes, bloc, clé, budget, plaque), `EX-GP-005` (blocs réduits),
`EX-CTRL-022` (« Interagir »), `EX-REN-033` (traduction), `EX-IHM-003` (affichage tête haute).
