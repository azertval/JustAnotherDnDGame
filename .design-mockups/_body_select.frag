<div class="screen" style="--accent: {{accent}}">
  <div class="grid-bg"></div>

  <div style="position: absolute; inset: 0; display: flex; flex-direction: column; padding: 56px 88px 40px;">
    <div style="display: flex; align-items: baseline; justify-content: space-between;">
      <div class="title" style="font-size: 28px;">CHOISIR UN NIVEAU</div>
      <div class="row" style="padding-right: 0;">@CARET@<span>Retour</span></div>
    </div>
    <div style="height: 30px;"></div>

    <div style="display: flex; gap: 8px;">
      <div style="padding: 14px 26px; font-size: 21px; font-weight: 600; color: #12161f; background: var(--accent); box-shadow: 0 -4px 0 var(--edge), -4px 0 0 var(--edge), 4px 0 0 var(--edge), inset 0 4px 0 rgba(255,255,255,.35);">Séquence</div>
      <div style="padding: 14px 26px; font-size: 21px; font-weight: 600; color: var(--muted); background: var(--surfaceAlt); box-shadow: 0 -4px 0 var(--edge), -4px 0 0 var(--edge), 4px 0 0 var(--edge), inset 0 4px 0 var(--ornament);">Niveaux personnels</div>
    </div>

    <div class="frame" style="flex-grow: 1; padding: 24px 0; display: flex; flex-direction: column;">
      <!-- Terminé : pastille pleine -->
      <div class="row" style="padding-left: 34px;">
        @CARET@
        <div style="width: 20px; height: 20px; flex: none; background: #7ee06a;"></div>
        <span style="flex-grow: 1;">01 — Premiers pas</span>
        <span style="font-size: 18px; color: var(--dim); padding-right: 40px;">terminé</span>
      </div>
      <div class="row" style="padding-left: 34px;">
        @CARET@
        <div style="width: 20px; height: 20px; flex: none; background: #7ee06a;"></div>
        <span style="flex-grow: 1;">02 — Sauts et plateformes</span>
        <span style="font-size: 18px; color: var(--dim); padding-right: 40px;">terminé</span>
      </div>
      <!-- Ligne focalisee : niveau atteint, jamais termine -->
      <div class="row row--focus" style="padding-left: 34px;">
        @CARET@
        <div style="width: 20px; height: 20px; flex: none; background: var(--accent);"></div>
        <span style="flex-grow: 1;">03 — Interrupteurs</span>
        <span style="font-size: 18px; color: var(--accent); padding-right: 40px;">en cours</span>
      </div>
      <!-- Verrouille : pastille creuse -->
      <div class="row row--off" style="padding-left: 34px;">
        @CARET@
        <div style="width: 20px; height: 20px; flex: none; box-shadow: inset 0 0 0 4px var(--dim);"></div>
        <span style="flex-grow: 1;">04 — Plateformes mobiles</span>
        <span style="font-size: 18px; padding-right: 40px;">verrouillé</span>
      </div>
      <div class="row row--off" style="padding-left: 34px;">
        @CARET@
        <div style="width: 20px; height: 20px; flex: none; box-shadow: inset 0 0 0 4px var(--dim);"></div>
        <span style="flex-grow: 1;">05 — Pièges temporisés</span>
        <span style="font-size: 18px; padding-right: 40px;">verrouillé</span>
      </div>
      <div class="row row--off" style="padding-left: 34px;">
        @CARET@
        <div style="width: 20px; height: 20px; flex: none; box-shadow: inset 0 0 0 4px var(--dim);"></div>
        <span style="flex-grow: 1;">06 — Le grand saut</span>
        <span style="font-size: 18px; padding-right: 40px;">verrouillé</span>
      </div>
    </div>

    <div style="height: 24px;"></div>
    <div style="display: flex; gap: 24px; justify-content: flex-end;">
      <div class="hint"><span class="key">↑↓</span><span>Naviguer</span></div>
      <div class="hint"><span class="key">←→</span><span>Onglet</span></div>
      <div class="hint"><span class="key">A</span><span>Jouer</span></div>
    </div>
  </div>
</div>
