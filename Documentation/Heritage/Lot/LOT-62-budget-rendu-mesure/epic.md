# LOT-62 — Budget de rendu mesuré {#lot-62}

> Statut : **fait**. Prérequis : [LOT-40](@ref lot-40) (composition, culling, compteurs et
> *QuadRecorder*). Bénéficie de [LOT-53](@ref lot-53), qui est le premier système capable
> d'émettre sans borne.

## Objectif
Transformer deux exigences aujourd'hui **déclaratives** en garanties **assertées**.

`EX-NFR-005` demande que le nombre de primitives émises par image reste « borné et **observable** ».
Les compteurs existent bel et bien — `hmi::ComposedScene::statistics()` expose primitives composées,
soumises et lots — mais ils ne sortent que dans une trace, consommée nulle part ailleurs que par
`hmi::SpriteRenderer` pour éviter de journaliser deux fois la même valeur. Aucun test ne vérifie
qu'un niveau livré reste dans un budget ; la seule mesure de bout en bout du programme d'habillage a
été **manuelle**, à la `TACHE-02` du `LOT-55`.

`EX-NFR-001` demande 60 images par seconde sur une configuration de bureau récente. Rien ne mesure
la cadence, et rien ne l'affiche : on ne sait ni si elle est tenue, ni où elle se dégraderait.

Le moment est le bon : sept calques ont été ajoutés en seize lots, et `LOT-53` introduira le premier
système capable d'émettre un nombre variable de primitives. Un budget qu'on ne mesure pas est un
budget qu'on dépasse.

## Périmètre

### Inclus
- **Test de non-régression du volume de primitives** sur les niveaux livrés : pour une caméra et un
  mode de rendu donnés, le nombre de primitives composées et soumises reste sous un plafond nommé.
- **Efficacité du culling assertée** : sur un grand niveau, la fraction écartée par le culling reste
  au-dessus d'un seuil — c'est ce qui distingue un culling qui fonctionne d'un culling qui compile.
- **Compteur de cadence et de primitives affichable** en jeu, activable par une touche ou une
  option, en réutilisant l'affichage tête haute du `LOT-52`.
- **Mesure de référence consignée** : valeurs par niveau livré, à la date du lot, dans la
  documentation.

### Exclus (hors périmètre de ce lot)
- Optimisation du rendu : ce lot **mesure**, il n'accélère pas. Si une mesure révèle un problème,
  il est consigné, pas corrigé ici.
- Test de performance chiffré en CI (images par seconde sur un *runner*) : une machine virtuelle
  partagée ne mesure pas une cadence de manière reproductible. Le volume de primitives, lui, est
  déterministe et se teste sans GPU — c'est précisément pourquoi c'est **lui** qu'on assert.
- Profilage GPU, instrumentation par étape du pipeline, requêtes de temps Direct3D.
- Budget mémoire des textures.

## Décisions de cadrage
- **Asserter le déterministe, afficher le reste.** Le nombre de primitives d'une scène est une
  fonction pure de l'état et de la caméra : il se teste sans GPU, exactement comme `LOT-40` l'a
  rendu possible (`EX-NFR-004`). La cadence, elle, dépend de la machine : elle s'affiche pour être
  observée, elle ne se teste pas en CI. Confondre les deux donnerait un test instable qu'on
  finirait par désactiver.
- **Un plafond par niveau, pas un plafond global.** `demo-salles` et `demo-deplacement` n'ont rien
  de comparable ; un plafond unique serait soit inutile, soit faux.
- **Des plafonds larges.** Le rôle du test est d'attraper un facteur deux accidentel — un calque
  émis deux fois, un culling contourné — pas de figer un chiffre exact qu'il faudrait mettre à jour
  à chaque lot de contenu.
- **Réutiliser l'affichage tête haute existant** (`LOT-52`) plutôt que d'ouvrir un second chemin de
  texte dans la scène.
- **Ce lot ne corrige rien.** Séparer la mesure de l'optimisation évite le travers classique où
  l'on ajuste le plafond jusqu'à ce que le test passe.

## Exigences couvertes
- Aucune nouvelle exigence. Ce lot **honore** des exigences existantes : `EX-NFR-005` (primitives
  bornées et observables) et `EX-NFR-001` (60 images par seconde), qui n'ont jamais eu de moyen de
  vérification.
- Réutilisées : `EX-NFR-004` (rendu vérifiable sans GPU), `EX-REN-015` (cadrage par salle),
  `EX-REN-043` (calques), `EX-REN-021` (pas fixe), `EX-IHM-003` (affichage tête haute),
  `EX-ARCH-012` (rendu sans effet sur la simulation).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-test-budget-primitives.md) | Test de non-régression du volume de primitives et de l'efficacité du culling | `Source/Test` | ✅ |
| [TACHE-02](tache-02-compteur-affichable.md) | Compteur de cadence et de primitives affichable en jeu | `Source/HMI/Game`, `Source/HMI/Graphics` | ✅ |
| [TACHE-03](tache-03-documentation-verification.md) | Documentation, mesure de référence et vérification | `Documentation` | ✅ |

## Critères d'acceptation du lot
1. Chaque niveau livré est couvert par un test asserant son volume de primitives sous un plafond
   nommé, en mode Physique **et** en mode Texture.
2. Émettre accidentellement un calque en double fait **échouer** un test — démontré par un test
   négatif.
3. Le culling écarte une fraction assertée des primitives d'un grand niveau.
4. Un compteur de cadence et de primitives s'affiche en jeu sur commande, sans effet sur la
   simulation.
5. Les mesures de référence sont consignées avec leur date.
6. Les tests s'exécutent **sans GPU** et restent stables d'une exécution à l'autre.
7. Build `/W4 /WX`, `ctest` à 100 %, Doxygen et lints verts.

## Dépendances
Bâtit sur [LOT-40](@ref lot-40) (composition, culling, statistiques, *QuadRecorder*),
[LOT-51](@ref lot-51) (visibilité par calque) et [LOT-52](@ref lot-52) (affichage tête haute).
Protège [LOT-53](@ref lot-53), dont le budget de particules devient vérifiable de bout en bout.

## Navigation des tâches
- @subpage lot-62-tache-01-test-budget-primitives
- @subpage lot-62-tache-02-compteur-affichable
- @subpage lot-62-tache-03-documentation-verification
