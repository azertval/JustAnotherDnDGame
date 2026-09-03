# TACHE-08 — Actes III et IV : terrain, dangers et plateformes {#lot-65-tache-08-actes-terrain-dangers}

**Lot :** [LOT-65](epic.md) · **Emplacement :** `Source/Elements/Levels` · **Statut :** non commencé

## Contexte
C'est le périmètre le plus abîmé de la séquence livrée : **six** des huit tableaux concernés se
franchissent en maintenant « droite », et quatre posent leur sujet **hors d'atteinte du
personnage** — les quatre variantes de plafond incliné, les deux concaves de plafond, et l'ensemble
des dangers, alcôves et interrupteur compris. Le `dangerSwitched` livré ne peut donc jamais être
commuté, et `demo-plafond` se réduit à dix-huit cases de marche en ligne droite sous des tuiles que
la physique interdit d'atteindre.

Deux tableaux sont par ailleurs redondants : `demo-pente` et `demo-arrondi` sont le même tracé à une
tuile près.

## Travail à réaliser

### Acte III — Terrain
| # | Tableau | Refonte |
|---|---|---|
| 13 | `demo-pente` | **Fusion** de `demo-pente` et `demo-arrondi` : pentes *et* arrondis sur plusieurs cases, montée **et** descente. `demo-arrondi.json` est supprimé. |
| 14 | `demo-pente-gauche` | Miroir gauche (pente, arrondi, concave), aller-retour plutôt qu'une descente unique. |
| 15 | `demo-plafond` | **Le plafond du couloir EST la silhouette inclinée** : les quatre variantes bloquent réellement le saut au lieu de flotter hors de portée. |
| 16 | `demo-concave` | Concaves de sol sur le chemin, concaves de plafond en plafond **bas réel**. |

### Acte IV — Dangers et plateformes
| # | Tableau | Refonte |
|---|---|---|
| 17 | `demo-dangers-directionnels` | Les quatre orientations **sur** le chemin : bandes mortelles à longer, face non mortelle servant d'appui — c'est précisément ce que `EX-GP-050` promet et que rien ne montre. |
| 18 | `demo-dangers-avances` | `dangerMover` horizontal **et** vertical à esquiver, `dangerBlink` à cadencer sur plusieurs déphasages, `dangerSwitched` dont l'interrupteur est **atteignable** et doit être laissé dans le bon état. Croisement `DangerMover` × bloc. |
| 19 | `demo-plateforme` | Plateforme horizontale **et** verticale, portant un **bloc poussable** (`EX-GP-026`, jamais démontré) ; chute mortelle réelle ; wall jump depuis une plateforme. **Aucune pente dans ce fichier.** |
| 20 | `demo-budget` | Trajet dont le nombre de sauts **égale exactement** le budget, plus un segment à dash borné : première utilisation de `dashBudget` du jeu. |

## Fichiers impactés
- `Source/Elements/Levels/demo-{pente,pente-gauche,plafond,concave,dangers-directionnels,dangers-avances,plateforme,budget}.json`
- `Source/Elements/Levels/demo-arrondi.json` — **supprimé**.
- `Source/Elements/Levels/sequence-demo.json`.
- `Source/Test/Systeme/test_parcours_complet.cpp` — scripts d'entrées.

## Tests (obligatoires)
- Chaque tableau franchi par un script déterministe qui emploie sa mécanique ; aucun franchissable
  en maintenant « droite ».
- Le garde-fou de proximité au trajet (`TACHE-05`) est vert sur ces huit tableaux : plus aucune
  tuile de mécanique hors de portée d'un saut depuis le trajet réel.
- `demo-budget` échoue si l'on retire un saut au budget — la contrainte doit **mordre**.
- `scripts/check_demo_sequence.py` vert après suppression de `demo-arrondi.json`.

## Points d'attention
- **Défaut moteur connu, non corrigé** : la seule présence d'une configuration de `movingPlatform`
  dans un niveau, même immobile et loin du personnage, casse la résolution de collision pendant le
  suivi d'une pente **ailleurs dans ce même niveau**. `demo-plateforme` ne contient donc ni pente,
  ni arrondi, ni concave — et aucun tableau à pente ne contient de plateforme.
- **Un plafond bas se dimensionne à la physique, pas à l'œil** : le personnage mesure 0,8 de haut et
  un saut simple monte de ≈ 2,4 tuiles. Un plafond censé bloquer doit être vérifié par le test, pas
  supposé.
- **Un danger sur le chemin doit rester franchissable** sans réflexe impossible : la bande mortelle
  de `core::dangerHitbox` est étroite, et la face opposée de la case est un appui légitime.
- Les tableaux de dangers sont ceux où la doctrine « introduire avant d'employer » compte le plus :
  chaque variante se montre inoffensive avant d'être mortelle sur le chemin.

## Définition de fait (DoD)
- Les huit tableaux sont redessinés et franchissables par un script déterministe employant leur
  mécanique ; `demo-arrondi.json` est supprimé et la séquence à jour ; les garde-fous anti-couloir
  et de proximité sont verts sur ce périmètre ; `dashBudget` est employé.

## Exigences
`EX-GP-003`, `EX-GP-004`, `EX-GP-006`, `EX-GP-007` (pentes, arrondis, plafonds, concaves),
`EX-GP-050` à `EX-GP-053` (dangers directionnels, mobiles, commutés, temporisés), `EX-GP-026`
(plateforme mobile), `EX-GP-024` (budget de mouvements), `EX-GP-022` (bloc poussable),
`EX-LVL-012` (progression).
