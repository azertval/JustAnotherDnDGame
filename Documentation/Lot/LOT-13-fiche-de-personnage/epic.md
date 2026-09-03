# LOT-13 — Fiche de personnage {#lot-13}

> Statut : **à faire**.
> Prérequis : [LOT-12](@ref lot-12).

## Objectif

Donner à toute créature — héros, PNJ, ennemi — une fiche complète : points de vie, classe
d'armure, classe, niveau, maîtrise, compétences, vitesse.

## Périmètre

- `Source/Core/Rpg/CharacterSheet.{h,cpp}` : caractéristiques, PV courants et maximum, CA, niveau,
  bonus de maîtrise, jets de sauvegarde, vitesse (en cases, dérivée de l'échelle du `LOT-12`).
- `ClassDefinition.{h,cpp}` : **pilotée par JSON**, jamais codée en dur. Dé de vie, maîtrises,
  progression par niveau.
- `Skill.h` : compétences et leur caractéristique associée.
- Composant ECS `Source/Core/Ecs/Components/RpgActor.h` référençant la fiche.
- Expérience et montée de niveau.

## Deux règles à tenir

**`CharacterSheet` est un objet autonome, jamais un singleton joueur.** La décision de cadrage est
« un héros au départ, quatre à terme » : le passage au groupe (`LOT-29`) ne doit **rien** changer à
cette classe. Si une fonction prend « le personnage » implicitement, elle est mal écrite.

**Aucune valeur de règle dans le C++** (`EX-VIS-007`). Dé de vie, seuils d'expérience, maîtrises :
tout en JSON. C'est ce qui rend l'équilibrage possible sans recompiler — et sans équilibrage, un
RPG n'est pas jouable.

## Exigences couvertes

`EX-DND-*` : PV, CA, bonus de maîtrise par niveau, jets de sauvegarde, expérience, progression.

## Critères d'acceptation

- Trois classes définies en JSON se chargent et donnent les bons modificateurs.
- Montée de niveau reproductible et testée aux bornes (seuil exact, dépassement, multi-niveaux).
- **Aucune valeur de règle codée en dur** dans le C++ — vérifiable par relecture du diff.
- Un test construit **quatre** fiches indépendantes : rien ne suppose l'unicité.
