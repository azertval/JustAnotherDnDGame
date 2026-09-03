# LOT-61 — Diagnostics d'une version publiée {#lot-61}

> Statut : **non commencé**. Prérequis : aucun. À livrer **avant** le tag `v0.1.0` : son intérêt
> commence le jour où quelqu'un d'extérieur exécute le jeu.

## Objectif
Faire qu'un défaut rencontré par un joueur sur la version publiée laisse une **trace**.

Aujourd'hui il n'en laisse aucune. `main()` n'installe de sinks que dans les builds de
développement :

```
if constexpr (core::kDeveloperBuild) {
    core::defaultLogger().addSink(std::make_unique<core::ConsoleLogSink>());
    // + MemoryLogSink
}
```

En Release — c'est-à-dire dans l'archive que télécharge un non-développeur — le journaliseur n'a
**aucun sink**. Tous les `HMI_LOG_*` et `CORE_LOG_*` du programme partent dans le vide. Le commentaire
qui justifie ce choix est exact sur ses prémisses (« l'exécutable n'a pas de console et une
croissance mémoire serait inutile ») et incomplet sur sa conclusion : il n'existe pas que la console
et la mémoire.

Le projet dispose pourtant de tout le nécessaire : une interface `core::ILogSink`, deux
implémentations, un format de ligne testé (`core::LogFormat`) et même une sérialisation de session
prête à l'emploi (`hmi::serializeSessionLog`, `LOT-15`). Il manque un sink fichier — et la décision
de l'activer.

## Périmètre

### Inclus
- **`core::FileLogSink`** : écriture des messages dans un fichier, sur le contrat `core::ILogSink`
  existant.
- **Activation en Release**, à côté de l'exécutable, avec un **volume borné** (taille maximale,
  rotation simple sur un petit nombre de fichiers).
- **Niveau de journalisation adapté** en Release : `Trace` inonderait le disque ; le niveau par
  défaut est plus haut, tout en restant ajustable par `JADG_LOG_LEVEL` et `--log-level=`,
  déjà implémentés.
- **Contexte de démarrage** journalisé une fois : version du moteur (`core::Engine::version()`),
  configuration, adaptateur graphique, présence d'une manette. Ce sont les informations qu'on
  demande systématiquement dans un rapport de défaut.
- **Emplacement documenté** dans le manuel utilisateur, pour qu'un joueur sache quoi joindre.

### Exclus (hors périmètre de ce lot)
- Télémétrie, envoi automatique, service distant : aucune donnée ne quitte la machine.
- Capture de vidage mémoire (*minidump*) et analyse de plantage natif.
- Interface de consultation des journaux dans l'application (le bouton de développement existant
  suffit).
- Journalisation par image ou par pas : le volume doit rester négligeable.

## Décisions de cadrage
- **Un fichier, pas une console.** Rendre une console en Release changerait l'expérience de
  lancement pour tout le monde afin de servir un cas rare. Un fichier ne se voit que si on le
  cherche.
- **Borné par construction.** Un journal sans limite sur une machine de joueur est une fuite disque
  à retardement. Taille maximale et rotation sont dans le périmètre, pas un raffinement ultérieur.
- **Aucune donnée personnelle, aucune sortie réseau.** Le fichier reste local, et le contenu se
  limite à ce que le programme journalise déjà.
- **Ne pas réutiliser `MemoryLogSink` en Release.** Sa croissance non bornée est exactement ce que
  le commentaire de `main()` écarte, à raison. Le sink fichier écrit et oublie.
- **Écrire hors du chemin critique**, sans jamais faire échouer le jeu : un disque plein ou un
  dossier en lecture seule dégradent la journalisation, pas la partie (`EX-NFR-040`).

## Exigences couvertes
- Nouvelle : `EX-NFR-042` (une version publiée produit une trace exploitable, bornée et locale).
- Réutilisées : `EX-NFR-040` (erreur récupérable), `EX-NFR-041` (RAII), `EX-NFR-003` (empreinte
  mémoire stable — un journal non borné y contreviendrait), `EX-NFR-013` (compilation sans
  avertissement), `EX-NFR-012` (conventions, dont la politique d'erreurs).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-file-log-sink.md) | `core::FileLogSink` : écriture, rotation, bornes | `Source/Core/Diagnostics` | ⬜ |
| [TACHE-02](tache-02-activation-release.md) | Activation en Release, niveau par défaut et contexte de démarrage | `Source/HMI` | ⬜ |
| [TACHE-03](tache-03-documentation-verification.md) | Documentation et vérification | `Documentation` | ⬜ |

## Critères d'acceptation du lot
1. Le zip de release, lancé sur une machine sans outil de développement, écrit un fichier de journal
   lisible contenant la version, la configuration et le contexte matériel.
2. Le fichier ne dépasse jamais la taille maximale, quelle que soit la durée de la session.
3. Un dossier en lecture seule ou un disque plein n'empêchent ni le lancement ni le jeu.
4. Le niveau de journalisation reste ajustable par variable d'environnement et par ligne de
   commande.
5. Les builds de développement conservent exactement leur comportement actuel (console + mémoire).
6. Le manuel indique où trouver le fichier et quoi en faire.
7. Build `/W4 /WX`, `ctest` à 100 %, Doxygen et lints verts.

## Dépendances
Aucun prérequis. Bâtit sur la journalisation du [LOT-02](@ref lot-02) et la sérialisation de
session du [LOT-15](@ref lot-15). Bénéficie de [LOT-58](@ref lot-58) (le job Release rend
vérifiable un comportement qui ne s'observe qu'en Release).

## Navigation des tâches
- @subpage lot-61-tache-01-file-log-sink
- @subpage lot-61-tache-02-activation-release
- @subpage lot-61-tache-03-documentation-verification
