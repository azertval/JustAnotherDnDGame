# TACHE-02 — Activation en Release et contexte de démarrage {#lot-61-tache-02-activation-release}

**Lot :** [LOT-61](epic.md) · **Emplacement :** `Source/HMI` · **Statut :** non commencé

## Contexte
La `TACHE-01` fournit l'outil ; celle-ci prend la décision qui manque. Dans `Source/HMI/main.cpp`,
le bloc `if constexpr (core::kDeveloperBuild)` ajoute console et mémoire en développement et **rien
du tout** ailleurs. Ce n'est pas un oubli : le commentaire justifie explicitement de n'installer ni
console (l'exécutable n'en a pas) ni sink mémoire (croissance inutile). Le raisonnement est bon, sa
conclusion est trop large.

Le second manque est le **contexte**. Même journalisés, les messages ne servent qu'à moitié si l'on
ignore la version, la configuration et le matériel : ce sont les trois premières questions qu'on
pose devant un rapport de défaut.

## Travail à réaliser
- **Installer `core::FileLogSink` dans tous les cas**, à côté de l'exécutable
  (`hmi::executableDirectory()`, dans un sous-dossier `Logs/`). La console et le sink mémoire
  restent réservés au développement — leur justification actuelle est correcte et n'est pas remise
  en cause.
- **Niveau par défaut différencié** : `Trace` en développement (inchangé), niveau plus élevé en
  Release. `JADG_LOG_LEVEL` et `--log-level=` continuent de primer, dans cet ordre — la
  logique de `resolveMinimumLogLevel` ne change pas, seule sa valeur initiale dépend de la
  configuration.
- **Contexte de démarrage**, journalisé une fois, juste après l'installation des sinks :
  - version du moteur — `core::Engine::version()`, alimentée par `project(VERSION)` ;
  - configuration (développement / Release) ;
  - description de l'adaptateur Direct3D 11 retenu et niveau de fonctionnalité obtenu, disponibles
    dans `hmi::GraphicsDevice` ;
  - présence et nom d'une manette XInput ;
  - langue active et thème effectif.
- **Journaliser aussi l'arrêt**, avec le code de sortie — la ligne existe déjà, vérifier qu'elle
  atteint le fichier (donc que le sink est détruit après elle).

## Fichiers impactés
- `Source/HMI/main.cpp`.
- `Source/HMI/Graphics/GraphicsDevice.{h,cpp}` — exposer la description de l'adaptateur si elle ne
  l'est pas déjà.
- `Source/HMI/Diagnostics/StartupContext.{h,cpp}` (nouveau) — assemblage du texte de contexte,
  fonction pure et testable.
- `Source/Test/Unit/HMI/Diagnostics/test_startup_context.cpp` (nouveau).

## Tests (obligatoires)
- L'assemblage du contexte est une **fonction pure** testée : à entrées données (version,
  configuration, adaptateur, manette, langue), texte attendu. C'est la seule partie testable sans
  matériel, et elle suffit.
- Le contexte est journalisé **une seule fois** par lancement.
- Un dossier de journaux impossible à créer laisse l'application démarrer normalement.
- Les builds de développement conservent leurs deux sinks **et** gagnent le fichier ; le
  comportement de l'outil de session du `LOT-15` est inchangé.

## Points d'attention
- **L'ordre de destruction compte** : le message d'arrêt doit être écrit avant que le sink ne soit
  détruit, sinon la dernière ligne — souvent la plus utile — manque toujours.
- **Ne pas journaliser par image.** Le niveau Release plus élevé y aide, mais une seule trace
  répétée par image suffit à remplir le fichier et à faire tourner la rotation, effaçant tout le
  contexte utile.
- La description de l'adaptateur passe par DXGI : la récupérer là où le périphérique est déjà créé,
  sans ouvrir un second chemin d'initialisation.
- `--export-atlas=` sort du programme avant l'ouverture de la fenêtre : vérifier que ce chemin
  n'écrit pas un journal inutile, ou qu'il en écrit un cohérent.
- Le comportement visé ne s'observe **qu'en Release** : le job livré par [LOT-58](@ref lot-58)
  est ce qui rend cette tâche vérifiable autrement qu'à la main.

## Définition de fait (DoD)
- Toute exécution, développement comme Release, écrit un journal fichier contenant un contexte de
  démarrage complet et le message d'arrêt ; le niveau par défaut est adapté à la configuration et
  reste surchargeable ; un dossier inaccessible ne bloque rien ; le contexte est assemblé par une
  fonction pure testée ; `/W4 /WX` propre.

## Exigences
`EX-NFR-042` (trace exploitable) ; réutilise `EX-NFR-040` (erreur récupérable), `EX-NFR-013`
(compilation sans avertissement), `EX-NFR-010` (logique testable), `EX-BUILD-010` (déploiement à
côté de l'exécutable), `EX-NFR-030` (version issue du build CMake).
