# Policy gradient et REINFORCE {#guide-annexe-reinforce}

**Lot concerné :** [LOT-ANNEXE-12](@ref lot-annexe-12) (policy gradient maison, REINFORCE).
**Prérequis :** [Autodiff](@ref guide-annexe-autodiff), [Réseaux de neurones](@ref
guide-annexe-reseaux-neurones), [Optimisation](@ref guide-annexe-optimisation),
[vocabulaire RL](@ref guide-annexe-apprentissage-renforcement).

## Pourquoi ce chapitre

Le chapitre [algorithmes évolutionnistes](@ref guide-annexe-algorithmes-evolutionnistes) apprend
sans jamais calculer de gradient. Ce chapitre montre comment, au contraire, calculer un **vrai**
gradient de la récompense par rapport aux poids d'une politique — c'est l'exigence ferme du
programme (« un vrai modèle appris par rétropropagation ») — via l'algorithme le plus direct de
cette famille : **REINFORCE**.

## 1. Le problème : la récompense n'est pas différentiable

On voudrait ajuster les poids `θ` d'un réseau de politique pour maximiser la récompense cumulée
attendue d'un épisode. Le chapitre [Autodiff](@ref guide-annexe-autodiff) sait dériver n'importe
quelle composition de calculs — mais la récompense d'un épisode dépend d'un **tirage aléatoire**
d'action (la politique est stochastique, chapitre
[vocabulaire RL](@ref guide-annexe-apprentissage-renforcement)) suivi d'une simulation de jeu
(chutes, collisions) qui n'a, en général, aucune formule différentiable connue reliant directement
« ce poids » à « cette récompense ». Il faut donc une autre approche que la dérivation directe.

## 2. L'astuce du log-gradient (*log-derivative trick*)

Voici l'observation mathématique qui rend tout le reste possible. Pour une distribution de
probabilité `π(a|s; θ)` (probabilité de choisir l'action `a` dans l'état `s`, selon des poids `θ`),
la dérivée du **logarithme** de cette probabilité s'écrit, par la règle de dérivation d'un
logarithme (`d(ln x)/dx = 1/x`, combinée à la règle de la chaîne du chapitre
[Autodiff](@ref guide-annexe-autodiff)) :

```
∇θ log π(a|s;θ) = ∇θ π(a|s;θ) / π(a|s;θ)
```

Réarrangé : `∇θ π(a|s;θ) = π(a|s;θ) × ∇θ log π(a|s;θ)`. Cette identité, purement algébrique et
toujours vraie, permet de remplacer un gradient de **probabilité** (difficile à estimer par
échantillonnage) par une **espérance** d'un gradient de **log-probabilité** (facile à estimer : il
suffit d'échantillonner des actions selon `π`, ce que la politique fait déjà naturellement).
C'est la clé qui permet de transformer « je ne peux pas dériver la récompense directement » en
« je peux estimer un gradient malgré tout, à partir d'épisodes joués ».

## 3. La règle de mise à jour REINFORCE

En admettant le résultat ci-dessus (la dérivation complète du **théorème du gradient de politique**
qui en découle dépasse le cadre de ce chapitre — voir Sources), REINFORCE ajuste les poids de la
politique, après un épisode complet, par :

```
θ ← θ + tauxApprentissage × Σ (pour chaque pas t de l'épisode) [ ∇θ log π(aₜ|sₜ;θ) × Gₜ ]
```

où `Gₜ` est le **retour** à partir du pas `t` — la somme actualisée des récompenses futures de
l'épisode à partir de ce pas (voir §4). En pratique, on formule ceci comme une **perte** à
minimiser (pour rester cohérent avec la convention de descente de gradient du chapitre
[Optimisation](@ref guide-annexe-optimisation), qui *soustrait* le gradient) :

```
perte = - Σ (pour chaque pas t) [ log π(aₜ|sₜ;θ) × Gₜ ]
```

(le signe négatif transforme une **maximisation** de récompense en une **minimisation** de perte —
minimiser `perte` revient à maximiser la quantité d'origine).

**Intuition centrale, sans formule** : si une action `a` prise à l'état `s` a été suivie d'un
**bon** retour (`Gₜ` grand et positif), la mise à jour **augmente** la probabilité que la politique
choisisse à nouveau cette action dans cet état ; si le retour a été **mauvais** (`Gₜ` négatif ou
faible), la mise à jour **diminue** cette probabilité. Répété sur de nombreux épisodes, ce mécanisme
pousse progressivement la politique vers les actions qui ont statistiquement mieux fonctionné.

## 4. Le retour actualisé

Le retour à partir du pas `t` est la somme des récompenses futures de l'épisode, chacune pondérée
par un **facteur d'actualisation** `γ` (gamma, entre 0 et 1) élevé à une puissance croissante :

```
Gₜ = rₜ + γ·rₜ₊₁ + γ²·rₜ₊₂ + γ³·rₜ₊₃ + ...
```

`γ` proche de `1` (par exemple `0.99`) valorise presque autant les récompenses lointaines que les
récompenses immédiates ; `γ` plus petit privilégie fortement les récompenses proches. Pour un
épisode qui se termine toujours (ce qui est le cas ici — un niveau du jeu finit par une victoire,
un échec, ou un timeout, chapitre [vocabulaire RL](@ref guide-annexe-apprentissage-renforcement)),
`Gₜ` peut être calculé **exactement**, en repartant de la fin de l'épisode vers le début :

```
Gₜ = rₜ + γ × Gₜ₊₁          (avec G après le dernier pas = 0)
```

C'est un calcul de type « somme cumulée inversée », bien moins coûteux que de recalculer la somme
complète à chaque pas.

## 5. Pourquoi l'exploration stochastique est indispensable pendant l'entraînement

`∇θ log π(a|s;θ)` n'a de sens que si l'action `a` a été **effectivement tirée** selon la
distribution `π` (décodage stochastique, [LOT-ANNEXE-07](@ref lot-annexe-07)) — jamais choisie de
façon déterministe (`argmax`) pendant l'entraînement. Si la politique était toujours certaine de son
choix (probabilité `1` pour une action, `0` pour toutes les autres), son gradient de
log-probabilité serait nul ou mal défini — il n'y aurait littéralement rien à ajuster, puisque la
politique ne considérerait jamais l'alternative « et si j'avais choisi autre chose ? ». Le mode
déterministe (`argmax`) n'intervient qu'une fois l'entraînement terminé, pour figer la politique en
vue de l'export en rejeu ([LOT-ANNEXE-11](@ref lot-annexe-11)).

## 6. Ce que « rétropropager la perte REINFORCE » signifie concrètement

`log π(aₜ|sₜ;θ)` est la sortie d'un calcul qui traverse le réseau de politique (une couche
`softmax`, chapitre [Réseaux de neurones](@ref guide-annexe-reseaux-neurones), suivie d'un
logarithme de la probabilité de l'action effectivement tirée) — c'est donc un **nœud du graphe
d'autodiff** (chapitre [Autodiff](@ref guide-annexe-autodiff)), au même titre que n'importe quel
autre calcul. En multipliant ce nœud par le retour `Gₜ` (une simple valeur numérique, **jamais**
elle-même différentiée — le retour dépend de la trajectoire jouée, pas des poids actuels du réseau
au sens d'un gradient à calculer) et en sommant sur tous les pas de l'épisode, on obtient un
scalaire unique (la perte) sur lequel appeler `backward()` exactement comme pour n'importe quelle
autre perte — **aucune formule de gradient n'est écrite à la main** : c'est le moteur d'autodiff qui
fait tout le travail, jusqu'aux poids de la toute première couche du réseau de politique.

## Sources

- Williams, R.J. (1992). *Simple Statistical Gradient-Following Algorithms for Connectionist
  Reinforcement Learning*. Machine Learning 8, 229–256. — l'article d'origine de REINFORCE ;
  « REINFORCE » est un acronyme donné dans cet article (*REward Increment = Nonnegative Factor ×
  Offset Reinforcement × Characteristic Eligibility*), reflétant la règle de mise à jour du §3.
- Sutton, R.S., McAllester, D., Singh, S., Mansour, Y. (1999/2000). *Policy Gradient Methods for
  Reinforcement Learning with Function Approximation*. Advances in NIPS 12. — établit le
  **théorème du gradient de politique** de façon rigoureuse (dont REINFORCE, antérieur, est un cas
  particulier/une instance pratique) ; la dérivation complète (au-delà de l'astuce du
  log-gradient présentée ici) s'y trouve.
- Sutton, R.S., Barto, A.G. (2018). *Reinforcement Learning: An Introduction* (2nd ed.). MIT Press,
  chapitre 13 (« Policy Gradient Methods »). — présentation pédagogique de REINFORCE, avec preuve
  du théorème du gradient de politique, plus accessible que l'article original.
