# Source/Core/

Fonctions **back** : le moteur et la logique du jeu.

## Périmètre
- Cadencement du temps de simulation (`Time/`), diagnostic (`Diagnostics/`), mathématiques (`Math/`).
- Entités et composants (`Ecs/`), cartes et leur format JSON (`Levels/`), lecture JSON (`Data/`).
- Règles du jeu de rôle : fiches, jets, équipement, dialogues (`Rpg/`) ; combat tactique (`Combat/`).
- Monde et exploration : atlas, graphe des lieux, quartiers, session d'exploration (`World/`) ;
  interactions et drapeaux de monde (`Gameplay/`).
- Clés d'assets d'entité et marqueurs de substitution (`Resources/`).

Ce dossier est indépendant de la présentation (`../HMI/`) : il expose un état, il ne l'affiche pas.
