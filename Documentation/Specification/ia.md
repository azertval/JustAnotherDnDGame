# Solveur IA {#spec-ia}

> Statut : **livré** (`0.1.2`). Une IA maison capable de terminer un niveau de façon autonome,
> développée **sans framework d'apprentissage automatique** et rejouée en jeu sous forme d'une
> séquence d'actions déterministe. Dépend de [`gameplay.md`](gameplay.md) (conditions de
> victoire/echec) et de [`controles.md`](controles.md) (actions logiques que l'agent produit).

Le découpage de travail correspondant est le programme `LOT-ANNEXE-*`, archivé sous
`Documentation/Heritage/Lot-Annexe/`, dont
chaque épic référence les exigences ci-dessous.

## 1. Fondations numériques
Bibliothèques de calcul écrites en interne, sans dépendance à un framework d'apprentissage
automatique tiers.

- \anchor EX-IA-001 **EX-IA-001** — Le programme d'IA doit disposer d'une bibliothèque de
  calcul tensoriel (`Tensor<T>`, opérations élémentaires, réductions, produit matriciel) et d'un
  générateur pseudo-aléatoire déterministe (`Rng`, seed explicite), tous deux **implémentés en
  interne** sans dépendance à un framework de calcul numérique tiers.
- \anchor EX-IA-002 **EX-IA-002** — Le programme d'IA doit disposer d'un moteur de
  différentiation automatique en mode *reverse* (graphe de calcul dynamique, `backward()` par
  parcours topologique inverse), **implémenté en interne**, avec une vérification systématique par
  différences finies (*gradient checking*) bloquante pour toute nouvelle opération différentiable.
- \anchor EX-IA-003 **EX-IA-003** — Le programme d'IA doit disposer d'une bibliothèque de
  réseaux de neurones **implémentée en interne** (couche dense, activations différentiables,
  opérations différentiables complémentaires, composition en réseau, initialisation Xavier/He,
  sérialisation versionnée des poids), sans dépendance à un framework de calcul numérique ou
  d'apprentissage automatique tiers.
- \anchor EX-IA-004 **EX-IA-004** — Le programme d'IA doit disposer d'optimiseurs de
  descente de gradient **implémentés en interne** (SGD avec inertie optionnelle, Adam), appliqués
  aux gradients accumulés par le moteur d'autodiff maison sur les paramètres d'un réseau, avec une
  vérification de convergence sur fonctions jouets bloquante avant tout usage réel.

## 2. Pont avec le jeu et observabilité
Rendre le jeu jouable et mesurable par une machine, sur la base de `Core` sans le modifier.

- \anchor EX-IA-005 **EX-IA-005** — Le jeu doit être jouable **sans fenêtre ni GPU**, au
  pas fixe, via une API `reset(chemin)`/`step(core::PlayerInput)` reproduisant fidèlement
  l'orchestration de simulation du jeu (mécanismes, blocs, physique, dangers, issue), sans aucune
  modification de `Core`, avec une garde de non-régression pas-à-pas permanente en CI.
- \anchor EX-IA-006 **EX-IA-006** — L'état du jeu, lu via `HeadlessLevelEnvironment`
  (`EX-IA-005`), doit pouvoir être encodé en un tenseur d'observation de forme stable (fenêtre de
  tuiles catégorielle, état joueur, état des mécanismes environnants), de façon **déterministe**
  (mêmes entrées `Core` → même tenseur), consommable par tout algorithme d'apprentissage sans
  connaissance des types `Core` sous-jacents.
- \anchor EX-IA-007 **EX-IA-007** — L'agent doit disposer d'un espace d'action discret et
  fini dérivé de `core::PlayerInput`, avec un décodage déterministe (`argmax`, pour le rejeu final)
  et un décodage stochastique (échantillonnage pondéré par température, via un générateur
  déterministe fourni par l'appelant, pour l'exploration en entraînement).
- \anchor EX-IA-008 **EX-IA-008** — Une séquence d'actions gagnante doit pouvoir être
  exportée dans un format de fichier de rejeu versionné (identifiant/empreinte du niveau, séquence
  de `core::PlayerInput` par pas fixe, métadonnées d'entraînement), lisible et inscriptible, destiné
  à être rejoué en jeu de façon strictement déterministe sans aucune inférence live.
- \anchor EX-IA-009 **EX-IA-009** — Un signal de récompense unique et partagé (progression
  vers la sortie, bonus de complétion, pénalité de mort, pénalité de temps) et une classification de
  fin d'épisode (victoire, échec, timeout, blocage) doivent être définis une seule fois et réutilisés
  par tout algorithme d'apprentissage du programme, pour garantir la comparabilité de leurs résultats.
- \anchor EX-IA-023 **EX-IA-023** — La récompense de progression doit se
  baser sur une distance de plus court chemin sur la grille (respectant les murs statiques), jamais
  une distance euclidienne en ligne droite, pour qu'un pas de détour nécessaire autour d'un mur ne
  reçoive jamais une récompense de progression négative alors qu'il rapproche réellement le
  personnage de la sortie.
- \anchor EX-IA-010 **EX-IA-010** — Tout algorithme d'entraînement doit journaliser, au
  fil de l'entraînement et par génération/épisode, un jeu de statistiques standard (récompenses
  meilleure/moyenne/pire, écart-type, pas du meilleur, taux de réussite, moyenne mobile et delta sur
  une fenêtre récente) dans un fichier CSV unique par run, au format identique quel que soit
  l'algorithme, pour permettre de détecter un plateau sans attendre la fin de l'entraînement.

## 3. Algorithmes d'apprentissage
Un agent évolutionniste comme ligne de base, puis l'apprentissage par gradient.

- \anchor EX-IA-011 **EX-IA-011** — algorithme évolutionniste maison : population de
  réseaux à poids indépendants, évaluation de fitness par récompense cumulée sur un run complet,
  sélection/croisement/mutation/élitisme, boucle de génération journalisée, reproductible à seed
  fixée.
- \anchor EX-IA-012 **EX-IA-012** — entraînement niveau-par-niveau (critère d'arrêt,
  un seul niveau par exécution) et export de la séquence d'actions gagnante au format de rejeu v1.
- \anchor EX-IA-013 **EX-IA-013** — L'agent doit disposer d'un algorithme de policy
  gradient maison (REINFORCE) : la politique est mise à jour par rétropropagation, via le moteur
  d'autodiff maison, du gradient de la perte `-log π(a|s) × retour`, et non par une recherche sans
  gradient (évolutionniste ou autre).
- \anchor EX-IA-014 **EX-IA-014** — L'agent doit disposer d'un mécanisme de réduction de
  variance du gradient de policy gradient (acteur-critique) : un réseau critique estime la valeur
  d'état, et l'avantage (`retour − valeur estimée`) remplace le retour brut dans la perte de
  politique de `EX-IA-013`, avec mesure chiffrée de l'amélioration de convergence par rapport à
  REINFORCE brut.
- \anchor EX-IA-015 **EX-IA-015** — L'agent doit disposer d'un algorithme d'apprentissage
  par gradient avancé (PPO ou DQN maison, le choix étant tranché à l'ouverture du lot sur la base de
  mesures chiffrées), intégré au harnais d'entraînement existant et comparé chiffres à l'appui à la
  génération 2 (évolutionniste) et au reste de la génération 3 (REINFORCE, acteur-critique).

## 4. Évaluation et robustesse
Mesurer ce que valent ces agents, et hors de leur niveau d'origine.

- \anchor EX-IA-016 **EX-IA-016** — Un harnais de benchmark générique, indépendant de la
  famille d'algorithme, doit permettre d'exécuter **de façon répétée** (`N` répétitions) un modèle
  déjà entraîné sur son niveau d'origine via `HeadlessLevelEnvironment`, de mesurer le taux de
  réussite, le nombre de pas moyen et la variance associés, de produire un rapport comparatif au
  format CSV par niveau × algorithme, et de mesurer la robustesse de la politique à un bruit léger
  de l'observation perçue, sans jamais modifier l'état réel simulé, le modèle évalué, ni la logique
  d'entraînement qui l'a produit.
- \anchor EX-IA-017 **EX-IA-017** — Le taux de réussite d'un modèle entraîné sur un
  niveau donné doit pouvoir être mesuré, par simple réutilisation du harnais de benchmark
  existant, lorsqu'il est exécuté sur un niveau différent de celui qui l'a produit ; ce résultat
  est une mesure de référence sur la nature de l'apprentissage réalisé, jamais un objectif
  d'entraînement ni un critère de qualité d'un algorithme.

## 5. Intégration au jeu et outillage
Ce que le joueur et le développeur en voient.

- \anchor EX-IA-018 **EX-IA-018** — Un fichier de rejeu doit être **validé à la
  lecture** : le niveau qu'il référence doit exister et son empreinte doit correspondre à celle
  enregistrée à l'export ; toute divergence est signalée comme une erreur récupérable, jamais
  silencieuse.
- \anchor EX-IA-019 **EX-IA-019** — Le jeu doit pouvoir rejouer, dans une partie réelle
  (rendu compris), un fichier de rejeu produit par le programme d'IA, en alimentant la boucle de
  simulation existante (`hmi::GameSession`) avec la séquence de `core::PlayerInput` enregistrée,
  après validation de sa cohérence avec le niveau référencé, sans qu'aucune inférence de réseau de
  neurones n'intervienne dans `Core` ou `HMI`.
- \anchor EX-IA-020 **EX-IA-020** — Un exécutable en ligne de commande unique doit
  exposer, sans dupliquer leur logique, les capacités d'entraînement (génération 2/3),
  d'évaluation (génération 4) et d'export de rejeu (génération 1/2) du programme, avec une
  configuration d'hyperparamètres systématiquement journalisée dans les traces produites (CSV,
  fichier de rejeu) pour garantir la reproductibilité d'un run passé.
- \anchor EX-IA-021 **EX-IA-021** — Un script de garde-fou, exécuté en intégration
  continue à chaque Pull Request, doit détecter tout rejeu versionné dont le fichier de niveau
  référencé a changé (empreinte divergente) ou n'existe plus depuis son export, et faire échouer la
  vérification en conséquence, sans dépendance à un build C++.
- \anchor EX-IA-022 **EX-IA-022** — Le menu principal doit exposer un écran « Mode IA »
  (entraînement, validation/sauvegarde, rejeu) équivalent aux capacités de `aisolver-cli`
  (`LOT-ANNEXE-19`), avec un entraînement non bloquant (thread séparé), observable (progression,
  aperçu en direct) et interruptible proprement (résultat partiel sauvegardé), sans dupliquer la
  moindre règle d'apprentissage.
