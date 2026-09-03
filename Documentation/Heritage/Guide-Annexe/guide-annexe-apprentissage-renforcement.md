# Vocabulaire de l'apprentissage par renforcement {#guide-annexe-apprentissage-renforcement}

**Lots concernés :** [LOT-ANNEXE-05](@ref lot-annexe-05) à [LOT-ANNEXE-09](@ref lot-annexe-09)
(environnement headless, observation, action, récompense, statistiques).
**Prérequis :** aucun (ce chapitre est le point d'entrée du vocabulaire RL — les chapitres
précédents concernaient uniquement le calcul numérique/réseaux, réutilisables par n'importe quel
domaine d'apprentissage automatique).

## Pourquoi ce chapitre

Jusqu'ici, les chapitres précédents ont posé des outils de calcul génériques (tenseurs, autodiff,
réseaux, optimiseurs) qui existeraient de la même façon pour n'importe quelle tâche d'apprentissage
automatique (reconnaissance d'image, traduction, etc.). Ce chapitre introduit le vocabulaire propre
à l'**apprentissage par renforcement** (*Reinforcement Learning*, RL) — la branche de
l'apprentissage automatique où un agent apprend en **interagissant** avec un environnement, par
essai-erreur, plutôt qu'à partir d'exemples déjà étiquetés (à la différence de l'apprentissage
supervisé, hors sujet ici). C'est le cadre général dans lequel s'inscrivent tous les algorithmes des
générations 2 et 3 du programme.

## 1. Agent, environnement, boucle d'interaction

- L'**agent** est ce qui prend des décisions — dans ce programme, à terme, un réseau de neurones
  (chapitre [Réseaux de neurones](@ref guide-annexe-reseaux-neurones)) qui choisit une action.
- L'**environnement** est le monde avec lequel l'agent interagit — ici, un niveau du jeu, simulé
  par `aisolver::HeadlessLevelEnvironment` (LOT-ANNEXE-05), qui enveloppe fidèlement la même
  simulation (`Core`) que le jeu réel, mais sans fenêtre ni rendu.
- La boucle d'interaction, répétée à chaque **pas** (*step*) :
  1. L'environnement fournit une **observation** de son état courant.
  2. L'agent choisit une **action** à partir de cette observation.
  3. L'environnement applique l'action, avance d'un pas, et renvoie une **récompense** et
     l'observation suivante.
  4. On répète, jusqu'à ce que l'**épisode** se termine.

Cette boucle `reset()`/`step(action) → (observation, récompense, fin ?)` est devenue une
convention quasi universelle en RL logiciel — `HeadlessLevelEnvironment::reset`/`step` en est une
implémentation maison, mais l'idée d'une interface aussi minimale et standardisée doit beaucoup à
la bibliothèque *OpenAI Gym* (voir Sources), qui a popularisé cette forme précise d'API.

## 2. État, observation

L'**état** est la description complète et exacte de la situation dans l'environnement (ici :
position/vitesse exactes du personnage, état de chaque mécanisme, etc. — tout ce qu'il faudrait
pour reproduire exactement la suite de la simulation). L'**observation** est ce que l'agent
**perçoit** de cet état — parfois identique à l'état complet, parfois une version partielle ou
transformée. Dans ce programme, l'observation est le tenseur produit par l'encodeur de
[LOT-ANNEXE-06](@ref lot-annexe-06) (fenêtre de tuiles + état joueur + état des mécanismes), une
transformation de l'état réel de `Core` en une forme exploitable par un réseau (voir
[Réseaux de neurones](@ref guide-annexe-reseaux-neurones)).

## 3. Concevoir une observation : ce que l'agent a le droit de voir

Le §2 dit *ce qu'est* une observation ; celui-ci explique les **choix de conception** qu'elle
impose, parce que ce sont eux — bien plus que l'algorithme d'apprentissage — qui décident si un
agent peut réussir. Un agent ne peut apprendre à réagir qu'à ce qu'il perçoit : toute information
absente de l'observation est, pour lui, inexistante. C'est le cadrage que fait
[LOT-ANNEXE-06](@ref lot-annexe-06), et ce chapitre en donne les raisons.

### 3.1. Trois tensions à arbitrer

- **Complétude contre taille.** Donner tout l'état du niveau (toutes les tuiles, tous les
  mécanismes) garantit que rien ne manque, mais produit un vecteur d'entrée énorme : le nombre de
  poids de la première couche est proportionnel à cette taille (chapitre
  [Réseaux de neurones](@ref guide-annexe-reseaux-neurones)), donc plus d'entrées = plus de poids à
  apprendre = plus d'épisodes nécessaires. À l'inverse, une observation trop maigre rend certaines
  situations **indistinguables** : si deux états qui exigent des actions différentes produisent la
  même observation, aucune politique — si bien entraînée soit-elle — ne peut faire les deux bons
  choix. C'est le compromis central.
- **Absolu contre relatif.** Une position absolue (`x = 137`) oblige l'agent à réapprendre la même
  situation à chaque endroit du niveau. Une observation **centrée sur le personnage** (une fenêtre
  de tuiles qui le suit, comme celle de LOT-ANNEXE-06) rend au contraire la même configuration
  locale reconnaissable partout : « un trou juste devant, un mur à droite » se présente à l'agent
  sous la même forme, où que ce soit dans le niveau. C'est cette invariance qui permet d'apprendre
  quelque chose de réutilisable plutôt qu'une suite de réflexes attachés à des coordonnées.
- **Brut contre pré-mâché.** On pourrait fournir directement « distance au danger le plus proche »,
  calculée à la main. C'est efficace, mais chaque grandeur pré-calculée est une **décision de jeu
  prise à la place de l'agent** : on lui apprend à suivre notre stratégie plutôt qu'à en trouver
  une. Le programme reste donc volontairement proche du brut (grille de tuiles + état du
  personnage), quitte à demander plus d'entraînement.

### 3.2. Encoder des catégories : le *one-hot*

Un type de tuile (`Vide`, `Plein`, `Pente`, `Danger`, `Sortie`…) est une grandeur **catégorielle** :
ses valeurs n'ont pas d'ordre. Les encoder par un simple numéro (`Vide = 0`, `Plein = 1`,
`Danger = 2`) serait une erreur classique — le réseau, qui ne manipule que des nombres, en
déduirait mécaniquement que `Danger` est « deux fois plus » que `Plein` et qu'il vient « après »
lui, relations qui n'existent pas. L'encodage **one-hot** (« un parmi n ») évite ce faux ordre :
chaque catégorie devient un vecteur de `n` cases valant `0` partout sauf `1` à sa position. Quatre
types de tuiles donnent `Vide = (1,0,0,0)`, `Plein = (0,1,0,0)`, etc. — quatre entrées au lieu
d'une, aucune relation d'ordre suggérée. C'est le choix de LOT-ANNEXE-06 pour la fenêtre de tuiles.

### 3.3. Mettre les grandeurs continues à la même échelle

La vitesse du personnage, sa position dans la tuile courante et un indicateur « au sol » (0 ou 1)
n'ont a priori ni la même unité ni le même ordre de grandeur. Or les poids d'un réseau sont
initialisés à des valeurs de taille comparable (chapitre
[Réseaux de neurones](@ref guide-annexe-reseaux-neurones)) : une entrée qui varie entre `-300` et
`+300` écrase mécaniquement, au départ, une entrée qui varie entre `0` et `1`, et produit un
gradient d'autant plus grand — ce qui déstabilise l'entraînement (chapitre
[Optimisation](@ref guide-annexe-optimisation)). D'où la règle suivie par LOT-ANNEXE-06 : ramener
chaque grandeur continue dans un intervalle comparable (typiquement `[-1, 1]`), en divisant par une
borne connue du jeu (vitesse maximale, taille d'une tuile).

### 3.4. Observation partielle et mémoire

Si l'observation ne suffit pas à déterminer la suite (par exemple : une image fixe ne dit pas si un
objet monte ou descend), l'environnement cesse d'être markovien **du point de vue de l'agent**
(§8) — on parle d'observation partielle. Deux remèdes classiques : inclure les grandeurs
dynamiques manquantes dans l'observation (c'est le choix du programme : la **vitesse** du
personnage en fait explicitement partie), ou empiler les `k` dernières observations pour que la
dynamique devienne lisible (*frame stacking*, popularisé par le DQN sur Atari, voir Sources). Le
premier remède est préférable quand on a accès à l'état interne du simulateur — c'est notre cas,
d'où l'absence d'empilement dans LOT-ANNEXE-06.

## 4. Action, politique

Une **action** est ce que l'agent choisit de faire à un pas donné — ici, un élément de l'espace
d'action discret de [LOT-ANNEXE-07](@ref lot-annexe-07) (direction, saut, dash), converti en
`core::PlayerInput`. Une **politique** (*policy*, souvent notée `π`) est la règle — apprise —
selon laquelle l'agent choisit une action à partir d'une observation. Une politique peut être :
- **déterministe** : toujours la même action pour la même observation (décodage `argmax`,
  [LOT-ANNEXE-07](@ref lot-annexe-07)) ;
- **stochastique** : une distribution de probabilité sur les actions, dont on **tire** une action
  au hasard à chaque pas (décodage `decodeStochastic`) — nécessaire à l'**exploration** pendant
  l'entraînement (voir [REINFORCE](@ref guide-annexe-reinforce)) : une politique toujours certaine
  de son choix ne peut jamais découvrir qu'une autre action ferait mieux.

## 5. Récompense

La **récompense** (*reward*) est un nombre, reçu à chaque pas, qui indique à quel point ce pas a
été « bon ». C'est le **seul** signal à partir duquel l'agent apprend — contrairement à
l'apprentissage supervisé, personne ne dit explicitement à l'agent quelle était la bonne action à
chaque pas, seulement à quel point le résultat était bon ou mauvais. Le choix de la fonction de
récompense (comment traduire « bien jouer » en un nombre) a un impact énorme sur ce que l'agent
finit par apprendre — [LOT-ANNEXE-08](@ref lot-annexe-08) construit cette fonction pour le
programme (progression vers la sortie, bonus de victoire, pénalité de mort/de temps), pensée pour
être **dense** (non nulle à chaque pas) plutôt que **sparse** (seulement à la toute fin) — un signal
dense est bien plus facile à exploiter par un algorithme de gradient (voir
[REINFORCE](@ref guide-annexe-reinforce)), qui a besoin de savoir, à *chaque* pas, si les choses se
sont améliorées ou dégradées.

## 6. Concevoir une fonction de récompense : *shaping* et pièges

La fonction de récompense est le seul endroit où l'on dit à l'agent ce qu'on attend de lui. C'est
aussi l'endroit où l'on se trompe le plus facilement, parce qu'un agent optimise **exactement** ce
qu'on a écrit, jamais ce qu'on avait en tête. [LOT-ANNEXE-08](@ref lot-annexe-08) construit celle
du programme ; ce chapitre donne les principes qui la justifient.

### 6.1. Récompense creuse contre récompense dense

- **Creuse** (*sparse*) : `+1` en atteignant la sortie, `0` partout ailleurs. C'est la définition la
  plus honnête de l'objectif — mais tant qu'aucun épisode n'atteint la sortie **par hasard**, tous
  les épisodes reçoivent exactement le même retour, donc le gradient est nul et l'agent n'a
  strictement rien à apprendre. Sur un niveau qui demande une suite de sauts précis, la probabilité
  d'y arriver au hasard est proche de zéro : l'entraînement ne démarre jamais.
- **Dense** : un petit signal à chaque pas (par exemple, la progression horizontale vers la sortie
  depuis le pas précédent). Le gradient a alors de quoi mordre dès le premier épisode.

C'est ce **guidage par récompense intermédiaire** qu'on appelle le *reward shaping*, et c'est le
choix du programme (LOT-ANNEXE-08 : progression, bonus de victoire, pénalités de mort et de temps).

### 6.2. Le piège : l'agent optimise la lettre, pas l'intention

Une récompense de progression mal posée s'exploite. Quelques cas concrets, tous observés dans la
littérature comme en pratique :

- Récompenser la **position** `x` atteinte plutôt que la **progression** : l'agent peut alors
  gagner des points en faisant des allers-retours autour d'un même point s'il touche la récompense
  à chaque pas où il est loin du départ, sans jamais avancer.
- Récompenser la vitesse : l'agent apprend à foncer dans un mur à pleine vitesse plutôt qu'à
  atteindre la sortie.
- Oublier la pénalité de temps : rien n'incite alors à finir, et une politique qui survit
  indéfiniment sans jamais gagner peut obtenir un meilleur retour cumulé qu'une politique qui gagne
  vite — surtout si mourir est puni et gagner faiblement récompensé.

Ce phénomène a un nom, *reward hacking* (voir Sources) ; il ne signale jamais un agent défaillant,
mais toujours une fonction de récompense qui dit autre chose que ce qu'on croyait. **Le réflexe de
diagnostic** : quand un agent se comporte de façon absurde, regarder d'abord le retour cumulé qu'il
obtient. S'il est élevé, le bug est dans la récompense, pas dans l'algorithme.

### 6.3. Ajouter du guidage sans changer l'objectif : le *shaping* par potentiel

Il existe un résultat théorique utile : si l'on ajoute à la récompense un terme de la forme
`γ × Φ(état suivant) − Φ(état courant)` — où `Φ` est une fonction quelconque de l'état, appelée
**potentiel** (par exemple : « moins la distance restante jusqu'à la sortie ») —, alors la
politique optimale reste **exactement** la même qu'avec la récompense d'origine, tout en donnant à
l'agent un signal dense à chaque pas. C'est le théorème du *potential-based reward shaping* (Ng,
Harada & Russell, 1999, voir Sources). La raison intuitive : sur un épisode complet, cette somme se
télescope et ne dépend que des potentiels de départ et d'arrivée, donc elle ne peut pas créer de
boucle rentable — précisément le défaut du premier exemple du §6.2.

La récompense de progression de LOT-ANNEXE-08 est de cette famille (elle mesure une **différence**
de distance entre deux pas consécutifs, pas une distance absolue) : c'est ce qui la rend robuste
aux allers-retours, et c'est la raison de fond du choix « différence » plutôt que « valeur ».

### 6.4. Ordres de grandeur et conflits entre termes

Une fonction de récompense combine plusieurs termes (progression, victoire, mort, temps). Leurs
**amplitudes relatives** sont un réglage à part entière : si le bonus de victoire est du même ordre
que la somme des récompenses de progression d'un épisode, l'agent n'a pas d'incitation nette à
terminer plutôt qu'à grappiller ; si la pénalité de mort est écrasante, l'agent apprend à ne plus
bouger (l'immobilité devient la stratégie la moins risquée). La règle pratique : le bonus terminal
doit **dominer** clairement le cumul des signaux intermédiaires, et toute pénalité doit rester
comparable à ce qu'elle est censée décourager, pas à ce que vaut tout l'épisode.

## 7. Épisode, horizon

Un **épisode** est une séquence complète d'interactions, du début (`reset`) jusqu'à une fin
naturelle — ici, victoire (`Won`), échec (`Lost`), ou une fin artificielle nécessaire pour borner la
durée d'un entraînement automatisé : plafond dur de pas (`TimedOut`) ou blocage détecté (`Stuck`,
absence de progression prolongée) — voir [LOT-ANNEXE-08](@ref lot-annexe-08). La **récompense
cumulée** d'un épisode (parfois appelée le **retour**, *return*) est la somme de toutes les
récompenses reçues pendant cet épisode — c'est, en un sens, ce que tout algorithme d'apprentissage
du programme cherche, in fine, à maximiser (voir
[algorithmes évolutionnistes](@ref guide-annexe-algorithmes-evolutionnistes) et
[REINFORCE](@ref guide-annexe-reinforce) pour deux façons très différentes d'y parvenir).

## 8. Le cadre théorique : processus de décision markovien

Le cadre mathématique standard du RL est le **processus de décision markovien** (*Markov Decision
Process*, MDP) : à tout instant, l'état complet du système détermine entièrement les probabilités
de transition vers l'état suivant (étant donné une action) — l'histoire passée n'ajoute rien de
plus, seul l'état **présent** compte (propriété dite « de Markov »). La simulation de `Core`
respecte cette propriété par construction : elle est déterministe au pas fixe (`EX-NFR-002`), donc
un état donné et une action donnée déterminent **exactement** l'état suivant — un cas particulier
(et le plus simple) de MDP, sans aucune incertitude de transition. Ce chapitre ne développe pas
davantage la théorie formelle du MDP (fonctions de valeur, équation de Bellman) : ces notions sont
introduites précisément là où elles deviennent nécessaires, dans le chapitre
[Acteur-critique](@ref guide-annexe-acteur-critique).

## 9. Pourquoi mesurer un plafond de performance (le CSV de statistiques)

Un entraînement RL ne progresse pas toujours indéfiniment : il finit typiquement par **plafonner**
— la récompense cumulée cesse de s'améliorer d'une génération/d'un épisode à l'autre, signe que
l'agent a extrait tout ce qu'il pouvait de la configuration actuelle (algorithme, réseau,
hyperparamètres). Détecter ce plafond **tôt** (plutôt que de laisser tourner un entraînement
infructueux) est exactement le rôle du CSV de [LOT-ANNEXE-09](@ref lot-annexe-09) — une
préoccupation pratique et opérationnelle, pas seulement théorique, essentielle dès qu'on entraîne
sans supervision constante.

## Sources

- Sutton, R.S., Barto, A.G. (2018). *Reinforcement Learning: An Introduction* (2nd ed.). MIT Press.
  — **la** référence de base de tout ce chapitre et des suivants ; couvre agent/environnement/état/
  action/récompense/politique/épisode/retour et le cadre du MDP en détail. Édition consultable
  gratuitement en ligne (site de l'auteur) une fois de retour avec accès Internet.
- Bellman, R. (1957). *A Markovian Decision Process*. Journal of Mathematics and Mechanics 6(5),
  679–684. — origine du processus de décision markovien.
- Brockman, G., Cheung, V., Pettersson, L., Schneider, J., Schulman, J., Tang, J., Zaremba, W.
  (2016). *OpenAI Gym*. arXiv:1606.01540. — popularise l'interface `reset`/`step` comme standard de
  fait pour un environnement RL logiciel, dont `HeadlessLevelEnvironment` (LOT-ANNEXE-05) s'inspire
  dans sa forme (sans aucune dépendance à cette bibliothèque, réimplémentée entièrement en C++
  maison).
- Puterman, M.L. (1994). *Markov Decision Processes: Discrete Stochastic Dynamic Programming*.
  Wiley. — traitement mathématique approfondi des MDP, au-delà de ce qui est nécessaire ici.
- Ng, A.Y., Harada, D., Russell, S. (1999). *Policy Invariance Under Reward Transformations: Theory
  and Application to Reward Shaping*. ICML. — le théorème du *shaping* par potentiel (§6.3) :
  ajouter `γ Φ(s') − Φ(s)` à la récompense ne change pas la politique optimale. Justification de
  fond du choix « différence de distance » plutôt que « distance » en LOT-ANNEXE-08.
- Mnih, V., Kavukcuoglu, K., Silver, D. et al. (2015). *Human-level control through deep
  reinforcement learning*. Nature 518, 529–533. — DQN sur Atari ; source de l'empilement
  d'observations (*frame stacking*, §3.4) comme réponse standard à l'observation partielle, et
  référence de l'algorithme DQN lui-même (chapitre [PPO et DQN](@ref guide-annexe-ppo-dqn)).
- Amodei, D., Olah, C., Steinhardt, J., Christiano, P., Schulman, J., Mané, D. (2016). *Concrete
  Problems in AI Safety*. arXiv:1606.06565. — recense le *reward hacking* (§6.2) : un agent
  optimise la fonction écrite, pas l'intention du concepteur, avec des exemples de récompenses
  détournées de leur objectif.
