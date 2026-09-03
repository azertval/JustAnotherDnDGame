# LOT-ANNEXE-16 — Évaluation hors-niveau (transfert) {#lot-annexe-16}

> Statut : **fait**. Prérequis : [LOT-ANNEXE-15](@ref lot-annexe-15) (harnais de
> benchmark), dont ce lot réutilise intégralement l'infrastructure sans la dupliquer. Dernier lot
> de la génération 4.

## Objectif
L'entraînement reste strictement **niveau par niveau** (décision transverse du programme
Lot-Annexe, réaffirmée à chaque génération concernée) : rien dans le programme ne cherche à
produire un modèle généraliste. Mais une question reste ouverte et légitime, distincte de cet
objectif d'entraînement : un modèle entraîné sur le niveau A, une fois exécuté sur un niveau B
qu'il n'a jamais vu, se comporte-t-il un minimum sensément (progresse-t-il, même sans réussir), ou
bien s'effondre-t-il totalement (comportement incohérent dès la première tuile inconnue) ? La
réponse renseigne sur la **nature** de ce qu'un algorithme a appris — une trajectoire précise
mémorisée, ou des réflexes un minimum transférables — sans jamais faire de cette question un
objectif d'entraînement. Ce lot mesure, il ne cherche pas à améliorer.

## Périmètre

### Inclus
- **Exécution croisée** : réutilise `aisolver::eval::BenchmarkRunner::run` (`LOT-ANNEXE-15`,
  TACHE-01) tel quel, en lui passant un `TrainedPolicy` chargé depuis le niveau A et un
  `levelPath` pointant vers le niveau B — aucune modification du harnais lui-même.
- **Rapport de taux de réussite par paire (A, B)** : réutilise `aisolver::BenchmarkReport`
  (`LOT-ANNEXE-15`, TACHE-02) avec une colonne supplémentaire distinguant le niveau
  d'entraînement du niveau d'exécution (le rapport de `LOT-ANNEXE-15` ne distingue pas les deux,
  puisqu'il suppose toujours qu'ils coïncident).
- **Documentation des résultats observés**, dans `Documentation/Lot-Annexe`, présentée
  explicitement comme une mesure de référence, pas une conclusion sur la qualité d'un algorithme.

### Exclus (hors périmètre de ce lot)
- **Toute amélioration du transfert** (entraînement multi-niveaux, augmentation de données,
  domaine de randomisation) : hors du régime d'entraînement niveau par niveau du programme —
  ce lot mesure un fait, il ne cherche pas à le changer. Une éventuelle génération future de
  généralisation partirait de ces mesures, sans que ce lot ne préjuge de sa nécessité.
- **Choix des paires (A, B) exhaustif** (toutes les combinaisons possibles de niveaux
  `demo-*.json`) : un sous-ensemble représentatif (quelques paires couvrant des mécaniques
  partagées et des mécaniques totalement différentes) suffit à répondre à la question posée, sans
  exploser le temps d'exécution d'une campagne (héritier direct du coût CPU déjà assumé par
  `LOT-ANNEXE-15`).
- **Interprétation automatique des résultats** (seuil de « bon » ou « mauvais » transfert codé en
  dur) : les chiffres sont rapportés, leur lecture reste humaine — cohérent avec le principe déjà
  retenu pour tout le harnais de la génération 4.

## Décisions de cadrage
- **Ce lot ne modifie ni le harnais (`LOT-ANNEXE-15`) ni aucun module d'entraînement.** Il
  l'utilise à travers son API publique existante (`BenchmarkRunner::run`, `BenchmarkReport`),
  exactement comme le ferait un troisième appelant quelconque — la seule nouveauté est le choix
  d'exécuter un modèle sur un niveau différent de celui qui l'a produit, une combinaison déjà
  permise par la signature de `run` (elle prend un `levelPath` indépendant du modèle chargé).
- **Les attentes sont documentées comme réalistes avant toute mesure : un transfert probablement
  faible.** Un modèle entraîné niveau par niveau, sans aucune pression d'entraînement à généraliser,
  n'a structurellement aucune raison de bien se comporter sur une géométrie de niveau inédite —
  documenter cette attente à l'avance évite de lire après coup un résultat faible comme un échec du
  programme plutôt que comme une confirmation attendue de son régime d'entraînement assumé.
- **Le choix des paires (A, B) privilégie la lisibilité pédagogique** (une paire de niveaux
  partageant une mécanique proche, ex. deux niveaux à interrupteur/porte ; une paire de niveaux
  n'ayant presque rien en commun, ex. un niveau de déplacement simple contre un niveau à salles) —
  pour que la mesure elle-même serve à illustrer la question posée, pas seulement à produire des
  chiffres.
- **Aucune conclusion normative n'est tirée dans la documentation de résultats** (TACHE-02) : le
  texte rapporte les chiffres et les met en regard des mécaniques communes/différentes entre A et
  B, sans affirmer qu'un algorithme est « meilleur » qu'un autre sur cette seule base — cohérent
  avec le caractère strictement descriptif de toute la génération 4.

## Notions abordées
Voir @ref guide-annexe-evaluation-rl, section 5 (transfert entre niveaux comme mesure, pas comme
objectif). Aucune source nouvelle par rapport à ce chapitre — ce lot applique directement son
principe à une paire de niveaux.

## Exigences couvertes
- Nouvelle, déclarée dans [la spécification IA](@ref spec-ia) : [`EX-IA-017`](@ref EX-IA-017).
- Réutilisées (inchangées) : `EX-IA-016` (harnais de benchmark, `LOT-ANNEXE-15`).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-execution-croisee.md) | Exécution croisée (modèle de A exécuté sur B) | `Source/AiSolver/Eval` | ✅ |
| [TACHE-02](tache-02-documentation-resultats.md) | Documentation des résultats observés | `Documentation/Lot-Annexe` | ✅ |

## Critères d'acceptation du lot
1. `BenchmarkRunner::run` (`LOT-ANNEXE-15`) est appelé sans aucune modification de sa signature ni
   de son comportement pour exécuter un modèle du niveau A sur le niveau B.
2. Le rapport croisé distingue explicitement, pour chaque ligne, le niveau d'entraînement du
   niveau d'exécution (deux colonnes distinctes, jamais confondues).
3. La documentation des résultats énonce explicitement, avant toute mesure, l'attente d'un
   transfert faible et le fait que ce lot mesure sans chercher à l'améliorer.
4. Aucune ligne de `Source/AiSolver/Eval/BenchmarkRunner.*`, `BenchmarkReport.*` ni d'aucun module
   `Source/AiSolver/Training/*` n'est modifiée par ce lot (vérifiable par diff).
5. Logique nouvelle **couverte par des tests** (`ctest` vert), déterministe, sans GPU. Build
   `/W4 /WX` sans avertissement, Doxygen et lint des exigences verts.

## Dépendances
Réutilise intégralement [LOT-ANNEXE-15](@ref lot-annexe-15) (`BenchmarkRunner`, `BenchmarkReport`),
sans dépendance directe supplémentaire au-delà de ce que `LOT-ANNEXE-15` référence déjà. Dernier lot
de la génération 4 : aucun lot de cette génération n'en dépend.

## Résultats
Campagne exécutée (2 paires de niveaux, modèle évolutionniste entraîné sur le niveau trivial) :
transfert nul mesuré dans les deux cas, y compris sur la paire de mécanique partagée — voir le
détail du protocole, les chiffres et leur lecture dans
@ref lot-annexe-16-resultats-transfert.

## Navigation des tâches
- @subpage lot-annexe-16-tache-01-execution-croisee
- @subpage lot-annexe-16-tache-02-documentation-resultats
- @subpage lot-annexe-16-resultats-transfert
