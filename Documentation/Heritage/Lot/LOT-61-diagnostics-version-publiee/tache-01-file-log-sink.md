# TACHE-01 — Puits de journalisation fichier {#lot-61-tache-01-file-log-sink}

**Lot :** [LOT-61](epic.md) · **Emplacement :** `Source/Core/Diagnostics` · **Statut :** non commencé

## Contexte
`Source/Core/Diagnostics/` contient deux sinks : `ConsoleLogSink` (écrit sur la sortie standard) et
`MemoryLogSink` (accumule en mémoire, consulté par l'outil de session du `LOT-15`). Tous deux
implémentent `core::ILogSink`, dont le contrat est déjà couvert par
`Source/Test/Unit/Core/Diagnostics/test_sinks.cpp`.

Il manque le troisième, celui qui survit à la fermeture du programme. Le format de ligne n'est pas
à réinventer : `core::LogFormat` le produit déjà, et `hmi::serializeSessionLog` sait assembler des
entrées mémorisées en un texte — c'est exactement ce qu'un fichier doit contenir.

## Travail à réaliser
- **`core::FileLogSink`** implémentant `core::ILogSink` : ouverture à la construction, écriture d'une
  ligne par message via `core::LogFormat`, fermeture garantie par RAII (`EX-NFR-041`).
- **Borne de taille** : au-delà d'une taille maximale (constante nommée), le fichier courant est
  renommé et un nouveau est ouvert ; au-delà d'un nombre maximal de fichiers conservés, le plus
  ancien est supprimé. Deux constantes, pas un système de rotation configurable.
- **Échec d'ouverture non fatal** : chemin invalide, dossier en lecture seule, disque plein → le
  sink se met dans un état inerte, absorbe les messages sans rien faire, et ne lève rien. Un sink
  qui plante en journalisant une erreur est le pire des cas.
- **Écriture vidée à intervalle raisonnable** : un tampon non vidé perd précisément les derniers
  messages, c'est-à-dire ceux qui décrivent le défaut. Vider au moins à chaque message de niveau
  `Error`, et à la destruction.
- **Aucune dépendance nouvelle** : `std::filesystem` et `std::ofstream` suffisent ; le sink vit dans
  `Core`, qui ne connaît ni Qt ni le système graphique.

## Fichiers impactés
- `Source/Core/Diagnostics/FileLogSink.{h,cpp}` (nouveau).
- `Source/Core/CMakeLists.txt`.
- `Source/Core/Diagnostics/README.md`.
- `Source/Test/Unit/Core/Diagnostics/test_file_log_sink.cpp` (nouveau), `Source/Test/CMakeLists.txt`.

## Tests (obligatoires)
- Un message écrit se retrouve dans le fichier, au format produit par `core::LogFormat`.
- **Rotation** : dépasser la taille maximale crée un nouveau fichier et conserve l'ancien ; dépasser
  le nombre de fichiers supprime le plus ancien. C'est le test central de la tâche.
- **Chemin invalide** → sink inerte, aucune exception, aucun plantage, et le journaliseur continue
  de servir ses autres sinks.
- Le fichier est **fermé** et complet après destruction du sink (RAII).
- Un message de niveau `Error` est présent sur disque **avant** la fin du programme (vidage).
- Tests purs `Core`, écrivant dans un répertoire temporaire, nettoyé après coup.

## Points d'attention
- **Le sink ne doit jamais jeter.** Il est appelé depuis des chemins de gestion d'erreur ; une
  exception y transformerait un défaut mineur en plantage.
- **Ne pas journaliser depuis le sink** : la récursion est immédiate et fatale.
- Les tests écrivent des fichiers : utiliser un répertoire temporaire unique et nettoyer, sans quoi
  deux exécutions parallèles de `ctest` se marchent dessus.
- Attention au coût : le sink sera actif en Release, où le niveau par défaut sera plus élevé
  (`TACHE-02`) — mais l'écriture doit rester hors du chemin critique de la boucle de rendu.
- Si le sink est un jour partagé entre threads, l'accès devra être sérialisé ; le projet est
  mono-thread (`EX-ARCH-060`), le noter plutôt que de l'implémenter par anticipation.

## Définition de fait (DoD)
- Un sink fichier implémente le contrat `ILogSink`, écrit au format existant, borne sa taille et son
  nombre de fichiers, absorbe silencieusement tout échec d'écriture, ferme par RAII, et est couvert
  par des tests `Core` purs ; `/W4 /WX` propre.

## Exigences
`EX-NFR-042` (trace exploitable et bornée) ; réutilise `EX-NFR-040` (erreur récupérable),
`EX-NFR-041` (RAII), `EX-NFR-003` (empreinte bornée), `EX-NFR-010` (`Core` sans dépendance système
inutile), `EX-NFR-020` (tests unitaires), `EX-ARCH-060` (mono-thread).
