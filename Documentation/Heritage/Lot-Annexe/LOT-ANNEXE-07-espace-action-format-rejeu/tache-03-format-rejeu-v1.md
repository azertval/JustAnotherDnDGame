# TACHE-03 — Format de rejeu v1 {#lot-annexe-07-tache-03-format-rejeu-v1}

**Lot :** [LOT-ANNEXE-07](epic.md) · **Emplacement :** `Source/AiSolver/Replay` · **Statut :** à faire

## Contexte
Une fois un niveau résolu par un algorithme d'entraînement (génération 2/3), le seul livrable
durable et rejouable en jeu (sans aucune inférence live, décision transverse du programme) est une
séquence enregistrée de `core::PlayerInput`. Cette tâche définit le format de fichier qui porte
cette séquence, dès la génération 1, pour que `LOT-ANNEXE-11` (premier export réel, génération 2)
dispose déjà d'une cible stable.

## Travail à réaliser
- **`aisolver::ReplayFile`** (`Source/AiSolver/Replay/ReplayFile.h`) : structure portant `uint32_t
  formatVersion` (absent à la lecture = version initiale, même principe qu'`EX-LVL-005`),
  `std::string levelPath` (chemin relatif du niveau d'origine), `uint64_t levelFingerprint`
  (réservé — calculé et vérifié seulement à partir de `LOT-ANNEXE-17`, `0` en attendant),
  `std::vector<core::PlayerInput> steps` (séquence ordonnée, un élément par pas fixe), et des
  métadonnées : `std::string algorithmName`, `std::string exportedAtIso8601`, `uint64_t seed`,
  `float finalReward` (ou équivalent — statistique finale de l'entraînement).
- **`aisolver::writeReplay(const std::filesystem::path&, const ReplayFile&)`** et
  **`aisolver::ReplayLoadResult aisolver::readReplay(const std::filesystem::path&)`** (`Source/
  AiSolver/Replay/ReplayFile.cpp`) : sérialisation JSON (réutilise `nlohmann::json`, déjà dépendance
  de `Core` — aucune nouvelle dépendance introduite), `ReplayLoadResult` porte un
  `std::optional<ReplayFile>` et un message d'erreur, sur le modèle de `core::LevelLoadResult`
  (jamais d'exception, erreur récupérable).
- Documentation du format (schéma JSON en commentaire Doxygen du `.h`), à l'image de la
  documentation du format JSON de niveau dans `Documentation/Specification/niveaux.md`.

## Fichiers impactés
- `Source/AiSolver/Replay/ReplayFile.h/.cpp` — nouveau.
- `Source/AiSolver/CMakeLists.txt` — ajout des nouveaux fichiers, lien vers `nlohmann_json` (déjà
  disponible via `Core`).

## Tests (obligatoires)
- **Round-trip** : un `ReplayFile` écrit puis relu produit une structure identique champ par champ,
  y compris une séquence de plusieurs milliers de `core::PlayerInput`.
- **Absence de numéro de version** : un fichier JSON sans champ `formatVersion` est lu comme la
  version initiale, sans erreur (`ReplayLoadResult::ok()` vrai).
- **Fichier introuvable** : `readReplay` sur un chemin inexistant renvoie une erreur récupérable
  (`ReplayLoadResult` sans valeur, message explicite), jamais une exception.
- **Fichier JSON malformé** : `readReplay` sur un contenu non-JSON ou incomplet renvoie une erreur
  récupérable, jamais une exception ni un plantage.
- **Séquence vide** : un `ReplayFile` sans aucun pas (`steps` vide) est un cas valide à
  l'écriture/lecture (utile pour tester le format indépendamment d'un entraînement réel), sans
  traitement spécial requis en aval de ce lot.

## Points d'attention
- **`levelFingerprint` est réservé mais non vérifié par ce lot** : le champ existe dans le format
  dès cette version pour éviter une migration de fichiers déjà écrits lorsque `LOT-ANNEXE-17`
  ajoutera la vérification — ce lot se contente de le sérialiser/désérialiser tel quel (`0` si
  absent), sans jugement de validité.
- **`levelPath` est un chemin relatif à `Source/Elements/Levels`, jamais un chemin absolu** :
  cohérent avec la portabilité du dépôt (un chemin absolu casserait sur une autre machine) — même
  logique que `JADG_LEVELS_DIR` (`Source/Test/CMakeLists.txt`), qui recompose un chemin
  absolu à partir d'un chemin relatif connu.
- **Aucune exception ne doit jamais traverser `readReplay`/`writeReplay`** : erreurs de parsing JSON
  (`nlohmann::json`) capturées et transformées en `ReplayLoadResult` en erreur, cohérent avec la
  politique du projet (`Documentation/Specification/conventions.md`, catégorie « erreur
  récupérable ») déjà appliquée par `core::LevelLoader`.

## Définition de fait (DoD)
- `ReplayFile`/`writeReplay`/`readReplay` disponibles et testés (`ctest` vert), format documenté ;
  build `/W4 /WX` sans avertissement ; Doxygen à jour ; `EX-IA-008` déclarée.

## Notions abordées
@ref guide-annexe-apprentissage-renforcement — action, espace d'action, politique déterministe ou
stochastique, exploration.

## Exigences
`EX-IA-008` (nouvelle).
