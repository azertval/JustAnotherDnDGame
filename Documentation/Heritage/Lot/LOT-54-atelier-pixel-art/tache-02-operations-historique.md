# TACHE-02 — Opérations sur tampon de pixels et historique nommé {#lot-54-tache-02-operations-historique}

**Lot :** [LOT-54](epic.md) · **Emplacement :** `Source/HMI/Editor` · **Statut :** fait

## Contexte
Le cœur de l'atelier, sous sa forme la plus testable : ce que fait chaque outil sur un tampon de
pixels, et comment on revient en arrière. Rien de tout cela n'a besoin de Qt, d'un widget ou d'un
GPU ; le canevas (TACHE-03) et les actions (TACHE-04) se contenteront ensuite d'appeler ces
fonctions.

Cette séparation n'est pas un principe abstrait : c'est ce qui permet de couvrir par des tests les
deux pièges classiques d'un canevas pixel art — le glisser rapide qui laisse des trous, et le
remplissage récursif qui déborde la pile — sans jamais instancier d'application.

L'historique, lui, porte une contrainte nouvelle par rapport au cadrage initial du lot : chaque
opération est **nommée**. Ce nom sert deux fois — dans le libellé de l'action « Annuler *…* »
(TACHE-04, sur les actions de [LOT-56](@ref lot-56)) et dans le panneau d'historique visuel. Sans
lui, l'action afficherait « Annuler » sans dire quoi, ce qui est précisément l'ambiguïté que la
cohabitation de deux historiques crée.

## Travail à réaliser
- **Opérations sur un tampon RGBA** : poser un pixel, effacer (mettre à transparent), tracer la
  **ligne entre deux positions** successives, **remplir une zone contiguë** de même couleur,
  **prélever** la couleur sous une position. Toutes en fonctions libres sur un tampon et ses
  dimensions, sans état global.
- **Remplissage itératif**, jamais récursif, avec une file explicite.
- **Historique local** : pile d'annulation et pile de rétablissement, propres au canevas et
  **totalement indépendantes** de `core::LevelDraft`. Une nouvelle opération après une annulation
  vide la pile de rétablissement.
- **Opérations nommées** : chaque entrée de l'historique porte une clé de traduction décrivant
  l'opération (pinceau, gomme, remplissage, déplacement de région, symétrie…). L'historique expose la
  liste ordonnée de ces noms, ce qui suffit à alimenter le panneau visuel de la TACHE-04.
- **Retour à un point antérieur** : annuler jusqu'à une entrée donnée de la liste, en une opération,
  plutôt que par annulations successives.
- **Coût mémoire borné** : mémoriser la région modifiée par une opération plutôt qu'une copie
  complète de l'image, et plafonner la profondeur de l'historique.

## Fichiers impactés
- `Source/HMI/Editor/PixelOperations.{h,cpp}` (nouveau) — opérations pures sur un tampon.
- `Source/HMI/Editor/PixelHistory.{h,cpp}` (nouveau) — historique nommé, annulation et rétablissement.
- `Source/Elements/Localization/fr.lang`, `en.lang` — noms d'opérations.
- `Source/Test/Unit/HMI/Editor/test_pixel_operations.cpp` (nouveau).
- `Source/Test/Unit/HMI/Editor/test_pixel_history.cpp` (nouveau).

## Tests (obligatoires)
- **Remplissage par zone contiguë** : zone fermée, zone ouverte débouchant sur un bord, remplissage
  sur la couleur déjà présente (aucun changement, et surtout pas de boucle infinie), image d'un seul
  pixel, image entièrement d'une seule couleur.
- **Tracé entre deux positions** : un glisser rapide ne doit pas laisser de trous — la ligne entre
  deux positions successives est remplie, y compris pour un déplacement diagonal ou purement
  vertical.
- **Annuler/refaire** : suite d'opérations, annulation partielle, nouvelle opération après annulation
  (la pile de rétablissement est vidée), annulation au-delà du fond de pile sans effet.
- **Retour à un point antérieur** : sauter directement à l'entrée *n* donne exactement le même tampon
  que *n* annulations successives.
- **Noms d'opérations** : chaque type d'opération produit une clé de traduction, et chaque clé existe
  dans les deux catalogues.
- **Profondeur plafonnée** : au-delà de la limite, les entrées les plus anciennes sont oubliées sans
  corrompre l'état courant.
- Opérations **pures**, testées sans Qt ni GPU.

## Points d'attention
- **Le tracé entre positions n'est pas optionnel.** Sans lui, un glisser rapide produit des pixels
  isolés — c'est le défaut le plus visible d'un canevas pixel art fait naïvement.
- **Le remplissage doit être itératif**, pas récursif : une récursion sur une grande zone
  déborderait la pile.
- **Une opération est un geste, pas un pixel.** Un coup de pinceau maintenu produit **une** entrée
  d'historique, sinon annuler devient inutilisable et l'aperçu live (TACHE-08) invalide le cache à
  chaque déplacement de souris.
- Le format des pixels reste `Format_RGBA8888`, non prémultiplié, comme au décodage et à l'encodage
  (TACHE-01) : mélanger les conventions dégraderait les bords des sprites à chaque aller-retour.

## Définition de fait (DoD)
- Les opérations de pinceau, gomme, ligne, remplissage et pipette existent en fonctions pures et sont
  testées sans Qt ; l'historique est local, nommé, plafonné, et permet le retour à un point
  antérieur ; les clés de traduction existent dans les deux catalogues ; `/W4 /WX` propre.

## Exigences
`EX-EDIT-045` (outil de dessin pixel art) ; réutilise `EX-EDIT-030` (éditeur intégré),
`EX-REN-033` (traduction), `EX-NFR-010` (testable sans GPU).
