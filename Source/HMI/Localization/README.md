# HMI/Localization/

Résolution des textes d'interface **par clé**, jamais par chaîne écrite en dur dans un widget.

- `Localization` — catalogue de traduction : `text("menu.quit")` rend le libellé de la langue
  active, avec **repli** sur la langue par défaut si la clé y manque, puis sur la clé elle-même si
  elle manque partout. Une clé absente se voit donc à l'écran au lieu de laisser un texte vide, et
  ne fait jamais échouer l'application.

Les catalogues eux-mêmes sont des données, pas du code : ils vivent dans
[`../../Elements/Localization/`](../../Elements/Localization/README.md) (`fr.lang`, `en.lang`).

Le catalogue sert les libellés de règles du jeu (`HMI/Runtime/RuleLabels`) et les dialogues ; les
écrans du jeu traduisent leurs textes par `qsTr`. L'éditeur n'en lit plus rien : outil interne, il
écrit ses textes en anglais dans le code (`LOT-EDITOR-01`).

Réf. specs : `EX-REN-033` ; guide [`guide-ecrans`](../../../Documentation/Guide/guide-ecrans.md).
