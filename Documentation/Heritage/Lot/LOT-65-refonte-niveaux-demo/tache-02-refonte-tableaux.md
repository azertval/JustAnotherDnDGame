# TACHE-02 — Refonte des tableaux existants {#lot-65-tache-02-refonte-tableaux}

**Lot :** [LOT-65](epic.md) · **Emplacement :** `Source/Elements/Levels` · **Statut :** fait

## Réalisé
Les dix-sept tableaux ont été redessinés (nouvelle géométrie, jamais recopiée depuis
l'historique git), habillés (fond, décors, jeu de skins `kenney` pour trois d'entre eux), et
portent un cadrage explicite — seize en niveau entier, `demo-salles.json` par salle,
`demo-final.json` en suivi. Chaque tableau a été vérifié individuellement par `ctest`, en
itérant sur la géométrie et les scripts d'entrée jusqu'au franchissement (voir les commentaires
de `Source/Test/Systeme/test_parcours_complet.cpp`, un par tableau).

**Défaut de moteur découvert et consigné (`CHANGELOG.md`), pas corrigé ici** : la seule présence
d'une configuration de plateforme mobile dans un niveau — même immobile, même loin du
personnage — casse la résolution de collision pendant le suivi d'une pente ailleurs dans le même
niveau. Isolé par bissection (retirer uniquement la tuile `movingPlatform` d'une copie de
travail suffit à faire disparaître l'échec). `demo-final.json` n'associe donc pas la plateforme
mobile aux autres mécaniques ; le mécanisme reste couvert isolément par `demo-plateforme.json`.

## Contexte
Les dix-sept tableaux hérités du `LOT-25` étaient des **bancs d'essai** : une grille de tuiles nue,
sans fond, sans décor, sans skin explicite, sans cadrage choisi. Ils remplissaient parfaitement le
rôle que le `LOT-25` leur donnait — démontrer qu'une mécanique fonctionne — et ne montraient rien du
travail des seize lots d'habillage qui ont suivi. `TACHE-00` les a retirés, avec la séquence qui les
ordonnait : cette tâche ne les **édite** donc pas, elle en **recrée** l'essentiel — au minimum une
introduction par mécanique déjà couverte avant `TACHE-00` — avec la banque d'assets renforcée par
`TACHE-00` et le cadrage du `LOT-64`, plutôt que d'hériter de leur géométrie d'origine.

C'est le contenu qui accompagne le binaire téléchargé. Un joueur qui lance la `0.1.0` juge le jeu
là-dessus.

## Travail à réaliser
- **Recréer et habiller chaque tableau** : fond de niveau (`EX-REN-044`), décors d'arrière-plan et de premier
  plan avec parallaxe (`EX-DEC-002`), skins de tuiles et raccords automatiques, animations là où
  elles existent. Réutiliser les assets du dépôt et les scripts de génération ; aucun art original.
- **Choisir explicitement le cadrage** de chaque tableau (`EX-LVL-006`, `LOT-64`) plutôt que de
  subir la règle automatique : un tableau de puzzle se voit en entier, un tableau d'adresse suit le
  personnage, un tableau à salles garde la coupure nette.
- **Revoir la progression de difficulté** : une mécanique introduite à la fois, le premier tableau
  servant de tutoriel **implicite** sans texte — c'est déjà la ligne directrice de `niveaux.md`,
  mais l'ordre actuel a été composé par accumulation.
- **Vérifier chaque tableau contre les lignes directrices** : aucune situation sans issue, aucun
  blocage définitif sans échec possible.
- **Conserver l'esprit d'isolement** : ces tableaux restent des démonstrations d'une mécanique. Les
  combinaisons appartiennent à la `TACHE-03`.
- **Nommer librement** : la séquence étant une donnée (`LOT-59`) et les fichiers d'origine retirés
  par `TACHE-00`, un nom différent de celui d'avant ne coûte qu'une entrée du fichier de séquence.

## Fichiers impactés
- `Source/Elements/Levels/demo-*.json` (recréés), fichier de séquence.
- `Source/Elements/Assets/skins.json`, `Source/Elements/Assets/Backgrounds/`, `Decors/` — assets
  complémentaires générés si besoin.
- `Source/Test/Systeme/test_parcours_complet.cpp` — séquence rejouée.
- `Source/Elements/Levels/README.md`.

## Tests (obligatoires)
- Chaque tableau reste **franchissable** de bout en bout (`EX-NFR-021`) — après refonte, pas avant.
- Chaque tableau **charge et valide** sans erreur, y compris les nouveaux champs (fond, cadrage,
  décors).
- Aller-retour de sérialisation sur chaque tableau : chargé puis réécrit, le modèle est identique.
- `python scripts/check_demo_sequence.py` reste vert après renommages et réordonnancements.
- Le budget de primitives de chaque tableau reste sous son plafond ([LOT-62](@ref lot-62)) — c'est
  la première fois que du contenu réellement habillé est mesuré, et le moment où un dépassement
  apparaîtra s'il doit apparaître.

## Points d'attention
- **La franchissabilité est fragile.** Modifier la géométrie d'un tableau pour l'habiller peut le
  rendre infranchissable de façon subtile — un saut qui passait de justesse. Rejouer le test
  système après **chaque** tableau, pas à la fin.
- **Ne pas corriger les défauts de moteur découverts ici.** Les consigner ; c'est une décision de
  cadrage du lot, et elle protège la relecture.
- Le piège de chemin d'asset déjà rencontré avec `TextureCache` : un asset rangé dans un
  sous-dossier doit être désigné par le **même** chemin partout, préfixe compris. Les tests sans
  GPU ne couvrent pas cette glu — c'est un point d'essai manuel.
- Un tableau habillé mais illisible est un échec : le fond et les décors ne doivent jamais rendre
  ambiguë la lecture du plan physique. La bascule `F8` et les ombres du `LOT-55` sont là pour
  arbitrer.
- Attention au volume : chaque fond est une image versionnée. Réutiliser plutôt que multiplier.

## Définition de fait (DoD)
- Les tableaux recréés couvrent au moins les dix-sept rôles hérités, sont habillés, portent un
  cadrage choisi, suivent une progression revue à une
  mécanique par tableau, restent franchissables, valides, sous leur budget de primitives, et le
  garde-fou de séquence est vert ; les défauts découverts sont consignés sans être corrigés.

## Exigences
Réutilise `EX-LVL-012` (niveaux de démonstration), `EX-LVL-006` (cadrage), `EX-LVL-013` (séquence
en donnée), `EX-LVL-004` (validation), `EX-NFR-021` (franchissabilité), `EX-REN-044` (fond),
`EX-DEC-002` (couches de décor), `EX-REN-005` (animations), `EX-NFR-005` (budget de primitives).
