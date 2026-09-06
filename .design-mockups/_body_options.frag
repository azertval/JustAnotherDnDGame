<div class="screen" style="--accent: {{accent}}">
  <div class="grid-bg"></div>

  <div style="position: absolute; inset: 0; display: flex; flex-direction: column; padding: 56px 88px 40px;">
    <div class="title" style="font-size: 32px;">OPTIONS</div>
    <div style="height: 36px;"></div>

    <!-- Onglets : 3 declares dans le .ui + 3 ajoutes en code (remappages). -->
    <div style="display: flex; gap: 8px;">
      <div style="padding: 14px 26px; font-size: 21px; font-weight: 600; color: #12161f; background: var(--accent); box-shadow: 0 -4px 0 var(--edge), -4px 0 0 var(--edge), 4px 0 0 var(--edge), inset 0 4px 0 rgba(255,255,255,.35);">Vidéo</div>
      <div style="padding: 14px 26px; font-size: 21px; font-weight: 600; color: var(--muted); background: var(--surfaceAlt); box-shadow: 0 -4px 0 var(--edge), -4px 0 0 var(--edge), 4px 0 0 var(--edge), inset 0 4px 0 var(--ornament);">Audio</div>
      <div style="padding: 14px 26px; font-size: 21px; font-weight: 600; color: var(--muted); background: var(--surfaceAlt); box-shadow: 0 -4px 0 var(--edge), -4px 0 0 var(--edge), 4px 0 0 var(--edge), inset 0 4px 0 var(--ornament);">Général</div>
      <div style="padding: 14px 26px; font-size: 21px; font-weight: 600; color: var(--muted); background: var(--surfaceAlt); box-shadow: 0 -4px 0 var(--edge), -4px 0 0 var(--edge), 4px 0 0 var(--edge), inset 0 4px 0 var(--ornament);">Clavier</div>
      <div style="padding: 14px 26px; font-size: 21px; font-weight: 600; color: var(--muted); background: var(--surfaceAlt); box-shadow: 0 -4px 0 var(--edge), -4px 0 0 var(--edge), 4px 0 0 var(--edge), inset 0 4px 0 var(--ornament);">Éditeur</div>
      <div style="padding: 14px 26px; font-size: 21px; font-weight: 600; color: var(--muted); background: var(--surfaceAlt); box-shadow: 0 -4px 0 var(--edge), -4px 0 0 var(--edge), 4px 0 0 var(--edge), inset 0 4px 0 var(--ornament);">Manette</div>
    </div>

    <div class="frame" style="flex-grow: 1; padding: 34px 48px; display: flex; flex-direction: column; gap: 26px;">
      <!-- Ligne : case a cocher pixel, focalisee -->
      <div style="display: flex; align-items: center; gap: 32px; padding: 8px 16px; margin: -8px -16px; background: rgba(255,209,51,.10); box-shadow: inset 0 0 0 4px rgba(255,209,51,.30);">
        <div style="width: 34px; flex: none; color: var(--accent);">@CARET@</div>
        <div style="width: 420px; font-size: 25px; color: var(--accent);">Synchronisation verticale</div>
        <div style="width: 32px; height: 32px; background: var(--surfaceAlt); position: relative; box-shadow: 0 -4px 0 var(--edge), 0 4px 0 var(--edge), -4px 0 0 var(--edge), 4px 0 0 var(--edge), inset 0 4px 0 var(--ornament), inset 0 -4px 0 var(--shadow);">
          <div style="position: absolute; inset: 8px; background: var(--accent);"></div>
        </div>
        <div style="font-size: 21px; color: var(--muted);">Activée</div>
      </div>

      <!-- Ligne : case a cocher non cochee -->
      <div style="display: flex; align-items: center; gap: 32px;">
        <div style="width: 34px; flex: none;"></div>
        <div style="width: 420px; font-size: 25px;">Affichage</div>
        <div style="width: 32px; height: 32px; background: var(--surfaceAlt); box-shadow: 0 -4px 0 var(--edge), 0 4px 0 var(--edge), -4px 0 0 var(--edge), 4px 0 0 var(--edge), inset 0 4px 0 var(--ornament), inset 0 -4px 0 var(--shadow);"></div>
        <div style="font-size: 21px; color: var(--muted);">Plein écran</div>
      </div>


      <!-- Compteur d'images/s (commentaire de relecture) : remplace l'ancien selecteur de limite,
           grise et jamais branche. Ce que le joueur veut voir, c'est le chiffre, pas un plafond. -->
      <div style="display: flex; align-items: center; gap: 32px;">
        <div style="width: 34px; flex: none;"></div>
        <div style="width: 420px; font-size: 25px;">Compteur d'images/s</div>
        <div style="width: 32px; height: 32px; background: var(--surfaceAlt); position: relative; box-shadow: 0 -4px 0 var(--edge), 0 4px 0 var(--edge), -4px 0 0 var(--edge), 4px 0 0 var(--edge), inset 0 4px 0 var(--ornament), inset 0 -4px 0 var(--shadow);">
          <div style="position: absolute; inset: 8px; background: var(--accent);"></div>
        </div>
        <div style="font-size: 21px; color: var(--muted);">Afficher en haut à droite</div>
      </div>

      <!-- Ligne : glissiere pixel -->
      <div style="display: flex; align-items: center; gap: 32px;">
        <div style="width: 34px; flex: none;"></div>
        <div style="width: 420px; font-size: 25px;">Volume</div>
        <div style="width: 320px; height: 20px; background: var(--surfaceAlt); position: relative; box-shadow: 0 -4px 0 var(--edge), 0 4px 0 var(--edge), -4px 0 0 var(--edge), 4px 0 0 var(--edge);">
          <div style="position: absolute; left: 0; top: 0; bottom: 0; width: 224px; background: var(--accent);"></div>
          <div style="position: absolute; left: 208px; top: -10px; width: 24px; height: 40px; background: #f2f2ff; box-shadow: 0 -4px 0 var(--edge), 0 4px 0 var(--edge), -4px 0 0 var(--edge), 4px 0 0 var(--edge), inset 0 -6px 0 #9aa1b0;"></div>
        </div>
        <div style="font-size: 21px; color: var(--muted);">70 %</div>
      </div>

      <!-- Ligne : liste deroulante pixel -->
      <div style="display: flex; align-items: center; gap: 32px;">
        <div style="width: 34px; flex: none;"></div>
        <div style="width: 420px; font-size: 25px;">Langue</div>
        <div style="width: 240px; display: flex; align-items: center; justify-content: space-between; padding: 10px 16px; background: var(--surfaceAlt); font-size: 21px; box-shadow: 0 -4px 0 var(--edge), 0 4px 0 var(--edge), -4px 0 0 var(--edge), 4px 0 0 var(--edge), inset 0 4px 0 var(--ornament), inset 0 -4px 0 var(--shadow);">
          <span>Français</span>
          <svg viewBox="0 0 8 5" width="20" height="13" shape-rendering="crispEdges" aria-hidden="true">
            <rect x="0" y="0" width="8" height="1" fill="#8b93a7"></rect>
            <rect x="1" y="1" width="6" height="1" fill="#8b93a7"></rect>
            <rect x="2" y="2" width="4" height="1" fill="#8b93a7"></rect>
            <rect x="3" y="3" width="2" height="1" fill="#8b93a7"></rect>
          </svg>
        </div>
      </div>

      <!-- Ligne : bouton pixel -->
      <div style="display: flex; align-items: center; gap: 32px;">
        <div style="width: 34px; flex: none;"></div>
        <div style="width: 420px; font-size: 25px;">Journaux</div>
        <div style="padding: 12px 24px; font-size: 21px; font-weight: 600; color: #12161f; background: var(--accent); box-shadow: 0 -4px 0 var(--edge), 0 4px 0 var(--edge), -4px 0 0 var(--edge), 4px 0 0 var(--edge), inset 0 4px 0 rgba(255,255,255,.4), inset 0 -6px 0 #a87f00;">Enregistrer les journaux de session</div>
      </div>
    </div>

    <div style="height: 28px;"></div>
    <div style="display: flex; align-items: center; justify-content: space-between;">
      <div class="row" style="padding-left: 0;">@CARET@<span>Retour</span></div>
      <div style="display: flex; gap: 24px;">
        <div class="hint"><span class="key">←→</span><span>Onglet</span></div>
        <div class="hint"><span class="key">B</span><span>Retour</span></div>
      </div>
    </div>
  </div>
</div>
