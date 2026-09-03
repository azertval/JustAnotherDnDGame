# TACHE-04 — Documentation de l'ordre de niveaux indicatif {#lot-annexe-11-tache-04-ordre-niveaux-curriculum}

**Lot :** [LOT-ANNEXE-11](epic.md) · **Emplacement :** `Documentation/Lot-Annexe` · **Statut :**
fait

## Contexte
`LevelTrainingSession` (TACHE-01) entraîne toujours **un seul** niveau par exécution, sans jamais
enchaîner automatiquement — décision ferme, répétée dans tout ce lot. Cela laisse néanmoins une
question ouverte pour qui **lance manuellement** des sessions d'entraînement les unes après les
autres : dans quel ordre ? Cette tâche est purement documentaire : elle propose un ordre indicatif,
sans introduire aucun mécanisme logiciel qui l'imposerait.

## Travail à réaliser
- Rédiger, dans ce fichier, un tableau **niveau → mécanique(s) exercée(s)** reprenant l'ordre
  effectif de la séquence de démonstration du jeu (`Source/Elements/Levels/README.md`,
  `Source/Test/Systeme/test_parcours_complet.cpp`, source de vérité de l'ordre) : un mécanisme
  nouveau à la fois, difficulté croissante — le même principe pédagogique que le curriculum humain
  du jeu, appliqué ici à l'entraînement d'un agent.
- Justifier explicitement, en tête de tableau, que cet ordre est une **suggestion d'usage manuel**
  sans portée logicielle : rien dans `Source/AiSolver` ne le lit, ne le vérifie ni ne l'impose — un
  entraînement sur `demo-final.json` avant `demo-saut.json` reste parfaitement possible et valide,
  simplement moins susceptible de converger vite pour cette ligne de base évolutionniste sans
  aucune connaissance acquise sur les mécaniques plus simples (chaque run repart de poids
  aléatoires, aucun transfert d'un niveau à l'autre, cf. décision de cadrage de l'épic).
- Noter, pour les niveaux dont l'objet unique n'est pas de nouvelles actions mais un test de
  passage à l'échelle (budget, salles), que l'ordre proposé n'est pas une exigence de difficulté
  stricte au sens de l'algorithme (le poids d'un mécanisme pour un agent évolutionniste ne suit pas
  nécessairement l'intuition humaine) — une observation, pas une garantie.

## Tableau niveau → mécanique(s) (ordre indicatif, repris de `demo-*.json`)

> Mis à jour au moment de la rédaction de ce lot pour refléter la séquence effective de
> `Source/Elements/Levels/sequence-demo.json`/`Source/Test/Systeme/ScriptedLevelSequence.h` (22
> niveaux) — la table d'origine de cette section, rédigée au cadrage du programme Lot-Annexe, avant
> l'évolution ultérieure de cette séquence (`LOT-65`), ne correspondait plus à l'ordre réel ; voir
> le point d'attention ci-dessus sur l'obsolescence possible.

| # | Fichier | Mécanique(s) exercée(s) |
|---|---------|--------------------------|
| 1 | `demo-deplacement.json` | Mouvement horizontal, chute, sol — aucun saut requis |
| 2 | `demo-saut.json` | Saut simple, hauteur variable |
| 3 | `demo-double-saut.json` | Saut aérien (double saut) |
| 4 | `demo-wall-jump.json` | Wall slide + wall jump |
| 5 | `demo-dash.json` | Dash |
| 6 | `demo-mouvement.json` | Synthèse : dash, double saut, wall jump enchaînés |
| 7 | `demo-interrupteur.json` | Interrupteur ↔ porte (activation au front) |
| 8 | `demo-plaque-pression.json` | Plaque de pression (activation continue, poids qui doit rester) |
| 9 | `demo-cle.json` | Clé ↔ porte verrouillée (contact + interaction) |
| 10 | `demo-bloc.json` | Bloc poussable |
| 11 | `demo-bloc-reduit.json` | Bloc à taille réduite (`×0.5`) |
| 12 | `demo-bloc-quart.json` | Bloc à taille quart (`×0.25`, obstrue un couloir bas) |
| 13 | `demo-pente.json` | Pentes et arrondis (montée puis descente) |
| 14 | `demo-pente-gauche.json` | Pentes orientées vers la gauche (miroir du précédent) |
| 15 | `demo-concave.json` | Arrondis concaves (sol et plafond) |
| 16 | `demo-plafond.json` | Plafond incliné (saut sous obstacle) |
| 17 | `demo-dangers-directionnels.json` | Dangers directionnels (marche sûre, saut dangereux) |
| 18 | `demo-dangers-avances.json` | Dangers mobile/commuté/temporisé (fenêtres d'attente) |
| 19 | `demo-plateforme.json` | Plateformes mobiles (portage de bloc, timing) |
| 20 | `demo-budget.json` | Budget limité de sauts/dashs |
| 21 | `demo-synthese.json` | Synthèse : mécanismes, terrain et dangers entrelacés |
| 22 | `demo-final.json` | Niveau final multi-salles : combine l'ensemble des mécaniques précédentes |

## Fichiers impactés
- `Documentation/Lot-Annexe/LOT-ANNEXE-11-entrainement-niveau-par-niveau/
  tache-04-ordre-niveaux-curriculum.md` (ce fichier, avec le tableau ci-dessus).

## Tests (obligatoires)
- Aucun test automatisé (travail de documentation pure, aucun code produit) — la validation est la
  **revue** de la cohérence du tableau avec `Source/Elements/Levels/README.md` et
  `Source/Test/Systeme/test_parcours_complet.cpp` (source de vérité de l'ordre effectif du jeu).

## Points d'attention
- **Cet ordre peut devenir obsolète** si de nouveaux niveaux `demo-*.json` sont ajoutés à la
  séquence du jeu après ce lot — aucun mécanisme ne le maintient synchronisé automatiquement (à la
  différence de `scripts/check_demo_sequence.py`, qui vérifie `Source/HMI/main.cpp` face à
  `test_parcours_complet.cpp`, pas cette documentation). Une dérive n'a aucun impact fonctionnel
  (l'ordre reste une suggestion), seulement documentaire.
- **Ne pas transformer cette suggestion en mécanisme** à l'occasion d'une évolution future de ce lot
  sans une décision de cadrage explicite qui lèverait la contrainte transverse « un run = un
  niveau » — ce tableau ne doit jamais devenir l'entrée d'une boucle de code.

## Définition de fait (DoD)
- Tableau rédigé, cohérent avec la séquence `demo-*.json` réelle du jeu au moment de la rédaction,
  revu.

## Notions abordées
@ref guide-annexe-algorithmes-evolutionnistes — boucle générationnelle, élitisme, reproductibilité
d'un entraînement.

## Exigences
Aucune exigence propre — contribue à `EX-IA-012` (documentation d'usage, aucun code).
