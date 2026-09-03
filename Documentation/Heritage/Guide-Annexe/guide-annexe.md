# Guide Annexe — Notions d'IA {#guide-annexe}

Ce guide est le pendant pédagogique du programme [Lot-Annexe](@ref lots-annexe) : là où
`Documentation/Guide/` explique le moteur de jeu à quelqu'un qui sait déjà programmer, ce guide
explique les **notions d'apprentissage automatique** à quelqu'un qui ne les connaît pas encore,
depuis zéro, sans supposer d'accès à Internet pour combler les trous.

Chaque chapitre est **autonome** : il part des bases (souvent des mathématiques du lycée/prépa) et
construit, étape par étape, la notion nécessaire à un ou plusieurs lots du programme. Chaque
chapitre se termine par une section **Sources**, qui cite les articles ou ouvrages d'origine — pour
donner crédit, pour permettre d'aller plus loin une fois de retour en ligne, et pour que rien ne
soit présenté comme « inventé ici » alors que c'est un résultat établi de la littérature.

**Comment lire ce guide** : les chapitres sont numérotés dans l'ordre où les notions deviennent
nécessaires en suivant les lots dans l'ordre de leur génération (@ref lots-annexe). Chaque
`tache-NN.md` d'un lot renvoie vers le(s) chapitre(s) pertinents dans sa section « Notions
abordées » plutôt que de répéter l'explication.

## Chapitres

- @subpage guide-annexe-algebre-tensorielle — vecteurs, matrices, tenseurs : le langage de calcul
  commun à tout le reste (LOT-ANNEXE-01).
- @subpage guide-annexe-autodiff — dérivées, règle de la chaîne, différentiation automatique en
  mode *reverse* (LOT-ANNEXE-02).
- @subpage guide-annexe-reseaux-neurones — neurone, couche, réseau, fonctions d'activation,
  pourquoi l'initialisation des poids compte (LOT-ANNEXE-03).
- @subpage guide-annexe-optimisation — descente de gradient, inertie, Adam : comment un gradient
  devient une mise à jour de poids (LOT-ANNEXE-04).
- @subpage guide-annexe-apprentissage-renforcement — vocabulaire de base de l'apprentissage par
  renforcement (agent, environnement, état, action, récompense, politique, épisode), puis les deux
  choix de conception qui décident de la réussite d'un agent : **concevoir une observation**
  (one-hot, normalisation, observation partielle — LOT-ANNEXE-06) et **concevoir une récompense**
  (*shaping*, *reward hacking*, shaping par potentiel — LOT-ANNEXE-08). (LOT-ANNEXE-05 à 09.)
- @subpage guide-annexe-algorithmes-evolutionnistes — sélection naturelle appliquée à des poids de
  réseau : population, mutation, sélection (LOT-ANNEXE-10/11).
- @subpage guide-annexe-reinforce — le premier algorithme d'apprentissage par renforcement à base
  de gradient : policy gradient et REINFORCE (LOT-ANNEXE-12).
- @subpage guide-annexe-acteur-critique — pourquoi REINFORCE est bruyant, et comment un second
  réseau (le critique) réduit ce bruit (LOT-ANNEXE-13).
- @subpage guide-annexe-ppo-dqn — deux façons de stabiliser encore l'apprentissage : PPO (limiter
  le pas) et DQN (apprendre une valeur plutôt qu'une politique) (LOT-ANNEXE-14).
- @subpage guide-annexe-evaluation-rl — pourquoi un seul rejeu ne prouve rien, et comment mesurer
  honnêtement un agent entraîné (LOT-ANNEXE-15/16).

## Convention de notation utilisée dans tout le guide

- Les scalaires sont en minuscule italique (`x`, `a`), les vecteurs/matrices/tenseurs en gras
  (`W`, `x`), cohérent avec la plupart des manuels cités en source.
- `∂f/∂x` note la dérivée partielle de `f` par rapport à `x` — au premier chapitre qui en a besoin
  (différentiation, @ref guide-annexe-autodiff), la notion est réexpliquée avant d'être utilisée.
- Le code C++ montré en exemple est **illustratif**, pas le code final du lot — le detail exact
  d'implémentation vit dans le `tache-NN.md` correspondant, qui reste la référence normative pour
  ce qui doit réellement être écrit.
