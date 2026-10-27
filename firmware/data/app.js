/**
 * AURA // Luxury Smart Lighting Architecture - Engine v3.0
 * Unified Controller: Spatial Studio, Circadian HCL, Ambilight, SoundSync, Gradient Lab & Focus Rhythm
 */

'use strict';

// ============================================================================
// 1. STATE & GLOBAL CONFIGURATION
// ============================================================================

const PRESETS = {
  'cyberpunk': {
    name: 'Cyberpunk Night',
    masterBrightness: 85,
    contrast: 15,
    desk: { power: true, brightness: 90, color: '#ec4899', effect: 'cyberpulse', speed: 65, intensity: 80 },
    wizLeft: { power: true, brightness: 70, mode: 'rgb', color: '#1e40af', kelvin: 4000 },
    wizRight: { power: true, brightness: 70, mode: 'rgb', color: '#1e40af', kelvin: 4000 }
  },
  'warm-minimal': {
    name: 'Warm Minimalist',
    masterBrightness: 70,
    contrast: -10,
    desk: { power: true, brightness: 60, color: '#f59e0b', effect: 'breathe', speed: 40, intensity: 50 },
    wizLeft: { power: true, brightness: 75, mode: 'cct', color: '#f59e0b', kelvin: 2400 },
    wizRight: { power: true, brightness: 75, mode: 'cct', color: '#f59e0b', kelvin: 2400 }
  },
  'deep-focus': {
    name: 'Deep Focus',
    masterBrightness: 80,
    contrast: 35,
    desk: { power: true, brightness: 95, color: '#00f2fe', effect: 'solid', speed: 50, intensity: 50 },
    wizLeft: { power: true, brightness: 25, mode: 'cct', color: '#ffffff', kelvin: 4500 },
    wizRight: { power: true, brightness: 25, mode: 'cct', color: '#ffffff', kelvin: 4500 }
  },
  'sunset': {
    name: 'Sunset Horizon',
    masterBrightness: 85,
    contrast: 5,
    desk: { power: true, brightness: 85, color: '#f43f5e', effect: 'sunset-wave', speed: 55, intensity: 85 },
    wizLeft: { power: true, brightness: 80, mode: 'rgb', color: '#fb923c', kelvin: 2700 },
    wizRight: { power: true, brightness: 80, mode: 'rgb', color: '#fb923c', kelvin: 2700 }
  },
  'aurora': {
    name: 'Aurora Drift',
    masterBrightness: 75,
    contrast: 0,
    desk: { power: true, brightness: 80, color: '#10b981', effect: 'aurora', speed: 45, intensity: 70 },
    wizLeft: { power: true, brightness: 65, mode: 'rgb', color: '#8b5cf6', kelvin: 4000 },
    wizRight: { power: true, brightness: 65, mode: 'rgb', color: '#8b5cf6', kelvin: 4000 }
  },
  'cinema': {
    name: 'Midnight Cinema',
    masterBrightness: 30,
    contrast: 20,
    desk: { power: true, brightness: 35, color: '#f59e0b', effect: 'solid', speed: 20, intensity: 30 },
    wizLeft: { power: true, brightness: 15, mode: 'cct', color: '#ff8a12', kelvin: 2000 },
    wizRight: { power: true, brightness: 15, mode: 'cct', color: '#ff8a12', kelvin: 2000 }
  }
};

const appState = {
  isMasterOn: true,
  masterBrightness: 80,
  contrastRatio: 0,
  syncCorners: true,
  activePreset: 'cyberpunk',
  activeTab: 'studio',
  transitionDuration: 800,
  useGammaCorrection: true,
  
  // Circadian HCL Engine
  circadian: {
    autoEnabled: false,
    simulatedMinutes: 870, // 14:30
    currentKelvin: 5500,
    currentLux: 90
  },

  // Ambilight
  ambilight: {
    active: false,
    smoothing: 0.70,
    saturationBoost: 1.4,
    blackFloor: 15,
    stream: null,
    rafId: null
  },

  // SoundSync
  soundsync: {
    active: false,
    mode: 'pulse',
    sensitivity: 75,
    audioCtx: null,
    analyser: null,
    micStream: null,
    rafId: null
  },

  // Gradient Lab
  gradient: {
    stops: ['#00f2fe', '#ec4899', '#8a2be2'],
    speed: 50
  },

  // Focus Rhythm (Pomodoro)
  pomodoro: {
    active: false,
    phase: 'focus', // 'focus' or 'break'
    secondsRemaining: 25 * 60,
    timerInterval: null
  },

  network: {
    hubIp: '192.168.1.99',
    wizLeftIp: localStorage.getItem('aura_wiz_left_ip') || '192.168.1.101',
    wizRightIp: localStorage.getItem('aura_wiz_right_ip') || '192.168.1.102'
  },

  devices: {
    desk: {
      power: true,
      brightness: 85,
      color: '#ec4899',
      effect: 'cyberpulse',
      speed: 60,
      intensity: 75
    },
    wizLeft: {
      power: true,
      brightness: 75,
      mode: 'rgb',
      color: '#1e40af',
      kelvin: 2700
    },
    wizRight: {
      power: true,
      brightness: 75,
      mode: 'rgb',
      color: '#1e40af',
      kelvin: 2700
    }
  }
};

// ============================================================================
// 2. DOM CACHE
// ============================================================================

const DOM = {
  // Tabs & Nav
  suiteTabs: document.querySelectorAll('.suite-tab'),
  tabViews: document.querySelectorAll('.tab-view'),
  selectTransitionSpeed: document.getElementById('select-transition-speed'),

  // Header & Status
  connectionBadge: document.getElementById('connection-badge'),
  connectionText: document.getElementById('connection-text'),
  btnMasterPower: document.getElementById('btn-master-power'),

  // Master Sliders
  sliderMasterBrightness: document.getElementById('slider-master-brightness'),
  valMasterBrightness: document.getElementById('val-master-brightness'),
  sliderMasterContrast: document.getElementById('slider-master-contrast'),
  valMasterContrast: document.getElementById('val-master-contrast'),

  // Spatial Visualizer Elements
  glowCornerLeft: document.getElementById('glow-corner-left'),
  glowCornerRight: document.getElementById('glow-corner-right'),
  glowDeskBias: document.getElementById('glow-desk-bias'),
  virtualLedStrip: document.getElementById('virtual-led-strip'),
  badgeWizLeftVal: document.getElementById('badge-wiz-left-val'),
  badgeWizRightVal: document.getElementById('badge-wiz-right-val'),
  badgeDeskVal: document.getElementById('badge-desk-val'),
  telemetryLux: document.getElementById('telemetry-lux'),
  telemetryContrast: document.getElementById('telemetry-contrast'),

  // Device Cards
  cardDesk: document.getElementById('card-desk-strip'),
  cardWizLeft: document.getElementById('card-wiz-left'),
  cardWizRight: document.getElementById('card-wiz-right'),

  // Desk Controls
  toggleDeskPower: document.getElementById('toggle-desk-power'),
  sliderDeskBrightness: document.getElementById('slider-desk-brightness'),
  valDeskBrightness: document.getElementById('val-desk-brightness'),
  pickerDeskColor: document.getElementById('picker-desk-color'),
  valDeskHex: document.getElementById('val-desk-hex'),
  deskSwatches: document.getElementById('desk-swatches'),
  effectsSelector: document.getElementById('effects-selector'),
  sliderDeskSpeed: document.getElementById('slider-desk-speed'),
  valDeskSpeed: document.getElementById('val-desk-speed'),
  sliderDeskIntensity: document.getElementById('slider-desk-intensity'),
  valDeskIntensity: document.getElementById('val-desk-intensity'),

  // WiZ Left Controls
  toggleWizLeftPower: document.getElementById('toggle-wiz-left-power'),
  sliderWizLeftBrightness: document.getElementById('slider-wiz-left-brightness'),
  valWizLeftBrightness: document.getElementById('val-wiz-left-brightness'),
  pickerWizLeftColor: document.getElementById('picker-wiz-left-color'),
  valWizLeftHex: document.getElementById('val-wiz-left-hex'),
  sliderWizLeftKelvin: document.getElementById('slider-wiz-left-kelvin'),
  valWizLeftKelvin: document.getElementById('val-wiz-left-kelvin'),
  viewWizLeftRgb: document.getElementById('view-wiz-left-rgb'),
  viewWizLeftCct: document.getElementById('view-wiz-left-cct'),

  // WiZ Right Controls
  toggleWizRightPower: document.getElementById('toggle-wiz-right-power'),
  sliderWizRightBrightness: document.getElementById('slider-wiz-right-brightness'),
  valWizRightBrightness: document.getElementById('val-wiz-right-brightness'),
  pickerWizRightColor: document.getElementById('picker-wiz-right-color'),
  valWizRightHex: document.getElementById('val-wiz-right-hex'),
  sliderWizRightKelvin: document.getElementById('slider-wiz-right-kelvin'),
  valWizRightKelvin: document.getElementById('val-wiz-right-kelvin'),
  viewWizRightRgb: document.getElementById('view-wiz-right-rgb'),
  viewWizRightCct: document.getElementById('view-wiz-right-cct'),
  btnSyncCorners: document.getElementById('btn-sync-corners'),

  // Circadian
  toggleCircadianAuto: document.getElementById('toggle-circadian-auto'),
  circadianClockVal: document.getElementById('circadian-clock-val'),
  circadianPhaseVal: document.getElementById('circadian-phase-val'),
  sliderCircadianTime: document.getElementById('slider-circadian-time'),
  solarSunMarker: document.getElementById('solar-sun-marker'),
  btnCircadianNow: document.getElementById('btn-circadian-now'),

  // Ambilight
  btnToggleAmbilight: document.getElementById('btn-toggle-ambilight'),
  ambilightBtnLabel: document.getElementById('ambilight-btn-label'),
  ambilightVideo: document.getElementById('ambilight-video'),
  ambilightCanvas: document.getElementById('ambilight-canvas'),
  ambilightPlaceholder: document.getElementById('ambilight-placeholder'),
  zoneLeft: document.getElementById('zone-preview-left'),
  zoneCenter: document.getElementById('zone-preview-center'),
  zoneRight: document.getElementById('zone-preview-right'),
  hexZoneLeft: document.getElementById('hex-zone-left'),
  hexZoneCenter: document.getElementById('hex-zone-center'),
  hexZoneRight: document.getElementById('hex-zone-right'),
  sliderAmbiSmoothing: document.getElementById('slider-ambi-smoothing'),
  valAmbiSmoothing: document.getElementById('val-ambi-smoothing'),
  sliderAmbiBoost: document.getElementById('slider-ambi-boost'),
  valAmbiBoost: document.getElementById('val-ambi-boost'),
  sliderAmbiBlackfloor: document.getElementById('slider-ambi-blackfloor'),
  valAmbiBlackfloor: document.getElementById('val-ambi-blackfloor'),

  // SoundSync
  btnToggleSoundSync: document.getElementById('btn-toggle-soundsync'),
  soundsyncBtnLabel: document.getElementById('soundsync-btn-label'),
  soundsyncCanvas: document.getElementById('soundsync-canvas'),
  sliderAudioSens: document.getElementById('slider-audio-sens'),
  valAudioSens: document.getElementById('val-audio-sens'),
  audioModesSelector: document.getElementById('audio-modes-selector'),

  // Gradient Lab
  gradientPreviewBar: document.getElementById('gradient-preview-bar'),
  gradColor1: document.getElementById('grad-color-1'),
  gradColor2: document.getElementById('grad-color-2'),
  gradColor3: document.getElementById('grad-color-3'),
  btnApplyGradient: document.getElementById('btn-apply-gradient'),

  // Focus Rhythm (Pomodoro)
  btnTogglePomodoro: document.getElementById('btn-toggle-pomodoro'),
  pomoTimeLeft: document.getElementById('pomo-time-left'),
  pomoStatusLabel: document.getElementById('pomo-status-label'),

  // Settings Modal & Discovery
  btnOpenSettings: document.getElementById('btn-open-settings'),
  btnCloseSettings: document.getElementById('btn-close-settings'),
  modalSettings: document.getElementById('modal-settings'),
  btnScanWiz: document.getElementById('btn-scan-wiz'),
  discoveryResults: document.getElementById('discovery-results'),
  inputWizLeftIp: document.getElementById('input-wiz-left-ip'),
  inputWizRightIp: document.getElementById('input-wiz-right-ip'),
  btnSaveSettings: document.getElementById('btn-save-settings'),
  ipDisplayWizLeft: document.getElementById('ip-display-wiz-left'),
  ipDisplayWizRight: document.getElementById('ip-display-wiz-right'),
  toggleGamma: document.getElementById('toggle-gamma'),

  // Toast
  toastContainer: document.getElementById('toast-container')
};

// ============================================================================
// 3. WEBSOCKET & COMMUNICATIONS
// ============================================================================

let socket = null;
let isSimulatedMode = false;
let socketReconnectTimer = null;

function initCommunications() {
  const isEsp32Host = window.location.protocol.startsWith('http') &&
    window.location.hostname !== 'localhost' &&
    window.location.hostname !== '127.0.0.1';

  if (!isEsp32Host) {
    activateSimulatedMode();
    return;
  }

  connectWebSocket();
}

function activateSimulatedMode() {
  isSimulatedMode = true;
  DOM.connectionBadge.className = 'status-chip simulated';
  DOM.connectionText.textContent = 'SIMULATOR (OFFLINE)';
  showToast('Running in local simulated mode with live spatial preview.');
}

function connectWebSocket() {
  const wsUrl = `ws://${window.location.host}/ws`;
  try {
    socket = new WebSocket(wsUrl);

    socket.onopen = () => {
      DOM.connectionBadge.className = 'status-chip connected';
      DOM.connectionText.textContent = 'ESP32 ONLINE';
      showToast('Connected to ESP32 Smart Hub.');
      clearTimeout(socketReconnectTimer);
      socket.send(JSON.stringify({ type: 'getState' }));
    };

    socket.onmessage = (event) => {
      try {
        const message = JSON.parse(event.data);
        handleIncomingMessage(message);
      } catch (e) {
        console.error('Failed to parse WS message:', e);
      }
    };

    socket.onclose = () => {
      DOM.connectionBadge.className = 'status-chip simulated';
      DOM.connectionText.textContent = 'RECONNECTING...';
      socketReconnectTimer = setTimeout(connectWebSocket, 3000);
    };

    socket.onerror = () => {
      if (socket) socket.close();
    };
  } catch (err) {
    activateSimulatedMode();
  }
}

function dispatchStateChange(payload) {
  if (socket && socket.readyState === WebSocket.OPEN) {
    socket.send(JSON.stringify({
      type: 'update',
      payload: payload
    }));
  }
}

function handleIncomingMessage(msg) {
  if (msg.type === 'state') {
    Object.assign(appState, msg.payload);
    renderAll();
  } else if (msg.type === 'wizDiscovery') {
    renderDiscoveredDevices(msg.devices);
  }
}

// ============================================================================
// 4. RENDERING & UI SYNC ENGINE
// ============================================================================

function renderAll() {
  renderVisualizer();
  syncAllInputs();
  renderScenesDeck();
  renderDeviceCards();
  updateCircadianSolarDisplay();
  updateGradientPreviewBar();
}

function renderVisualizer() {
  const root = document.documentElement;

  // Master power & brightness scaling
  let masterFactor = appState.isMasterOn ? (appState.masterBrightness / 100) : 0;
  if (appState.useGammaCorrection && masterFactor > 0) {
    masterFactor = Math.pow(masterFactor, 1.8); // Perceptual human eye curve
  }

  // Contrast ratio
  const contrastFactor = appState.contrastRatio / 100;
  const deskFactor = Math.min(1.5, Math.max(0, 1 + contrastFactor));
  const cornerFactor = Math.min(1.5, Math.max(0, 1 - contrastFactor));

  const effBrightDesk = (appState.devices.desk.power && appState.isMasterOn)
    ? Math.min(1, (appState.devices.desk.brightness / 100) * masterFactor * deskFactor)
    : 0;

  const effBrightWizLeft = (appState.devices.wizLeft.power && appState.isMasterOn)
    ? Math.min(1, (appState.devices.wizLeft.brightness / 100) * masterFactor * cornerFactor)
    : 0;

  const effBrightWizRight = (appState.devices.wizRight.power && appState.isMasterOn)
    ? Math.min(1, (appState.devices.wizRight.brightness / 100) * masterFactor * cornerFactor)
    : 0;

  const deskColor = appState.devices.desk.color;
  const wizLeftColor = appState.devices.wizLeft.mode === 'rgb'
    ? appState.devices.wizLeft.color
    : kelvinToRgbHex(appState.devices.wizLeft.kelvin);
  const wizRightColor = appState.devices.wizRight.mode === 'rgb'
    ? appState.devices.wizRight.color
    : kelvinToRgbHex(appState.devices.wizRight.kelvin);

  root.style.setProperty('--color-desk', deskColor);
  root.style.setProperty('--color-wiz-left', wizLeftColor);
  root.style.setProperty('--color-wiz-right', wizRightColor);
  root.style.setProperty('--bright-desk', effBrightDesk.toFixed(2));
  root.style.setProperty('--bright-wiz-left', effBrightWizLeft.toFixed(2));
  root.style.setProperty('--bright-wiz-right', effBrightWizRight.toFixed(2));
  root.style.setProperty('--lighting-fade-speed', `${appState.transitionDuration / 1000}s`);

  DOM.badgeDeskVal.textContent = appState.devices.desk.power && appState.isMasterOn
    ? `${Math.round(effBrightDesk * 100)}% • ${capitalize(appState.devices.desk.effect)}`
    : 'OFF';

  DOM.badgeWizLeftVal.textContent = appState.devices.wizLeft.power && appState.isMasterOn
    ? `${Math.round(effBrightWizLeft * 100)}%`
    : 'OFF';

  DOM.badgeWizRightVal.textContent = appState.devices.wizRight.power && appState.isMasterOn
    ? `${Math.round(effBrightWizRight * 100)}%`
    : 'OFF';

  const totalLux = Math.round(((effBrightDesk + effBrightWizLeft + effBrightWizRight) / 3) * 100);
  DOM.telemetryLux.innerHTML = `TOTAL LUMENS: <strong>${totalLux}%</strong>`;

  const ratioVal = effBrightWizLeft > 0 ? (effBrightDesk / Math.max(0.1, effBrightWizLeft)).toFixed(1) : '∞';
  DOM.telemetryContrast.innerHTML = `CONTRAST RATIO: <strong>${ratioVal}:1</strong>`;
}

function syncAllInputs() {
  DOM.btnMasterPower.classList.toggle('active', appState.isMasterOn);
  DOM.sliderMasterBrightness.value = appState.masterBrightness;
  DOM.valMasterBrightness.textContent = `${appState.masterBrightness}%`;
  syncRangeProgress(DOM.sliderMasterBrightness);

  DOM.sliderMasterContrast.value = appState.contrastRatio;
  const cVal = appState.contrastRatio;
  DOM.valMasterContrast.textContent = cVal === 0 ? 'Balanced (1:1)' : (cVal > 0 ? `+${cVal}% Bias` : `${cVal}% Ambient`);
  syncRangeProgress(DOM.sliderMasterContrast);

  // Desk inputs
  DOM.toggleDeskPower.checked = appState.devices.desk.power;
  DOM.sliderDeskBrightness.value = appState.devices.desk.brightness;
  DOM.valDeskBrightness.textContent = `${appState.devices.desk.brightness}%`;
  syncRangeProgress(DOM.sliderDeskBrightness);

  DOM.pickerDeskColor.value = appState.devices.desk.color;
  DOM.valDeskHex.textContent = appState.devices.desk.color.toUpperCase();

  DOM.sliderDeskSpeed.value = appState.devices.desk.speed;
  DOM.valDeskSpeed.textContent = `${appState.devices.desk.speed}%`;
  syncRangeProgress(DOM.sliderDeskSpeed);

  DOM.sliderDeskIntensity.value = appState.devices.desk.intensity;
  DOM.valDeskIntensity.textContent = `${appState.devices.desk.intensity}%`;
  syncRangeProgress(DOM.sliderDeskIntensity);

  document.querySelectorAll('#effects-selector .effect-chip').forEach(chip => {
    chip.classList.toggle('active', chip.dataset.effect === appState.devices.desk.effect);
  });

  // WiZ Left inputs
  DOM.toggleWizLeftPower.checked = appState.devices.wizLeft.power;
  DOM.sliderWizLeftBrightness.value = appState.devices.wizLeft.brightness;
  DOM.valWizLeftBrightness.textContent = `${appState.devices.wizLeft.brightness}%`;
  syncRangeProgress(DOM.sliderWizLeftBrightness);

  DOM.pickerWizLeftColor.value = appState.devices.wizLeft.color;
  DOM.valWizLeftHex.textContent = appState.devices.wizLeft.color.toUpperCase();

  DOM.sliderWizLeftKelvin.value = appState.devices.wizLeft.kelvin;
  DOM.valWizLeftKelvin.textContent = `${appState.devices.wizLeft.kelvin}K`;
  syncRangeProgress(DOM.sliderWizLeftKelvin);

  syncModeTabs('wiz-left', appState.devices.wizLeft.mode);

  // WiZ Right inputs
  DOM.toggleWizRightPower.checked = appState.devices.wizRight.power;
  DOM.sliderWizRightBrightness.value = appState.devices.wizRight.brightness;
  DOM.valWizRightBrightness.textContent = `${appState.devices.wizRight.brightness}%`;
  syncRangeProgress(DOM.sliderWizRightBrightness);

  DOM.pickerWizRightColor.value = appState.devices.wizRight.color;
  DOM.valWizRightHex.textContent = appState.devices.wizRight.color.toUpperCase();

  DOM.sliderWizRightKelvin.value = appState.devices.wizRight.kelvin;
  DOM.valWizRightKelvin.textContent = `${appState.devices.wizRight.kelvin}K`;
  syncRangeProgress(DOM.sliderWizRightKelvin);

  syncModeTabs('wiz-right', appState.devices.wizRight.mode);

  DOM.btnSyncCorners.classList.toggle('active', appState.syncCorners);
  DOM.toggleCircadianAuto.checked = appState.circadian.autoEnabled;
}

function renderDeviceCards() {
  DOM.cardDesk.classList.toggle('powered-off', !appState.devices.desk.power || !appState.isMasterOn);
  DOM.cardWizLeft.classList.toggle('powered-off', !appState.devices.wizLeft.power || !appState.isMasterOn);
  DOM.cardWizRight.classList.toggle('powered-off', !appState.devices.wizRight.power || !appState.isMasterOn);
}

function renderScenesDeck() {
  document.querySelectorAll('.scene-card').forEach(card => {
    card.classList.toggle('active', card.dataset.preset === appState.activePreset);
  });
}

function syncModeTabs(device, mode) {
  const tabs = document.querySelectorAll(`.mode-tab[data-device="${device}"]`);
  tabs.forEach(tab => tab.classList.toggle('active', tab.dataset.mode === mode));

  const isLeft = device === 'wiz-left';
  const rgbView = isLeft ? DOM.viewWizLeftRgb : DOM.viewWizRightRgb;
  const cctView = isLeft ? DOM.viewWizLeftCct : DOM.viewWizRightCct;

  if (mode === 'rgb') {
    rgbView.classList.remove('hidden');
    cctView.classList.add('hidden');
  } else {
    rgbView.classList.add('hidden');
    cctView.classList.remove('hidden');
  }
}

function syncRangeProgress(slider) {
  if (!slider) return;
  const min = parseFloat(slider.min) || 0;
  const max = parseFloat(slider.max) || 100;
  const val = parseFloat(slider.value) || 0;
  slider.style.setProperty('--attr-min', min);
  slider.style.setProperty('--attr-max', max);
  slider.style.setProperty('--control-value', val);
}

// ============================================================================
// 5. FEATURE SUITE 1: CIRCADIAN RHYTHM (Natural Human Centric Lighting)
// ============================================================================

function calculateCircadianSolar(minutesOfDay) {
  // Normalize 0-1440 to hours
  const hours = minutesOfDay / 60;
  let targetKelvin = 2400;
  let targetLux = 30;
  let phaseName = 'Midnight Sleep';

  if (hours >= 5 && hours < 8) {
    // Dawn: 2200K -> 3500K
    const progress = (hours - 5) / 3;
    targetKelvin = Math.round(2200 + (3500 - 2200) * progress);
    targetLux = Math.round(25 + (70 - 25) * progress);
    phaseName = 'Dawn Awakening';
  } else if (hours >= 8 && hours < 16) {
    // Daylight Focus: 3500K -> 6500K -> 5000K (Bell curve centered around noon)
    const noonDist = Math.abs(hours - 12);
    targetKelvin = Math.round(6500 - (noonDist * 300));
    targetLux = Math.round(100 - (noonDist * 5));
    phaseName = 'Zenith Daylight Focus';
  } else if (hours >= 16 && hours < 19) {
    // Golden Hour: 5000K -> 2700K
    const progress = (hours - 16) / 3;
    targetKelvin = Math.round(5000 - (5000 - 2700) * progress);
    targetLux = Math.round(90 - (90 - 55) * progress);
    phaseName = 'Golden Hour Sunset';
  } else if (hours >= 19 && hours < 22) {
    // Evening Chill: 2700K -> 2000K
    const progress = (hours - 19) / 3;
    targetKelvin = Math.round(2700 - (2700 - 2000) * progress);
    targetLux = Math.round(55 - (55 - 25) * progress);
    phaseName = 'Evening Relaxation';
  } else {
    // Night: Melatonin rest
    targetKelvin = 1800;
    targetLux = 15;
    phaseName = 'Deep Rest (Melatonin Guard)';
  }

  return { targetKelvin, targetLux, phaseName };
}

function updateCircadianSolarDisplay() {
  const mins = appState.circadian.simulatedMinutes;
  const h = Math.floor(mins / 60).toString().padStart(2, '0');
  const m = (mins % 60).toString().padStart(2, '0');
  DOM.circadianClockVal.textContent = `${h}:${m}`;

  const { targetKelvin, targetLux, phaseName } = calculateCircadianSolar(mins);
  appState.circadian.currentKelvin = targetKelvin;
  appState.circadian.currentLux = targetLux;
  DOM.circadianPhaseVal.textContent = `${phaseName} (${targetKelvin}K • ${targetLux}% Lux)`;

  // Move SVG Sun Marker along bell curve:
  // X: 0 to 1000
  const x = (mins / 1440) * 1000;
  // Y: Bell curve peaking at midday (X = 500, Y = 40; Horizon at Y = 200)
  const normX = Math.abs(x - 500) / 500;
  const y = 40 + Math.pow(normX, 2) * 160;

  DOM.solarSunMarker.setAttribute('cx', x.toFixed(1));
  DOM.solarSunMarker.setAttribute('cy', y.toFixed(1));
}

function applyCircadianToRoom() {
  const { targetKelvin, targetLux } = calculateCircadianSolar(appState.circadian.simulatedMinutes);
  appState.masterBrightness = targetLux;
  appState.devices.wizLeft.mode = 'cct';
  appState.devices.wizLeft.kelvin = targetKelvin;
  appState.devices.wizRight.mode = 'cct';
  appState.devices.wizRight.kelvin = targetKelvin;

  // Set desk strip to complementary soft white
  appState.devices.desk.color = kelvinToRgbHex(targetKelvin);
  appState.devices.desk.effect = 'breathe';
  renderAll();
  dispatchStateChange({ state: appState });
}

// ============================================================================
// 6. FEATURE SUITE 2: AMBILIGHT REAL-TIME SCREEN SYNC
// ============================================================================

async function toggleAmbilight() {
  if (appState.ambilight.active) {
    stopAmbilight();
  } else {
    try {
      const stream = await navigator.mediaDevices.getDisplayMedia({
        video: { frameRate: 30, cursor: 'never' }
      });
      startAmbilight(stream);
    } catch (err) {
      showToast('Screen capture permission was not granted.');
    }
  }
}

function startAmbilight(stream) {
  appState.ambilight.active = true;
  appState.ambilight.stream = stream;
  DOM.ambilightVideo.srcObject = stream;
  DOM.ambilightPlaceholder.classList.add('hidden');
  DOM.ambilightBtnLabel.textContent = 'Stop Screen Sync';
  DOM.btnToggleAmbilight.classList.add('active');

  stream.getVideoTracks()[0].onended = stopAmbilight;

  const canvas = DOM.ambilightCanvas;
  const ctx = canvas.getContext('2d', { willReadFrequently: true });
  canvas.width = 64;
  canvas.height = 36;

  let smoothedLeft = [30, 64, 175];
  let smoothedCenter = [236, 72, 153];
  let smoothedRight = [30, 64, 175];

  function loop() {
    if (!appState.ambilight.active) return;
    if (DOM.ambilightVideo.readyState >= 2) {
      ctx.drawImage(DOM.ambilightVideo, 0, 0, canvas.width, canvas.height);

      // Sample Left Region (0 to 18px width)
      const leftPixel = sampleRegionColor(ctx, 0, 0, 18, canvas.height);
      // Sample Center/Bottom Region (18 to 46px width)
      const centerPixel = sampleRegionColor(ctx, 18, 12, 28, canvas.height - 12);
      // Sample Right Region (46 to 64px width)
      const rightPixel = sampleRegionColor(ctx, 46, 0, 18, canvas.height);

      // Apply Exponential Moving Average (EMA) smoothing
      const alpha = 1.0 - (appState.ambilight.smoothing);
      smoothedLeft = lerpRGB(smoothedLeft, leftPixel, alpha);
      smoothedCenter = lerpRGB(smoothedCenter, centerPixel, alpha);
      smoothedRight = lerpRGB(smoothedRight, rightPixel, alpha);

      // Format Hex
      const hexLeft = rgbToHex(smoothedLeft[0], smoothedLeft[1], smoothedLeft[2]);
      const hexCenter = rgbToHex(smoothedCenter[0], smoothedCenter[1], smoothedCenter[2]);
      const hexRight = rgbToHex(smoothedRight[0], smoothedRight[1], smoothedRight[2]);

      // Update UI Previews
      DOM.hexZoneLeft.textContent = hexLeft;
      DOM.hexZoneCenter.textContent = hexCenter;
      DOM.hexZoneRight.textContent = hexRight;
      DOM.zoneLeft.style.borderColor = hexLeft;
      DOM.zoneCenter.style.borderColor = hexCenter;
      DOM.zoneRight.style.borderColor = hexRight;

      // Apply to Room Devices
      appState.devices.wizLeft.mode = 'rgb';
      appState.devices.wizLeft.color = hexLeft;
      appState.devices.desk.color = hexCenter;
      appState.devices.desk.effect = 'solid';
      appState.devices.wizRight.mode = 'rgb';
      appState.devices.wizRight.color = hexRight;

      renderVisualizer();
      dispatchStateChange({
        desk: { color: hexCenter, effect: 'solid' },
        wizLeft: { color: hexLeft, mode: 'rgb' },
        wizRight: { color: hexRight, mode: 'rgb' }
      });
    }
    appState.ambilight.rafId = requestAnimationFrame(loop);
  }

  appState.ambilight.rafId = requestAnimationFrame(loop);
  showToast('Ambilight Screen Sync started (30 FPS)');
}

function stopAmbilight() {
  appState.ambilight.active = false;
  if (appState.ambilight.rafId) cancelAnimationFrame(appState.ambilight.rafId);
  if (appState.ambilight.stream) {
    appState.ambilight.stream.getTracks().forEach(t => t.stop());
    appState.ambilight.stream = null;
  }
  DOM.ambilightVideo.srcObject = null;
  DOM.ambilightPlaceholder.classList.remove('hidden');
  DOM.ambilightBtnLabel.textContent = 'Start Screen Sync';
  DOM.btnToggleAmbilight.classList.remove('active');
  showToast('Ambilight Screen Sync stopped.');
}

function sampleRegionColor(ctx, x, y, w, h) {
  const imgData = ctx.getImageData(x, y, w, h);
  const data = imgData.data;
  let r = 0, g = 0, b = 0;
  const count = data.length / 4;

  for (let i = 0; i < data.length; i += 4) {
    r += data[i];
    g += data[i + 1];
    b += data[i + 2];
  }

  r = Math.round(r / count);
  g = Math.round(g / count);
  b = Math.round(b / count);

  // Saturation Boost
  const boost = appState.ambilight.saturationBoost;
  const avg = (r + g + b) / 3;
  r = Math.min(255, Math.max(0, Math.round(avg + (r - avg) * boost)));
  g = Math.min(255, Math.max(0, Math.round(avg + (g - avg) * boost)));
  b = Math.min(255, Math.max(0, Math.round(avg + (b - avg) * boost)));

  return [r, g, b];
}

function lerpRGB(prev, target, alpha) {
  return [
    Math.round(prev[0] + (target[0] - prev[0]) * alpha),
    Math.round(prev[1] + (target[1] - prev[1]) * alpha),
    Math.round(prev[2] + (target[2] - prev[2]) * alpha)
  ];
}

// ============================================================================
// 7. FEATURE SUITE 3: SOUNDSYNC AUDIO REACTIVE (Web Audio FFT)
// ============================================================================

async function toggleSoundSync() {
  if (appState.soundsync.active) {
    stopSoundSync();
  } else {
    try {
      const stream = await navigator.mediaDevices.getUserMedia({ audio: true, video: false });
      startSoundSync(stream);
    } catch (err) {
      showToast('Microphone access was denied or unavailable.');
    }
  }
}

function startSoundSync(stream) {
  appState.soundsync.active = true;
  appState.soundsync.micStream = stream;
  DOM.soundsyncBtnLabel.textContent = 'Deactivate Audio Reactive';
  DOM.btnToggleSoundSync.classList.add('active');

  const AudioCtx = window.AudioContext || window.webkitAudioContext;
  const audioCtx = new AudioCtx();
  const analyser = audioCtx.createAnalyser();
  analyser.fftSize = 128;
  analyser.smoothingTimeConstant = 0.8;

  const source = audioCtx.createMediaStreamSource(stream);
  source.connect(analyser);

  appState.soundsync.audioCtx = audioCtx;
  appState.soundsync.analyser = analyser;

  const canvas = DOM.soundsyncCanvas;
  const ctx = canvas.getContext('2d');
  const bufferLength = analyser.frequencyBinCount;
  const dataArray = new Uint8Array(bufferLength);

  function draw() {
    if (!appState.soundsync.active) return;
    analyser.getByteFrequencyData(dataArray);

    ctx.fillStyle = '#090c14';
    ctx.fillRect(0, 0, canvas.width, canvas.height);

    const barWidth = (canvas.width / bufferLength) * 2;
    let x = 0;

    // Split into bands: Sub-bass (0-4), Mids (5-18), Highs (19+)
    let subBassSum = 0;
    for (let i = 0; i < 4; i++) subBassSum += dataArray[i];
    const avgBass = subBassSum / 4;

    let midsSum = 0;
    for (let i = 5; i < 18; i++) midsSum += dataArray[i];
    const avgMids = midsSum / 13;

    for (let i = 0; i < bufferLength; i++) {
      const barHeight = (dataArray[i] / 255) * canvas.height;
      const hue = (i / bufferLength) * 280 + 160;
      ctx.fillStyle = `hsl(${hue}, 100%, 55%)`;
      ctx.fillRect(x, canvas.height - barHeight, barWidth - 2, barHeight);
      x += barWidth;
    }

    // Audio reactive pulse mapping
    const sens = appState.soundsync.sensitivity / 100;
    if (avgBass * sens > 75) {
      const bassFactor = Math.min(100, Math.round((avgBass / 255) * 100));
      appState.devices.wizLeft.brightness = bassFactor;
      appState.devices.wizRight.brightness = bassFactor;
      renderVisualizer();
    }

    if (avgMids * sens > 60) {
      appState.devices.desk.brightness = Math.min(100, Math.round((avgMids / 255) * 100));
      renderVisualizer();
    }

    appState.soundsync.rafId = requestAnimationFrame(draw);
  }

  appState.soundsync.rafId = requestAnimationFrame(draw);
  showToast('SoundSync active: Listening to audio stream');
}

function stopSoundSync() {
  appState.soundsync.active = false;
  if (appState.soundsync.rafId) cancelAnimationFrame(appState.soundsync.rafId);
  if (appState.soundsync.micStream) {
    appState.soundsync.micStream.getTracks().forEach(t => t.stop());
    appState.soundsync.micStream = null;
  }
  if (appState.soundsync.audioCtx) {
    appState.soundsync.audioCtx.close();
    appState.soundsync.audioCtx = null;
  }
  DOM.soundsyncBtnLabel.textContent = 'Activate Microphone / Audio';
  DOM.btnToggleSoundSync.classList.remove('active');
  showToast('SoundSync deactivated.');
}

// ============================================================================
// 8. FEATURE SUITE 4: GRADIENT LAB (Multi-Stop WS2812B Palette Designer)
// ============================================================================

function updateGradientPreviewBar() {
  const c1 = DOM.gradColor1.value;
  const c2 = DOM.gradColor2.value;
  const c3 = DOM.gradColor3.value;
  DOM.gradientPreviewBar.style.background = `linear-gradient(90deg, ${c1} 0%, ${c2} 50%, ${c3} 100%)`;
  appState.gradient.stops = [c1, c2, c3];
}

function applyCustomGradientToDesk() {
  const [c1, c2, c3] = appState.gradient.stops;
  appState.devices.desk.color = c2; // Center dominant
  appState.devices.desk.effect = 'sunset-wave';
  renderAll();
  showToast('Custom multi-stop gradient streaming to WS2812B desk strip.');
  dispatchStateChange({
    desk: {
      color: c2,
      effect: 'sunset-wave',
      gradientStops: [c1, c2, c3]
    }
  });
}

// ============================================================================
// 9. FEATURE SUITE 5: FOCUS & REST RHYTHM (Pomodoro Lighting Engine)
// ============================================================================

function togglePomodoro() {
  if (appState.pomodoro.active) {
    clearInterval(appState.pomodoro.timerInterval);
    appState.pomodoro.active = false;
    DOM.btnTogglePomodoro.textContent = 'Start 25m Focus Sprint';
    DOM.btnTogglePomodoro.classList.remove('active');
    DOM.pomoStatusLabel.textContent = 'PAUSED';
    showToast('Focus Sprint paused.');
  } else {
    appState.pomodoro.active = true;
    DOM.btnTogglePomodoro.textContent = 'Pause Focus Sprint';
    DOM.btnTogglePomodoro.classList.add('active');
    startPomodoroPhase('focus');

    appState.pomodoro.timerInterval = setInterval(() => {
      appState.pomodoro.secondsRemaining--;
      if (appState.pomodoro.secondsRemaining <= 0) {
        if (appState.pomodoro.phase === 'focus') {
          startPomodoroPhase('break');
        } else {
          startPomodoroPhase('focus');
        }
      }
      updatePomodoroDisplay();
    }, 1000);
  }
}

function startPomodoroPhase(phase) {
  appState.pomodoro.phase = phase;
  if (phase === 'focus') {
    appState.pomodoro.secondsRemaining = 25 * 60;
    DOM.pomoStatusLabel.textContent = 'FOCUS PHASE ACTIVE (4500K CRISP BIAS)';
    // Apply Focus Lighting Profile
    appState.devices.desk.color = '#00f2fe';
    appState.devices.desk.brightness = 95;
    appState.devices.desk.effect = 'solid';
    appState.devices.wizLeft.brightness = 15;
    appState.devices.wizLeft.mode = 'cct';
    appState.devices.wizLeft.kelvin = 4500;
    appState.devices.wizRight.brightness = 15;
    appState.devices.wizRight.mode = 'cct';
    appState.devices.wizRight.kelvin = 4500;
    showToast('Entering 25m Focus Phase: Anti-distraction bias lighting engaged.');
  } else {
    appState.pomodoro.secondsRemaining = 5 * 60;
    DOM.pomoStatusLabel.textContent = 'REST INTERVAL (2400K AMBER BLOOM)';
    // Apply Break Relax Lighting Profile
    appState.devices.desk.color = '#f59e0b';
    appState.devices.desk.brightness = 60;
    appState.devices.desk.effect = 'breathe';
    appState.devices.wizLeft.brightness = 75;
    appState.devices.wizLeft.mode = 'cct';
    appState.devices.wizLeft.kelvin = 2400;
    appState.devices.wizRight.brightness = 75;
    appState.devices.wizRight.mode = 'cct';
    appState.devices.wizRight.kelvin = 2400;
    showToast('Time for a 5m break! Ambient relaxation bloom activated.');
  }
  renderAll();
  dispatchStateChange({ state: appState });
}

function updatePomodoroDisplay() {
  const m = Math.floor(appState.pomodoro.secondsRemaining / 60).toString().padStart(2, '0');
  const s = (appState.pomodoro.secondsRemaining % 60).toString().padStart(2, '0');
  DOM.pomoTimeLeft.textContent = `${m}:${s}`;
}

// ============================================================================
// 10. EVENT BINDING & INTERACTIVITY
// ============================================================================

function bindEvents() {
  // Navigation Tabs
  DOM.suiteTabs.forEach(tab => {
    tab.addEventListener('click', () => {
      const target = tab.dataset.tab;
      appState.activeTab = target;
      DOM.suiteTabs.forEach(t => t.classList.toggle('active', t === tab));
      DOM.tabViews.forEach(v => v.classList.toggle('active', v.id === `view-${target}`));
    });
  });

  // Transition Timing Speed
  DOM.selectTransitionSpeed.addEventListener('change', (e) => {
    appState.transitionDuration = parseInt(e.target.value, 10);
    document.documentElement.style.setProperty('--lighting-fade-speed', `${appState.transitionDuration / 1000}s`);
    showToast(`Fade transition set to ${e.target.options[e.target.selectedIndex].text}`);
    dispatchStateChange({ transitionDuration: appState.transitionDuration });
  });

  // Master Power
  DOM.btnMasterPower.addEventListener('click', () => {
    appState.isMasterOn = !appState.isMasterOn;
    renderAll();
    dispatchStateChange({ isMasterOn: appState.isMasterOn });
    showToast(appState.isMasterOn ? 'Room lighting turned ON' : 'Room lighting turned OFF');
  });

  // Master Brightness Slider
  DOM.sliderMasterBrightness.addEventListener('input', (e) => {
    appState.masterBrightness = parseInt(e.target.value, 10);
    renderAll();
  });
  DOM.sliderMasterBrightness.addEventListener('change', () => {
    dispatchStateChange({ masterBrightness: appState.masterBrightness });
  });

  // Contrast Slider
  DOM.sliderMasterContrast.addEventListener('input', (e) => {
    appState.contrastRatio = parseInt(e.target.value, 10);
    renderAll();
  });
  DOM.sliderMasterContrast.addEventListener('change', () => {
    dispatchStateChange({ contrastRatio: appState.contrastRatio });
  });

  // Presets
  document.querySelectorAll('.scene-card').forEach(card => {
    card.addEventListener('click', () => {
      applyPreset(card.dataset.preset);
    });
  });

  // Desk Controls
  DOM.toggleDeskPower.addEventListener('change', (e) => {
    appState.devices.desk.power = e.target.checked;
    renderAll();
    dispatchStateChange({ desk: { power: appState.devices.desk.power } });
  });

  DOM.sliderDeskBrightness.addEventListener('input', (e) => {
    appState.devices.desk.brightness = parseInt(e.target.value, 10);
    renderAll();
  });
  DOM.sliderDeskBrightness.addEventListener('change', () => {
    dispatchStateChange({ desk: { brightness: appState.devices.desk.brightness } });
  });

  DOM.pickerDeskColor.addEventListener('input', (e) => {
    appState.devices.desk.color = e.target.value;
    renderAll();
  });
  DOM.pickerDeskColor.addEventListener('change', () => {
    dispatchStateChange({ desk: { color: appState.devices.desk.color } });
  });

  document.querySelectorAll('#effects-selector .effect-chip').forEach(chip => {
    chip.addEventListener('click', () => {
      appState.devices.desk.effect = chip.dataset.effect;
      renderAll();
      dispatchStateChange({ desk: { effect: appState.devices.desk.effect } });
    });
  });

  DOM.deskSwatches.addEventListener('click', (e) => {
    const dot = e.target.closest('.swatch-dot');
    if (dot) {
      appState.devices.desk.color = dot.dataset.color;
      renderAll();
      dispatchStateChange({ desk: { color: appState.devices.desk.color } });
    }
  });

  DOM.sliderDeskSpeed.addEventListener('input', (e) => {
    appState.devices.desk.speed = parseInt(e.target.value, 10);
    renderAll();
  });
  DOM.sliderDeskIntensity.addEventListener('input', (e) => {
    appState.devices.desk.intensity = parseInt(e.target.value, 10);
    renderAll();
  });

  // WiZ Left Controls
  DOM.toggleWizLeftPower.addEventListener('change', (e) => {
    appState.devices.wizLeft.power = e.target.checked;
    if (appState.syncCorners) appState.devices.wizRight.power = e.target.checked;
    renderAll();
    dispatchStateChange({
      wizLeft: { power: appState.devices.wizLeft.power },
      ...(appState.syncCorners ? { wizRight: { power: appState.devices.wizRight.power } } : {})
    });
  });

  DOM.sliderWizLeftBrightness.addEventListener('input', (e) => {
    appState.devices.wizLeft.brightness = parseInt(e.target.value, 10);
    if (appState.syncCorners) appState.devices.wizRight.brightness = appState.devices.wizLeft.brightness;
    renderAll();
  });

  DOM.pickerWizLeftColor.addEventListener('input', (e) => {
    appState.devices.wizLeft.color = e.target.value;
    if (appState.syncCorners) appState.devices.wizRight.color = e.target.value;
    renderAll();
  });

  DOM.sliderWizLeftKelvin.addEventListener('input', (e) => {
    appState.devices.wizLeft.kelvin = parseInt(e.target.value, 10);
    if (appState.syncCorners) appState.devices.wizRight.kelvin = appState.devices.wizLeft.kelvin;
    renderAll();
  });

  // WiZ Right Controls
  DOM.toggleWizRightPower.addEventListener('change', (e) => {
    appState.devices.wizRight.power = e.target.checked;
    renderAll();
    dispatchStateChange({ wizRight: { power: appState.devices.wizRight.power } });
  });

  DOM.sliderWizRightBrightness.addEventListener('input', (e) => {
    appState.devices.wizRight.brightness = parseInt(e.target.value, 10);
    renderAll();
  });

  DOM.pickerWizRightColor.addEventListener('input', (e) => {
    appState.devices.wizRight.color = e.target.value;
    renderAll();
  });

  DOM.sliderWizRightKelvin.addEventListener('input', (e) => {
    appState.devices.wizRight.kelvin = parseInt(e.target.value, 10);
    renderAll();
  });

  // Mode Tabs
  document.querySelectorAll('.mode-tab').forEach(tab => {
    tab.addEventListener('click', () => {
      const dev = tab.dataset.device;
      const mode = tab.dataset.mode;
      if (dev === 'wiz-left') {
        appState.devices.wizLeft.mode = mode;
        if (appState.syncCorners) appState.devices.wizRight.mode = mode;
      } else {
        appState.devices.wizRight.mode = mode;
      }
      renderAll();
    });
  });

  DOM.btnSyncCorners.addEventListener('click', () => {
    appState.syncCorners = !appState.syncCorners;
    if (appState.syncCorners) {
      Object.assign(appState.devices.wizRight, JSON.parse(JSON.stringify(appState.devices.wizLeft)));
      showToast('Corner bulbs linked.');
    } else {
      showToast('Corner bulbs unlinked.');
    }
    renderAll();
  });

  // Circadian Controls
  DOM.toggleCircadianAuto.addEventListener('change', (e) => {
    appState.circadian.autoEnabled = e.target.checked;
    if (appState.circadian.autoEnabled) {
      const now = new Date();
      appState.circadian.simulatedMinutes = now.getHours() * 60 + now.getMinutes();
      applyCircadianToRoom();
      showToast('Auto-Circadian Adaptation enabled.');
    } else {
      showToast('Auto-Circadian Adaptation disabled.');
    }
    renderAll();
  });

  DOM.sliderCircadianTime.addEventListener('input', (e) => {
    appState.circadian.simulatedMinutes = parseInt(e.target.value, 10);
    updateCircadianSolarDisplay();
    if (appState.circadian.autoEnabled) {
      applyCircadianToRoom();
    }
  });

  DOM.btnCircadianNow.addEventListener('click', () => {
    const now = new Date();
    appState.circadian.simulatedMinutes = now.getHours() * 60 + now.getMinutes();
    DOM.sliderCircadianTime.value = appState.circadian.simulatedMinutes;
    applyCircadianToRoom();
    showToast('Synchronized to current solar clock.');
  });

  document.querySelectorAll('.phase-card').forEach(btn => {
    btn.addEventListener('click', () => {
      const phase = btn.dataset.phase;
      if (phase === 'dawn') appState.circadian.simulatedMinutes = 6 * 60;
      else if (phase === 'noon') appState.circadian.simulatedMinutes = 12 * 60;
      else if (phase === 'dusk') appState.circadian.simulatedMinutes = 18 * 60;
      else if (phase === 'night') appState.circadian.simulatedMinutes = 23 * 60;
      DOM.sliderCircadianTime.value = appState.circadian.simulatedMinutes;
      applyCircadianToRoom();
    });
  });

  // Ambilight
  DOM.btnToggleAmbilight.addEventListener('click', toggleAmbilight);
  DOM.sliderAmbiSmoothing.addEventListener('input', (e) => {
    appState.ambilight.smoothing = parseInt(e.target.value, 10) / 100;
    DOM.valAmbiSmoothing.textContent = `${e.target.value}% (Cinematic)`;
  });
  DOM.sliderAmbiBoost.addEventListener('input', (e) => {
    appState.ambilight.saturationBoost = parseInt(e.target.value, 10) / 10;
    DOM.valAmbiBoost.textContent = `${appState.ambilight.saturationBoost}× Vibrant`;
  });

  // SoundSync
  DOM.btnToggleSoundSync.addEventListener('click', toggleSoundSync);
  DOM.sliderAudioSens.addEventListener('input', (e) => {
    appState.soundsync.sensitivity = parseInt(e.target.value, 10);
    DOM.valAudioSens.textContent = `${e.target.value}%`;
  });

  // Gradient Lab
  [DOM.gradColor1, DOM.gradColor2, DOM.gradColor3].forEach(picker => {
    picker.addEventListener('input', updateGradientPreviewBar);
  });
  DOM.btnApplyGradient.addEventListener('click', applyCustomGradientToDesk);

  document.querySelectorAll('.palette-chip').forEach(chip => {
    chip.addEventListener('click', () => {
      const stops = chip.dataset.stops.split(',');
      DOM.gradColor1.value = stops[0];
      DOM.gradColor2.value = stops[1];
      DOM.gradColor3.value = stops[2];
      updateGradientPreviewBar();
      applyCustomGradientToDesk();
    });
  });

  // Focus Rhythm (Pomodoro)
  DOM.btnTogglePomodoro.addEventListener('click', togglePomodoro);

  // Settings Dialog
  DOM.btnOpenSettings.addEventListener('click', () => {
    DOM.modalSettings.showModal();
    DOM.inputWizLeftIp.value = appState.network.wizLeftIp;
    DOM.inputWizRightIp.value = appState.network.wizRightIp;
  });

  DOM.btnCloseSettings.addEventListener('click', () => DOM.modalSettings.close());
  DOM.modalSettings.addEventListener('click', (e) => {
    const d = DOM.modalSettings.getBoundingClientRect();
    if (e.clientX < d.left || e.clientX > d.right || e.clientY < d.top || e.clientY > d.bottom) {
      DOM.modalSettings.close();
    }
  });

  DOM.btnScanWiz.addEventListener('click', runWizDiscovery);
  DOM.btnSaveSettings.addEventListener('click', () => {
    appState.network.wizLeftIp = DOM.inputWizLeftIp.value.trim();
    appState.network.wizRightIp = DOM.inputWizRightIp.value.trim();
    localStorage.setItem('aura_wiz_left_ip', appState.network.wizLeftIp);
    localStorage.setItem('aura_wiz_right_ip', appState.network.wizRightIp);
    DOM.ipDisplayWizLeft.textContent = appState.network.wizLeftIp;
    DOM.ipDisplayWizRight.textContent = appState.network.wizRightIp;
    DOM.modalSettings.close();
    showToast('Network settings saved.');
  });

  DOM.toggleGamma.addEventListener('change', (e) => {
    appState.useGammaCorrection = e.target.checked;
    renderAll();
    showToast(appState.useGammaCorrection ? 'CIE 1931 Perceptual Gamma enabled.' : 'Linear dimming active.');
  });

  setupBulbSwatches('wiz-left-swatches', 'wizLeft');
  setupBulbSwatches('wiz-right-swatches', 'wizRight');
}

function setupBulbSwatches(containerId, deviceKey) {
  const container = document.getElementById(containerId);
  if (!container) return;
  container.addEventListener('click', (e) => {
    const dot = e.target.closest('.swatch-dot');
    if (dot) {
      appState.devices[deviceKey].color = dot.dataset.color;
      appState.devices[deviceKey].mode = 'rgb';
      if (appState.syncCorners) {
        appState.devices.wizRight.color = dot.dataset.color;
        appState.devices.wizRight.mode = 'rgb';
      }
      renderAll();
    }
  });
}

function applyPreset(presetKey) {
  const preset = PRESETS[presetKey];
  if (!preset) return;

  appState.activePreset = presetKey;
  appState.masterBrightness = preset.masterBrightness;
  appState.contrastRatio = preset.contrast;
  appState.isMasterOn = true;

  Object.assign(appState.devices.desk, preset.desk);
  Object.assign(appState.devices.wizLeft, preset.wizLeft);
  Object.assign(appState.devices.wizRight, preset.wizRight);

  renderAll();
  dispatchStateChange({ state: appState });
  showToast(`Activated scene: ${preset.name}`);
}

function runWizDiscovery() {
  DOM.discoveryResults.innerHTML = `
    <div class="empty-state">
      <span class="status-dot" style="display:inline-block; margin-right:8px; background:var(--accent-cyan); animation:pulse 1s infinite"></span>
      Broadcasting UDP discovery query to 255.255.255.255:38899...
    </div>
  `;

  if (socket && socket.readyState === WebSocket.OPEN) {
    socket.send(JSON.stringify({ type: 'scanWiz' }));
  } else {
    setTimeout(() => {
      const mockDevices = [
        { ip: '192.168.1.101', mac: 'a8bb501234a1', model: 'WiZ Bulb RGB 800lm', rssi: -48 },
        { ip: '192.168.1.102', mac: 'a8bb501234a2', model: 'WiZ Bulb RGB 800lm', rssi: -54 }
      ];
      renderDiscoveredDevices(mockDevices);
    }, 1200);
  }
}

function renderDiscoveredDevices(devices) {
  if (!devices || devices.length === 0) {
    DOM.discoveryResults.innerHTML = `<div class="empty-state">No WiZ bulbs responded.</div>`;
    return;
  }

  DOM.discoveryResults.innerHTML = '';
  devices.forEach(dev => {
    const item = document.createElement('div');
    item.className = 'discovered-device-item';
    item.innerHTML = `
      <div>
        <strong style="font-size:0.8rem; color:#fff">${dev.model || 'Philips WiZ Bulb'}</strong>
        <div style="font-size:0.7rem; color:var(--text-muted); font-family:var(--font-mono)">
          IP: ${dev.ip} • MAC: ${dev.mac || 'N/A'} • ${dev.rssi ? dev.rssi + ' dBm' : 'Strong'}
        </div>
      </div>
      <div class="assign-btn-group">
        <button class="assign-btn" data-assign="left" data-ip="${dev.ip}">Set Left</button>
        <button class="assign-btn" data-assign="right" data-ip="${dev.ip}">Set Right</button>
      </div>
    `;

    item.querySelectorAll('.assign-btn').forEach(btn => {
      btn.addEventListener('click', () => {
        if (btn.dataset.assign === 'left') {
          DOM.inputWizLeftIp.value = btn.dataset.ip;
          showToast(`Assigned ${btn.dataset.ip} to Left WiZ`);
        } else {
          DOM.inputWizRightIp.value = btn.dataset.ip;
          showToast(`Assigned ${btn.dataset.ip} to Right WiZ`);
        }
      });
    });

    DOM.discoveryResults.appendChild(item);
  });
}

function kelvinToRgbHex(kelvin) {
  const temp = kelvin / 100;
  let red, green, blue;

  if (temp <= 66) red = 255;
  else {
    red = temp - 60;
    red = 329.698727446 * Math.pow(red, -0.1332047592);
    if (red < 0) red = 0;
    if (red > 255) red = 255;
  }

  if (temp <= 66) {
    green = temp;
    green = 99.4708025861 * Math.log(green) - 161.1195681661;
    if (green < 0) green = 0;
    if (green > 255) green = 255;
  } else {
    green = temp - 60;
    green = 288.1221695283 * Math.pow(green, -0.0755148492);
    if (green < 0) green = 0;
    if (green > 255) green = 255;
  }

  if (temp >= 66) blue = 255;
  else if (temp <= 19) blue = 0;
  else {
    blue = temp - 10;
    blue = 138.5177312231 * Math.log(blue) - 305.0447927307;
    if (blue < 0) blue = 0;
    if (blue > 255) blue = 255;
  }

  return rgbToHex(Math.round(red), Math.round(green), Math.round(blue));
}

function rgbToHex(r, g, b) {
  const rH = Math.min(255, Math.max(0, r)).toString(16).padStart(2, '0');
  const gH = Math.min(255, Math.max(0, g)).toString(16).padStart(2, '0');
  const bH = Math.min(255, Math.max(0, b)).toString(16).padStart(2, '0');
  return `#${rH}${gH}${bH}`;
}

function capitalize(str) {
  if (!str) return '';
  return str.charAt(0).toUpperCase() + str.slice(1);
}

function showToast(message) {
  if (!DOM.toastContainer) return;
  const toast = document.createElement('div');
  toast.className = 'toast';
  toast.textContent = message;
  DOM.toastContainer.appendChild(toast);

  setTimeout(() => {
    toast.style.opacity = '0';
    toast.style.transform = 'translateY(8px)';
    toast.style.transition = 'all 0.3s ease';
    setTimeout(() => toast.remove(), 300);
  }, 2800);
}

// ============================================================================
// 11. INITIALIZATION & RECURRING TIMER
// ============================================================================

window.addEventListener('DOMContentLoaded', () => {
  DOM.ipDisplayWizLeft.textContent = appState.network.wizLeftIp;
  DOM.ipDisplayWizRight.textContent = appState.network.wizRightIp;

  bindEvents();
  initCommunications();
  renderAll();

  // Background ticker for Circadian HCL auto-update
  setInterval(() => {
    if (appState.circadian.autoEnabled) {
      const now = new Date();
      appState.circadian.simulatedMinutes = now.getHours() * 60 + now.getMinutes();
      applyCircadianToRoom();
    }
  }, 30000);
});
