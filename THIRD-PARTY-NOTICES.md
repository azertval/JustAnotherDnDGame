# Mentions de tiers

Ce fichier recense les composants tiers redistribués avec JustAnotherDnDGame, ou dont il dépend au
build, avec leur licence. Il ne remplace pas les fichiers de licence livrés à côté des ressources
concernées : il y renvoie.

JustAnotherDnDGame lui-même est sous **GPL-3.0-or-later** ([`LICENSE`](LICENSE)).

> **À ne pas confondre.** Les en-têtes `SPDX-License-Identifier: GPL-3.0-or-later` marquent le
> **code du projet** — `Source/**/*.h`, `Source/**/*.cpp`, `scripts/*.py`, les `CMakeLists.txt` — et
> **rien d'autre**. Les images, les sons, les polices et les bibliothèques tierces gardent chacun
> leur propre licence, listée ci-dessous. Voir un fichier `.cpp` marqué GPL ne dit **rien** de la
> licence d'un `.png` du même dépôt.

Ces mentions sont également affichées **dans le jeu**, à l'écran *Crédits* : un utilisateur qui
n'ouvrira jamais ce fichier doit tout de même savoir que le jeu embarque Qt sous LGPLv3 et des
ressources sous CC0 et SIL OFL. La LGPLv3 et la SIL OFL l'exigent ; le CC0 non, mais le projet
crédite quand même.

Compatibilité vérifiée : la **LGPLv3 autorise explicitement** la redistribution de l'ensemble sous
**GPLv3** (article 2 de la LGPLv3), donc Qt et ce projet cohabitent sans conflit. Les licences des
ressources (CC0, SIL OFL) et celles des bibliothèques de test (BSD 3-Clause, MIT) sont toutes
permissives, donc compatibles avec la GPL dans ce sens.

## Bibliothèques

| Composant | Version | Licence | Mode | Redistribué ? |
|---|---|---|---|---|
| [Qt](https://www.qt.io/) | 6.11.2 (`win64_msvc2022_64`) | **LGPLv3** | Provisionné hors dépôt | **Oui** — DLL déployées à côté de l'exécutable (`windeployqt`) |
| [GoogleTest](https://github.com/google/googletest) | v1.15.2 | BSD 3-Clause | FetchContent | Non — tests seulement |
| [nlohmann/json](https://github.com/nlohmann/json) | v3.11.3 | MIT | FetchContent | Non — en-têtes compilés dans l'exécutable |
| [aqtinstall](https://github.com/miurahr/aqtinstall) | commit épinglé | MIT | Outil de build | Non |

`DirectX` ne figure pas ici : il provient du **Windows SDK** et relève de sa licence, comme tout
composant du système d'exploitation.

### Qt et la LGPLv3 — la seule obligation qui pèse réellement

Qt est utilisé sous **LGPLv3**, en **lien dynamique**. C'est ce qui permet de ne pas publier le
source du jeu. En contrepartie, trois obligations doivent rester vraies, et elles le sont :

1. **Le lien reste dynamique.** Les DLL Qt sont déployées à côté de l'exécutable par `windeployqt`,
   jamais liées statiquement — un utilisateur peut donc les remplacer par sa propre version de Qt.
2. **L'usage de Qt et sa licence sont mentionnés** — c'est l'objet de ce fichier.
3. **Aucune modification de Qt n'est distribuée.** Le projet consomme Qt tel quel ; si cela changeait
   un jour, les sources modifiées devraient être publiées sous LGPLv3.

Passer Qt en lien **statique** invaliderait le point 1 et changerait les obligations du projet : ce
n'est pas un détail d'optimisation, c'est une décision de licence.

## Ressources

Toutes les ressources tierces sont déjà créditées à l'endroit où elles vivent — ce tableau n'en est
que l'index.

| Type | Emplacement | Licence | Détail |
|---|---|---|---|
| Graphismes | `Source/Elements/Assets/` | **CC0 1.0** (packs Kenney) ; le reste **généré par script** | [`Assets/CREDITS.md`](Source/Elements/Assets/CREDITS.md) |
| Bruitages | `Source/Elements/Audio/` | **CC0 1.0** (packs Kenney) | [`Audio/CREDITS.md`](Source/Elements/Audio/CREDITS.md) |
| Police *Inter* | `Source/Elements/Assets/Fonts/` | SIL Open Font License 1.1 | `Inter-LICENSE.txt` |
| Police *Pixelify Sans* | `Source/Elements/Assets/Fonts/` | SIL Open Font License 1.1 | `PixelifySans-LICENSE.txt` |
| Police *Press Start 2P* | `Source/Elements/Assets/Fonts/` | SIL Open Font License 1.1 | `PressStart2P-LICENSE.txt` |

Le **CC0** n'exige aucune attribution ; le projet crédite malgré tout ses auteurs. La **SIL OFL**,
elle, en exige une : les fichiers de licence doivent accompagner les polices partout où elles sont
redistribuées, et un nom de police réservé (« Press Start 2P ») ne peut pas être réutilisé pour une
version modifiée.

## Entretien

Ce fichier se met à jour **avec** la dépendance qu'il décrit, jamais après coup : ajouter une
bibliothèque ou une ressource tierce sans l'inscrire ici, c'est perdre l'information au moment où
elle est encore connue. Le tableau des dépendances de build vit en double dans
[`External/README.md`](External/README.md), qui en donne le *mode de provisionnement* ; ici, seule
la **licence** compte.
