# LOT-04 — Documentation Doxygen & réorganisation de l'arborescence documentaire {#lot-04}

> Statut : **terminé**. Le site Doxygen offre désormais une page d'accueil, les spécifications, les lots et un manuel — navigables — en plus de la référence de code. Les documents projet vivent sous `Documentation/`.

## Objectif
Transformer la documentation générée en un site **utilisable** : une **page d'accueil** décrivant le projet, les **spécifications** (conventions incluses), les **lots** et un premier **manuel utilisateur**, le tout navigable et publié sur `gh-pages`. En préalable, **réorganiser l'arborescence** documentaire : `Specification/` et `Lot/` deviennent des sous-dossiers de `Documentation/`.

À l'issue du lot : ouvrir l'URL GitHub Pages affiche une page d'accueil claire, d'où l'on navigue vers les spécifications, les lots et le manuel, en plus de la référence de code.

## Périmètre

### Inclus
- **Réorganisation** : `Specification/` et `Lot/` déplacés sous `Documentation/` ; `conventions.md` intégré aux spécifications ; toutes les références mises à jour.
- **Configuration Doxygen** : ingestion du Markdown (`Documentation/**`) en plus du code (`Source/`), page principale (`USE_MDFILE_AS_MAINPAGE`).
- **Page d'accueil** (`index.md`) décrivant le projet.
- **Pages de spécification** navigables (hiérarchie explicite), conventions comprises.
- **Pages de lots** navigables (epics + tâches).
- **Manuel utilisateur** : squelette + une première page réelle.
- **Traçabilité des exigences** : IDs `EX-…` stables (ancres Doxygen `@ref`), lint CI (doublons, références orphelines, prochain numéro libre).
- **CI docs** : génération avec garde-fou `WARN_AS_ERROR`, déploiement `gh-pages` complet.

### Exclus (lots ultérieurs)
- Contenu détaillé du manuel utilisateur au-delà de la première page (prise en main avancée, tutoriels).
- Internationalisation de la documentation.
- Thème/CSS personnalisé au-delà du thème Doxygen par défaut.

## Décisions de cadrage
- **Manuel utilisateur** : squelette **+ une première page réelle** (télécharger / lancer la release Debug), le reste différé.
- **Navigation** : hiérarchie **explicite** via `@page` / `@subpage` (arbre Accueil → Spécifications → … ; Lots → LOT-xx → tâches).
- **Ordre des spécifications** : porté par la **liste `@subpage`** de l'index (source unique), et non par des préfixes de fichiers. Les specs perdent leur préfixe numérique (`00-vision.md` → `vision.md`) et reçoivent une **ancre stable** (`{#spec-vision}`) ; insérer/réordonner une spec = éditer l'index, sans renuméroter ni casser de lien. `conventions.md` rejoint la rubrique sans numéro.
- **Identifiants d'exigences (`EX-…`)** : traités comme des **identifiants immuables**, jamais renumérotés ni réutilisés (ils sont référencés dans les lots **et** dans le code). Règle : allocation au **prochain numéro libre**, l'ordre de lecture = l'ordre du document (pas le numéro). **Aucun ID existant n'est renuméroté** (`EX-VIS` inclus, laissé tel quel) ; l'auteur reste vigilant à ne pas créer de doublon, **le lint CI l'imposant** (échec sur doublon). Chaque exigence devient une **ancre Doxygen** référençable par `@ref` (référence cassée = échec CI via `WARN_AS_ERROR`), et le **lint CI** vérifie doublons / références orphelines et affiche le prochain numéro libre.
- **Qualité** : `WARN_AS_ERROR = YES` en CI — un avertissement Doxygen (référence cassée, symbole non documenté) fait échouer la génération, dans le même esprit que `/W4 /WX` côté code.

## Exigences couvertes
- `EX-NFR-012` — guide de conventions (dont le lien est corrigé et le contenu intégré aux spécifications).
- Prépare une exigence « documentation publiée et navigable » (à formaliser en TACHE-04 si retenu).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement principal | État |
|-------|----------|-----------------------|:----:|
| [TACHE-01](tache-01-reorganisation-arbo.md) | Réorganisation de l'arborescence documentaire | racine → `Documentation/` | ✅ Fait |
| [TACHE-02](tache-02-config-doxygen-markdown.md) | Configuration Doxygen pour le Markdown | `Documentation/Doxyfile` | ✅ Fait |
| [TACHE-03](tache-03-page-accueil.md) | Page d'accueil du projet (mainpage) | `Documentation/index.md` | ✅ Fait |
| [TACHE-04](tache-04-pages-specification.md) | Pages de spécification navigables (+ conventions) | `Documentation/Specification/` | ✅ Fait |
| [TACHE-05](tache-05-pages-lots.md) | Pages de lots navigables | `Documentation/Lot/` | ✅ Fait |
| [TACHE-06](tache-06-manuel-utilisateur.md) | Manuel utilisateur (squelette + 1re page) | `Documentation/Manuel/` | ✅ Fait |
| [TACHE-07](tache-07-ci-docs.md) | CI documentation (WARN_AS_ERROR, déploiement) | `.github/workflows/docs.yml` | ✅ Fait |
| [TACHE-08](tache-08-tracabilite-exigences.md) | Traçabilité des exigences (IDs stables, ancres, lint CI) | `Documentation/Specification/`, CI | ✅ Fait |

## Critères d'acceptation du lot
1. `Specification/` et `Lot/` vivent sous `Documentation/` ; `conventions.md` fait partie des spécifications ; **aucune référence cassée** dans le dépôt (README, CONTRIBUTING, CHANGELOG, READMEs `Source/`, liens internes).
2. `doxygen Doxyfile` génère un site avec une **page d'accueil** décrivant le projet (et non la page vide par défaut).
3. Depuis l'accueil, on **navigue** vers les spécifications (conventions incluses), les lots et le manuel ; la hiérarchie de nav est explicite.
4. Le **manuel** comporte au moins une page réelle (télécharger / lancer la release).
5. Les exigences `EX-…` sont des **ancres** référençables par `@ref` ; le **lint CI** échoue sur un ID en double ou une référence orpheline et sait afficher le prochain numéro libre par catégorie.
6. La génération Doxygen ne produit **aucun avertissement** (`WARN_AS_ERROR`) ; la CI `docs.yml` publie l'ensemble sur `gh-pages`.
7. `CHANGELOG.md` mis à jour ; `main` reste cohérente.

## Dépendances
- S'appuie sur la CI documentaire existante (`.github/workflows/docs.yml`) et le `Doxyfile` actuel.
- Aucun impact sur le code de `Source/` (lot purement documentaire et outillage).

## Navigation des tâches
- @subpage lot-04-tache-01-reorganisation-arbo
- @subpage lot-04-tache-02-config-doxygen-markdown
- @subpage lot-04-tache-03-page-accueil
- @subpage lot-04-tache-04-pages-specification
- @subpage lot-04-tache-05-pages-lots
- @subpage lot-04-tache-06-manuel-utilisateur
- @subpage lot-04-tache-07-ci-docs
- @subpage lot-04-tache-08-tracabilite-exigences
