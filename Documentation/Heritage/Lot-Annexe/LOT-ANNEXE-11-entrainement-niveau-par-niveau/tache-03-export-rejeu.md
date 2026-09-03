# TACHE-03 — Export au format de rejeu v1 {#lot-annexe-11-tache-03-export-rejeu}

**Lot :** [LOT-ANNEXE-11](epic.md) · **Emplacement :** `Source/AiSolver/Training` · **Statut :**
fait

## Contexte
TACHE-02 produit une séquence d'actions en mémoire. Cette tâche l'écrit sur disque au format de
rejeu v1 défini par LOT-ANNEXE-07 (`Source/AiSolver/Replay`) — le seul format qu'un futur lot
d'intégration jeu (génération 5) saura consommer pour rejouer une IA sans inférence live. Cette
tâche est aussi celle qui assemble TACHE-01 et TACHE-02 en un point d'entrée exécutable unique.

## Travail à réaliser
- **`aisolver::training::exportReplay(const ActionSequence&, const std::filesystem::path&
  levelPath, const std::filesystem::path& outputPath)`** : sérialise la séquence au format v1 en
  s'appuyant sur l'API d'écriture fournie par LOT-ANNEXE-07 (noms exacts dépendants de cette API —
  cette tâche **consomme** le format, ne le redéfinit ni ne le duplique). Associe la séquence au
  niveau d'origine par référence (chemin/nom), sans copier le contenu du niveau dans le fichier de
  rejeu.
- **Refus d'export si `TrainingResult::solved == false`** : la fonction (ou l'appelant) retourne une
  erreur explicite plutôt que d'écrire un fichier de rejeu partiel ou trompeur — un fichier de rejeu
  exporté doit toujours correspondre à un niveau réellement terminé.
- **Point d'entrée minimal** enchaînant `LevelTrainingSession` (TACHE-01) → `replayBestIndividual`
  (TACHE-02) → `exportReplay` (cette tâche) pour un chemin de niveau donné en argument — nécessaire
  à un usage manuel réel de ce lot (sans point d'entrée, TACHE-01/02/03 restent des bibliothèques
  sans utilisateur), mais reste volontairement minimal : l'outillage ergonomique (CLI, options,
  reprise) est hors périmètre (génération 5).

## Fichiers impactés
- `Source/AiSolver/Training/ReplayExport.h`/`.cpp` (nouveaux).
- Point d'entrée minimal (fichier exact — `main` dédié ou fonction appelée par un exécutable du
  module `AiSolver` — à confirmer selon la structure retenue par le `CMakeLists.txt` initial du
  module, posé en amont de la génération 2).
- Tests : `Source/Test/Unit/AiSolver/Training/test_replay_export.cpp` (nouveau).

## Tests (obligatoires)
- **Export réussi et round-trip** : l'export d'une séquence résolue produit un fichier conforme au
  format v1 ; relu par le lecteur de LOT-ANNEXE-07, il reproduit exactement la même séquence
  d'actions.
- **Refus d'export sur échec** : une tentative d'export d'un `TrainingResult::solved == false`
  échoue explicitement (aucun fichier écrit, contrat exact — exception ou code d'erreur — documenté
  et testé).
- **Référence au bon niveau** : le fichier exporté référence le chemin/nom du niveau source utilisé
  pour l'entraînement.
- **Bout en bout** : sur un niveau trivial et un budget de générations réduit, un appel unique au
  point d'entrée minimal produit un fichier de rejeu valide sur disque, sans étape manuelle
  intermédiaire.

## Points d'attention
- **Le format lui-même appartient entièrement à LOT-ANNEXE-07** : cette tâche ne le modifie ni ne le
  duplique, elle l'utilise en tant que client.
- **Refuser l'export d'un résultat non résolu est un choix délibéré** : produire quand même un
  fichier « au cas où » créerait un risque d'utilisation par erreur pour une démonstration en jeu —
  sans inférence live pour vérifier a posteriori qu'il fonctionne, un tel fichier ne peut être
  corrigé qu'en relançant un entraînement.
- **Le test bout en bout doit rester rapide** (niveau trivial, plafonds réduits) : ce n'est pas le
  lieu de mesurer la performance réelle de l'algorithme sur des niveaux complexes — ce sera le rôle
  de la génération 4 (évaluation et robustesse).

## Définition de fait (DoD)
- Export disponible et testé (`ctest` vert, round-trip vérifié), point d'entrée minimal fonctionnel
  bout en bout ; build `/W4 /WX` sans avertissement ; Doxygen à jour ; `EX-IA-012` déclarée.

## Notions abordées
@ref guide-annexe-algorithmes-evolutionnistes — boucle générationnelle, élitisme, reproductibilité
d'un entraînement.

## Exigences
`EX-IA-012` (nouvelle).
