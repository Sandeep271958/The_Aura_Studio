/**
 * THE AURA STUDIO // Luxury Smart Lighting Architecture - Engine v3.0
 * Designed by Sandy
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
  },
  'zen-sanctuary': {
    name: 'Aman Kyoto Zen',
    masterBrightness: 75,
    contrast: -5,
    desk: { power: true, brightness: 75, color: '#fbcfe8', effect: 'wave', speed: 30, intensity: 45 },
    wizLeft: { power: true, brightness: 60, mode: 'cct', color: '#ff8a12', kelvin: 2000 },
    wizRight: { power: true, brightness: 60, mode: 'cct', color: '#ff8a12', kelvin: 2000 }
  },
  'grand-welcome': {
    name: 'Grand Welcome',
    masterBrightness: 85,
    contrast: 10,
    desk: { power: true, brightness: 85, color: '#ffc078', effect: 'breathe', speed: 35, intensity: 60 },
    wizLeft: { power: true, brightness: 80, mode: 'cct', color: '#ffa757', kelvin: 2700 },
    wizRight: { power: true, brightness: 80, mode: 'cct', color: '#ffa757', kelvin: 2700 }
  },
  'bio-spa': {
    name: 'Deep Sanctuary Spa',
    masterBrightness: 70,
    contrast: -15,
    desk: { power: true, brightness: 75, color: '#0d9488', effect: 'shimmer', speed: 25, intensity: 55 },
    wizLeft: { power: true, brightness: 65, mode: 'cct', color: '#ff9b3d', kelvin: 2400 },
    wizRight: { power: true, brightness: 65, mode: 'cct', color: '#ff9b3d', kelvin: 2400 }
  },
  'ocean-abyss': {
    name: 'Oceanic Abyss',
    masterBrightness: 80,
    contrast: 20,
    desk: { power: true, brightness: 90, color: '#00f2fe', effect: 'soap-bubble', speed: 50, intensity: 85 },
    wizLeft: { power: true, brightness: 70, mode: 'rgb', color: '#0b192c', kelvin: 4000 },
    wizRight: { power: true, brightness: 70, mode: 'rgb', color: '#0b192c', kelvin: 4000 }
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
  onOffAnimation: 'gaussian-ripple',
  
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
    stops: ['#a5f3fc', '#f472b6', '#c084fc', '#fef08a'],
    motionMode: 'soap-bubble', // 'soap-bubble', 'wave', 'caustics', 'soliton', 'thermal-plume', 'marangoni', 'breathe', 'shimmer'
    speed: 45,
    turbulence: 50,
    bulbSyncMode: 'spatial-sync', // 'spatial-sync', 'warm-white', 'custom'
    warmWhiteKelvin: 2400,
    bulbCustomColor: '#ffa757',
    phase: 0,
    animFrameId: null
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
    wizLeftMac: localStorage.getItem('aura_wiz_left_mac') || '',
    wizRightIp: localStorage.getItem('aura_wiz_right_ip') || '192.168.1.102',
    wizRightMac: localStorage.getItem('aura_wiz_right_mac') || ''
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
  btnRoomPowerCard: document.getElementById('btn-room-power-card'),
  labelRoomPowerCard: document.getElementById('label-room-power-card'),

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
  valDeskHue: document.getElementById('val-desk-hue'),
  sliderDeskHue: document.getElementById('slider-desk-hue'),
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
  valWizLeftHue: document.getElementById('val-wiz-left-hue'),
  sliderWizLeftHue: document.getElementById('slider-wiz-left-hue'),
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
  valWizRightHue: document.getElementById('val-wiz-right-hue'),
  sliderWizRightHue: document.getElementById('slider-wiz-right-hue'),
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
  gradientFluidCanvas: document.getElementById('gradient-fluid-canvas'),
  gradientPreviewBar: document.getElementById('gradient-preview-bar'),
  gradColor1: document.getElementById('grad-color-1'),
  gradColor2: document.getElementById('grad-color-2'),
  gradColor3: document.getElementById('grad-color-3'),
  gradColor4: document.getElementById('grad-color-4'),
  gradientMotionSelector: document.getElementById('gradient-motion-selector'),
  sliderGradientSpeed: document.getElementById('slider-gradient-speed'),
  valGradientSpeed: document.getElementById('val-gradient-speed'),
  sliderGradientTurbulence: document.getElementById('slider-gradient-turbulence'),
  valGradientTurbulence: document.getElementById('val-gradient-turbulence'),
  gradientBulbSyncModes: document.getElementById('gradient-bulb-sync-modes'),
  wrapWarmWhiteCct: document.getElementById('wrap-warm-white-cct'),
  selectGradientWarmCct: document.getElementById('select-gradient-warm-cct'),
  wrapCustomBulbColor: document.getElementById('wrap-custom-bulb-color'),
  pickerGradientBulbColor: document.getElementById('picker-gradient-bulb-color'),
  btnApplyGradient: document.getElementById('btn-apply-gradient'),

  // Focus Rhythm (Pomodoro)
  pomoTimerRing: document.getElementById('pomo-timer-ring'),
  btnTogglePomodoro: document.getElementById('btn-toggle-pomodoro'),
  btnResetPomodoro: document.getElementById('btn-reset-pomodoro'),
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
  telemHubIp: document.getElementById('telem-hub-ip'),
  toggleGamma: document.getElementById('toggle-gamma'),

  // Wireless OTA Firmware Update
  badgeFirmwareVersion: document.getElementById('badge-firmware-version'),
  badgeOtaPartition: document.getElementById('badge-ota-partition'),
  otaRollbackBanner: document.getElementById('ota-rollback-banner'),
  btnDismissRollback: document.getElementById('btn-dismiss-rollback'),
  otaDropZone: document.getElementById('ota-drop-zone'),
  inputOtaFile: document.getElementById('input-ota-file'),
  btnSelectOtaFile: document.getElementById('btn-select-ota-file'),
  btnStartOtaUpload: document.getElementById('btn-start-ota-upload'),
  otaFileLabel: document.getElementById('ota-file-label'),
  otaFileSize: document.getElementById('ota-file-size'),
  otaProgressContainer: document.getElementById('ota-progress-container'),
  otaProgressBar: document.getElementById('ota-progress-bar'),
  otaStatusText: document.getElementById('ota-status-text'),
  otaPctText: document.getElementById('ota-pct-text'),

  // Desk Strip On/Off Transition Animation
  selectOnOffAnim: document.getElementById('select-onoff-anim'),

  // Wi-Fi & Network Provisioning
  btnToggleWifiChange: document.getElementById('btn-toggle-wifi-change'),
  wifiCurrentSsid: document.getElementById('wifi-current-ssid'),
  wifiCurrentRssi: document.getElementById('wifi-current-rssi'),
  wifiChangePanel: document.getElementById('wifi-change-panel'),
  btnScanWifi: document.getElementById('btn-scan-wifi'),
  selectWifiNetworks: document.getElementById('select-wifi-networks'),
  inputWifiSsid: document.getElementById('input-wifi-ssid'),
  inputWifiPass: document.getElementById('input-wifi-pass'),
  btnToggleWifiPassVisibility: document.getElementById('btn-toggle-wifi-pass-visibility'),
  btnSubmitWifiChange: document.getElementById('btn-submit-wifi-change'),
  btnResetWifi: document.getElementById('btn-reset-wifi'),
  wifiStatusFeedback: document.getElementById('wifi-status-feedback'),

  // Hardware & Pixel Diagnostics
  sliderPixelDiag: document.getElementById('slider-pixel-diag'),
  valPixelDiag: document.getElementById('val-pixel-diag'),
  btnPixelDiagReset: document.getElementById('btn-pixel-diag-reset'),

  // Smart Hub Target (for Localhost / Companion Mode)
  inputHubHost: document.getElementById('input-hub-host'),

  // Browser Security & Media Permissions
  btnOpenSecurityGuide: document.getElementById('btn-open-security-guide'),
  securityStatusBadge: document.getElementById('security-status-badge'),
  modalSecurityGuide: document.getElementById('modal-security-guide'),
  btnCloseSecurityGuide: document.getElementById('btn-close-security-guide'),
  btnDismissSecurityGuide: document.getElementById('btn-dismiss-security-guide'),
  securityStatusBanner: document.getElementById('security-status-banner'),
  securityStatusTitle: document.getElementById('security-status-title'),
  securityStatusDesc: document.getElementById('security-status-desc'),
  btnCopyChromeFlag: document.getElementById('btn-copy-chrome-flag'),
  btnCopyChromeOrigin: document.getElementById('btn-copy-chrome-origin'),
  btnCopyPythonCmd: document.getElementById('btn-copy-python-cmd'),
  valChromeOrigin: document.getElementById('val-chrome-origin'),

  // Toast
  toastContainer: document.getElementById('toast-container')
};

// ============================================================================
// 3. WEBSOCKET & COMMUNICATIONS
// ============================================================================

let socket = null;
let isSimulatedMode = false;
let socketReconnectTimer = null;

function getTargetHubHost() {
  const isLocalhost = window.location.hostname === 'localhost' || window.location.hostname === '127.0.0.1';
  if (isLocalhost) {
    return localStorage.getItem('aura_hub_host') || 'aura-hub.local';
  }
  return window.location.host;
}

function initCommunications() {
  if (window.location.protocol === 'file:') {
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

let wsRetryCount = 0;
function connectWebSocket() {
  const targetHost = getTargetHubHost();
  const wsUrl = `ws://${targetHost}/ws`;
  try {
    socket = new WebSocket(wsUrl);

    socket.onopen = () => {
      wsRetryCount = 0;
      isSimulatedMode = false;
      DOM.connectionBadge.className = 'status-chip connected';
      const isLocalhost = window.location.hostname === 'localhost' || window.location.hostname === '127.0.0.1';
      DOM.connectionText.textContent = isLocalhost ? `ESP32 (${targetHost})` : 'ESP32 ONLINE';
      showToast(`Connected to ESP32 Smart Hub (${targetHost}).`);
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
      wsRetryCount++;
      DOM.connectionBadge.className = 'status-chip simulated';
      DOM.connectionText.textContent = 'RECONNECTING...';
      const isLocalhost = window.location.hostname === 'localhost' || window.location.hostname === '127.0.0.1';
      if (isLocalhost && wsRetryCount > 2) {
        DOM.connectionText.textContent = 'SIMULATOR / RETRYING';
      }
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
    if (payload && payload.state) {
      payload = {
        isMasterOn: payload.state.isMasterOn,
        masterBrightness: payload.state.masterBrightness,
        contrastRatio: payload.state.contrastRatio,
        syncCorners: payload.state.syncCorners,
        activePreset: payload.state.activePreset,
        onOffAnimation: payload.state.onOffAnimation,
        network: payload.state.network,
        devices: payload.state.devices
      };
    }
    socket.send(JSON.stringify({
      type: 'update',
      payload: payload
    }));
  }
}

document.addEventListener('visibilitychange', () => {
  if (document.visibilityState === 'visible' && (!socket || socket.readyState !== WebSocket.OPEN)) {
    if (window.location.protocol.startsWith('http') &&
        window.location.hostname !== 'localhost' &&
        window.location.hostname !== '127.0.0.1') {
      connectWebSocket();
    }
  }
});

function handleIncomingMessage(msg) {
  if (msg.type === 'state') {
    Object.assign(appState, msg.payload);
    if (msg.payload && msg.payload.firmwareVersion && DOM.badgeFirmwareVersion) {
      DOM.badgeFirmwareVersion.textContent = `v${msg.payload.firmwareVersion}`;
    }
    if (msg.payload && DOM.badgeOtaPartition && msg.payload.activePartition) {
      const partName = msg.payload.activePartition === 'app0' ? 'ota_0' : (msg.payload.activePartition === 'app1' ? 'ota_1' : msg.payload.activePartition);
      const isTrial = !!msg.payload.otaPendingVerify;
      DOM.badgeOtaPartition.textContent = `Slot: ${partName}${isTrial ? ' [Trial]' : ''}`;
      if (isTrial) {
        DOM.badgeOtaPartition.classList.add('trial');
      } else {
        DOM.badgeOtaPartition.classList.remove('trial');
      }
    }
    if (msg.payload && DOM.otaRollbackBanner) {
      if (msg.payload.otaRollbackOccurred) {
        DOM.otaRollbackBanner.classList.remove('hidden');
      } else {
        DOM.otaRollbackBanner.classList.add('hidden');
      }
    }
    if (msg.payload && msg.payload.onOffAnimation) {
      appState.onOffAnimation = msg.payload.onOffAnimation;
      if (DOM.selectOnOffAnim) DOM.selectOnOffAnim.value = appState.onOffAnimation;
    }
    if (msg.payload && msg.payload.network) {
      if (msg.payload.network.wizLeftMac) {
        appState.network.wizLeftMac = msg.payload.network.wizLeftMac;
        localStorage.setItem('aura_wiz_left_mac', appState.network.wizLeftMac);
      }
      if (msg.payload.network.wizRightMac) {
        appState.network.wizRightMac = msg.payload.network.wizRightMac;
        localStorage.setItem('aura_wiz_right_mac', appState.network.wizRightMac);
      }
    }
    // Preserve user assigned WiZ bulb IPs if ESP32 returns unconfigured factory defaults
    const savedLeft = localStorage.getItem('aura_wiz_left_ip');
    const savedRight = localStorage.getItem('aura_wiz_right_ip');
    if (savedLeft && appState.network.wizLeftIp === '192.168.1.101') {
      appState.network.wizLeftIp = savedLeft;
    }
    if (savedRight && appState.network.wizRightIp === '192.168.1.102') {
      appState.network.wizRightIp = savedRight;
    }
    const savedLeftMac = localStorage.getItem('aura_wiz_left_mac');
    const savedRightMac = localStorage.getItem('aura_wiz_right_mac');
    if (savedLeftMac && !appState.network.wizLeftMac) {
      appState.network.wizLeftMac = savedLeftMac;
    }
    if (savedRightMac && !appState.network.wizRightMac) {
      appState.network.wizRightMac = savedRightMac;
    }
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

  let ratioDisplay;
  if (!appState.isMasterOn || (effBrightDesk === 0 && effBrightWizLeft === 0)) {
    ratioDisplay = 'OFF';
  } else if (effBrightWizLeft > 0) {
    ratioDisplay = `${(effBrightDesk / effBrightWizLeft).toFixed(1)}:1`;
  } else {
    ratioDisplay = effBrightDesk > 0 ? '10.0:1' : '1:1';
  }
  DOM.telemetryContrast.innerHTML = ratioDisplay === 'OFF'
    ? 'CONTRAST RATIO: <strong>OFF</strong>'
    : `CONTRAST RATIO: <strong>${ratioDisplay}</strong>`;
}

function syncAllInputs() {
  if (DOM.ipDisplayWizLeft && appState.network && appState.network.wizLeftIp) {
    DOM.ipDisplayWizLeft.textContent = appState.network.wizLeftIp;
  }
  if (DOM.ipDisplayWizRight && appState.network && appState.network.wizRightIp) {
    DOM.ipDisplayWizRight.textContent = appState.network.wizRightIp;
  }
  if (DOM.inputWizLeftIp && appState.network && appState.network.wizLeftIp) {
    DOM.inputWizLeftIp.value = appState.network.wizLeftIp;
  }
  if (DOM.inputWizRightIp && appState.network && appState.network.wizRightIp) {
    DOM.inputWizRightIp.value = appState.network.wizRightIp;
  }
  if (DOM.telemHubIp && appState.network && appState.network.hubIp) {
    DOM.telemHubIp.textContent = appState.network.hubIp;
  }
  if (DOM.wifiCurrentSsid && appState.network) {
    DOM.wifiCurrentSsid.textContent = appState.network.wifiSsid || 'Setup Mode (SoftAP)';
  }
  if (DOM.wifiCurrentRssi && appState.network) {
    const rssi = appState.network.wifiRssi;
    if (rssi) {
      let qual = 'Good';
      if (rssi >= -55) qual = 'Excellent';
      else if (rssi <= -75) qual = 'Fair';
      DOM.wifiCurrentRssi.textContent = `${rssi} dBm (${qual})`;
    } else {
      DOM.wifiCurrentRssi.textContent = appState.network.isSetupMode ? 'SoftAP Active' : '--';
    }
  }

  if (DOM.btnMasterPower) DOM.btnMasterPower.classList.toggle('active', appState.isMasterOn);
  if (DOM.btnRoomPowerCard) {
    DOM.btnRoomPowerCard.classList.toggle('active', appState.isMasterOn);
    if (DOM.labelRoomPowerCard) {
      DOM.labelRoomPowerCard.textContent = appState.isMasterOn ? 'ROOM ON' : 'ROOM OFF';
    }
  }
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
  const deskHue = hexToHue(appState.devices.desk.color);
  if (DOM.sliderDeskHue) {
    DOM.sliderDeskHue.value = deskHue;
    DOM.sliderDeskHue.style.setProperty('--control-value', deskHue);
  }
  if (DOM.valDeskHue) DOM.valDeskHue.textContent = `${deskHue}\u00b0 Hue`;
  syncActiveSwatches(DOM.deskSwatches, appState.devices.desk.color);

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
  DOM.valWizLeftHex.textContent = appState.devices.wizLeft.mode === 'cct'
    ? `${appState.devices.wizLeft.kelvin}K`
    : appState.devices.wizLeft.color.toUpperCase();
  const wizLeftHue = hexToHue(appState.devices.wizLeft.color);
  if (DOM.sliderWizLeftHue) {
    DOM.sliderWizLeftHue.value = wizLeftHue;
    DOM.sliderWizLeftHue.style.setProperty('--control-value', wizLeftHue);
  }
  if (DOM.valWizLeftHue) DOM.valWizLeftHue.textContent = `${wizLeftHue}\u00b0 Hue`;
  syncActiveSwatches(document.getElementById('wiz-left-swatches'), appState.devices.wizLeft.mode === 'cct' ? null : appState.devices.wizLeft.color);

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
  DOM.valWizRightHex.textContent = appState.devices.wizRight.mode === 'cct'
    ? `${appState.devices.wizRight.kelvin}K`
    : appState.devices.wizRight.color.toUpperCase();
  const wizRightHue = hexToHue(appState.devices.wizRight.color);
  if (DOM.sliderWizRightHue) {
    DOM.sliderWizRightHue.value = wizRightHue;
    DOM.sliderWizRightHue.style.setProperty('--control-value', wizRightHue);
  }
  if (DOM.valWizRightHue) DOM.valWizRightHue.textContent = `${wizRightHue}\u00b0 Hue`;
  syncActiveSwatches(document.getElementById('wiz-right-swatches'), appState.devices.wizRight.mode === 'cct' ? null : appState.devices.wizRight.color);

  DOM.sliderWizRightKelvin.value = appState.devices.wizRight.kelvin;
  DOM.valWizRightKelvin.textContent = `${appState.devices.wizRight.kelvin}K`;
  syncRangeProgress(DOM.sliderWizRightKelvin);

  syncModeTabs('wiz-right', appState.devices.wizRight.mode);

  DOM.btnSyncCorners.classList.toggle('active', appState.syncCorners);
  DOM.toggleCircadianAuto.checked = appState.circadian.autoEnabled;

  if (DOM.sliderCircadianTime) {
    DOM.sliderCircadianTime.value = appState.circadian.simulatedMinutes;
    syncRangeProgress(DOM.sliderCircadianTime);
  }
  if (DOM.sliderAmbiSmoothing) {
    const sVal = Math.round(appState.ambilight.smoothing * 100);
    DOM.sliderAmbiSmoothing.value = sVal;
    if (DOM.valAmbiSmoothing) DOM.valAmbiSmoothing.textContent = `${sVal}% (Cinematic)`;
    syncRangeProgress(DOM.sliderAmbiSmoothing);
  }
  if (DOM.sliderAmbiBoost) {
    const bVal = Math.round(appState.ambilight.saturationBoost * 10);
    DOM.sliderAmbiBoost.value = bVal;
    if (DOM.valAmbiBoost) DOM.valAmbiBoost.textContent = `${(bVal / 10).toFixed(1)}× Vibrant`;
    syncRangeProgress(DOM.sliderAmbiBoost);
  }
  if (DOM.sliderAmbiBlackfloor) {
    DOM.sliderAmbiBlackfloor.value = appState.ambilight.blackFloor;
    if (DOM.valAmbiBlackfloor) DOM.valAmbiBlackfloor.textContent = `${appState.ambilight.blackFloor}% Brightness`;
    syncRangeProgress(DOM.sliderAmbiBlackfloor);
  }
  if (DOM.sliderAudioSens) {
    DOM.sliderAudioSens.value = appState.soundsync.sensitivity;
    syncRangeProgress(DOM.sliderAudioSens);
  }
  if (DOM.sliderGradientSpeed) {
    DOM.sliderGradientSpeed.value = appState.gradient.speed;
    if (DOM.valGradientSpeed) DOM.valGradientSpeed.textContent = `${appState.gradient.speed}% Velocity`;
    syncRangeProgress(DOM.sliderGradientSpeed);
  }
  if (DOM.sliderGradientTurbulence) {
    DOM.sliderGradientTurbulence.value = appState.gradient.turbulence;
    const val = appState.gradient.turbulence;
    let label = `${val}% (Organic Flow)`;
    if (val < 30) label = `${val}% (Laminar Smooth)`;
    else if (val > 70) label = `${val}% (Vortex Plasma)`;
    if (DOM.valGradientTurbulence) DOM.valGradientTurbulence.textContent = label;
    syncRangeProgress(DOM.sliderGradientTurbulence);
  }
  if (DOM.selectTransitionSpeed) {
    DOM.selectTransitionSpeed.value = appState.transitionDuration;
  }
  if (DOM.toggleGamma) {
    DOM.toggleGamma.checked = appState.useGammaCorrection;
  }
  if (DOM.selectGradientWarmCct) {
    DOM.selectGradientWarmCct.value = appState.gradient.warmWhiteKelvin;
  }
  if (DOM.pickerGradientBulbColor) {
    DOM.pickerGradientBulbColor.value = appState.gradient.bulbCustomColor;
  }
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
// BROWSER SECURITY CONTEXT & MEDIA PERMISSIONS
// ============================================================================

function checkIsSecureContext() {
  if (typeof window === 'undefined') return true;
  if (window.isSecureContext) return true;
  if (window.location.hostname === 'localhost' || window.location.hostname === '127.0.0.1') return true;
  return false;
}

function updateSecurityContextUI() {
  const isSecure = checkIsSecureContext();
  const hasMediaDevices = typeof navigator !== 'undefined' && !!navigator.mediaDevices && !!navigator.mediaDevices.getDisplayMedia;

  if (DOM.securityStatusBadge) {
    if (isSecure && hasMediaDevices) {
      DOM.securityStatusBadge.className = 'badge-secure-status secure';
      DOM.securityStatusBadge.textContent = '✓ Secure Context (Unlocked)';
    } else {
      DOM.securityStatusBadge.className = 'badge-secure-status insecure';
      DOM.securityStatusBadge.textContent = '⚠️ Insecure Origin (Restricted)';
    }
  }

  if (DOM.securityStatusBanner && DOM.securityStatusTitle && DOM.securityStatusDesc) {
    if (isSecure && hasMediaDevices) {
      DOM.securityStatusBanner.className = 'security-banner secure';
      DOM.securityStatusTitle.textContent = '✓ Secure Context Active (Safe & Verified)';
      DOM.securityStatusDesc.textContent = 'Google Chrome has granted Safe & Secure status to this dashboard. Screen Sharing (Ambilight) and Microphone (SoundSync) access are fully unlocked and ready.';
    } else {
      DOM.securityStatusBanner.className = 'security-banner insecure';
      DOM.securityStatusTitle.textContent = 'Insecure Origin Restricted by Google Chrome';
      DOM.securityStatusDesc.textContent = 'Google Chrome requires a "Secure Context" before granting Screen Capture (Ambilight) and Microphone (SoundSync) permissions. Because aura-hub.local is served over local HTTP, Chrome restricts these sensitive media APIs.';
    }
  }

  if (DOM.valChromeOrigin && typeof window !== 'undefined') {
    const curOrigin = (window.location.protocol && window.location.protocol.startsWith('http') && window.location.origin) 
      ? window.location.origin 
      : 'http://aura-hub.local';
    DOM.valChromeOrigin.textContent = curOrigin;
  }
}

function openSecurityGuideModal() {
  updateSecurityContextUI();
  if (DOM.modalSecurityGuide && typeof DOM.modalSecurityGuide.showModal === 'function') {
    DOM.modalSecurityGuide.showModal();
  }
}

function closeSecurityGuideModal() {
  if (DOM.modalSecurityGuide && typeof DOM.modalSecurityGuide.close === 'function') {
    DOM.modalSecurityGuide.close();
  }
}

function copyToClipboard(text, btnElement) {
  function onSuccess() {
    if (btnElement) {
      const orig = btnElement.textContent;
      btnElement.textContent = 'Copied!';
      btnElement.classList.add('copied');
      setTimeout(() => {
        btnElement.textContent = orig;
        btnElement.classList.remove('copied');
      }, 2000);
    }
    showToast(`Copied: ${text}`);
  }

  if (typeof navigator !== 'undefined' && navigator.clipboard && navigator.clipboard.writeText) {
    navigator.clipboard.writeText(text).then(onSuccess).catch(fallback);
  } else {
    fallback();
  }

  function fallback() {
    try {
      const ta = document.createElement('textarea');
      ta.value = text;
      ta.style.position = 'fixed';
      ta.style.left = '-9999px';
      document.body.appendChild(ta);
      ta.focus();
      ta.select();
      document.execCommand('copy');
      document.body.removeChild(ta);
      onSuccess();
    } catch (e) {
      showToast(`Copy manually: ${text}`);
    }
  }
}

// ============================================================================
// BACKGROUND SYNC ENGINE (Inline Web Worker Ticker)
// Keeps Ambilight Screen Sync and SoundSync Audio Reactive running continuously
// at 30 FPS even when the dashboard tab is in the background or minimized.
// ============================================================================

let syncWorker = null;

function initSyncWorker() {
  if (syncWorker || typeof Worker === 'undefined' || typeof Blob === 'undefined') return;
  try {
    const workerScript = `
      let timer = null;
      self.onmessage = function(e) {
        if (e.data === 'start') {
          if (!timer) {
            timer = setInterval(function() {
              self.postMessage('tick');
            }, 33);
          }
        } else if (e.data === 'stop') {
          if (timer) {
            clearInterval(timer);
            timer = null;
          }
        }
      };
    `;
    const blob = new Blob([workerScript], { type: 'application/javascript' });
    const workerUrl = URL.createObjectURL(blob);
    syncWorker = new Worker(workerUrl);
    syncWorker.onmessage = function(e) {
      if (e.data === 'tick') {
        onBackgroundSyncTick();
      }
    };
  } catch (e) {
    console.warn('[SyncWorker] Fallback to window timers:', e);
  }
}

function updateSyncWorkerState() {
  const needsWorker = (appState.ambilight && appState.ambilight.active) || 
                      (appState.soundsync && appState.soundsync.active);
  if (needsWorker) {
    if (!syncWorker) initSyncWorker();
    if (syncWorker) syncWorker.postMessage('start');
  } else {
    if (syncWorker) syncWorker.postMessage('stop');
  }
}

function onBackgroundSyncTick() {
  // Only execute via worker when tab is in background (requestAnimationFrame is paused by browser)
  if (typeof document !== 'undefined' && document.visibilityState === 'hidden') {
    if (appState.ambilight && appState.ambilight.active && typeof appState.ambilight.processFrame === 'function') {
      appState.ambilight.processFrame(false);
    }
    if (appState.soundsync && appState.soundsync.active && typeof appState.soundsync.processFrame === 'function') {
      appState.soundsync.processFrame(false);
    }
  }
}

// ============================================================================
// 6. FEATURE SUITE 2: AMBILIGHT REAL-TIME SCREEN SYNC
// ============================================================================

async function toggleAmbilight() {
  if (appState.ambilight.active) {
    stopAmbilight();
    return;
  }

  // Pre-flight check for Secure Context & mediaDevices support
  const isSecure = checkIsSecureContext();
  const hasMediaDevices = typeof navigator !== 'undefined' && !!navigator.mediaDevices && !!navigator.mediaDevices.getDisplayMedia;
  if (!isSecure || !hasMediaDevices) {
    openSecurityGuideModal();
    showToast('Secure Context required: Chrome restricts Screen Share on local HTTP.');
    return;
  }

  try {
    const stream = await navigator.mediaDevices.getDisplayMedia({
      video: { frameRate: 30, cursor: 'never' }
    });
    startAmbilight(stream);
  } catch (err) {
    if (err && err.name === 'NotAllowedError') {
      showToast('Screen capture permission was cancelled.');
    } else {
      openSecurityGuideModal();
      showToast('Screen capture permission was restricted by browser.');
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
  let lastAmbiDispatch = 0;

  function processAmbilightFrame(updateUi) {
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

      // Update UI Previews only when visible
      if (updateUi) {
        DOM.hexZoneLeft.textContent = hexLeft;
        DOM.hexZoneCenter.textContent = hexCenter;
        DOM.hexZoneRight.textContent = hexRight;
        DOM.zoneLeft.style.borderColor = hexLeft;
        DOM.zoneCenter.style.borderColor = hexCenter;
        DOM.zoneRight.style.borderColor = hexRight;
        renderVisualizer();
      }

      // Apply to Room Devices
      appState.devices.wizLeft.mode = 'rgb';
      appState.devices.wizLeft.color = hexLeft;
      appState.devices.desk.color = hexCenter;
      appState.devices.desk.effect = 'solid';
      appState.devices.wizRight.mode = 'rgb';
      appState.devices.wizRight.color = hexRight;

      const nowMs = performance.now();
      if (nowMs - lastAmbiDispatch >= 60) {
        lastAmbiDispatch = nowMs;
        dispatchStateChange({
          desk: { color: hexCenter, effect: 'solid' },
          wizLeft: { color: hexLeft, mode: 'rgb' },
          wizRight: { color: hexRight, mode: 'rgb' }
        });
      }
    }
  }

  appState.ambilight.processFrame = processAmbilightFrame;

  function loop() {
    if (!appState.ambilight.active) return;
    if (document.visibilityState === 'visible') {
      processAmbilightFrame(true);
    }
    appState.ambilight.rafId = requestAnimationFrame(loop);
  }

  appState.ambilight.rafId = requestAnimationFrame(loop);
  updateSyncWorkerState();
  showToast('Ambilight Screen Sync started (30 FPS)');
}

function stopAmbilight() {
  appState.ambilight.active = false;
  appState.ambilight.processFrame = null;
  if (appState.ambilight.rafId) cancelAnimationFrame(appState.ambilight.rafId);
  if (appState.ambilight.stream) {
    appState.ambilight.stream.getTracks().forEach(t => t.stop());
    appState.ambilight.stream = null;
  }
  DOM.ambilightVideo.srcObject = null;
  DOM.ambilightPlaceholder.classList.remove('hidden');
  DOM.ambilightBtnLabel.textContent = 'Start Screen Sync';
  DOM.btnToggleAmbilight.classList.remove('active');
  updateSyncWorkerState();
  renderAll();
  showToast('Ambilight Screen Sync stopped.');
}

function sampleRegionColor(ctx, x, y, w, h) {
  x = Math.max(0, Math.floor(x));
  y = Math.max(0, Math.floor(y));
  w = Math.max(1, Math.floor(w));
  h = Math.max(1, Math.floor(h));
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
    return;
  }

  // Pre-flight check for Secure Context & mediaDevices support
  const isSecure = checkIsSecureContext();
  const hasMediaDevices = typeof navigator !== 'undefined' && !!navigator.mediaDevices && !!navigator.mediaDevices.getUserMedia;
  if (!isSecure || !hasMediaDevices) {
    openSecurityGuideModal();
    showToast('Secure Context required: Chrome restricts Microphone on local HTTP.');
    return;
  }

  try {
    const stream = await navigator.mediaDevices.getUserMedia({ audio: true, video: false });
    startSoundSync(stream);
  } catch (err) {
    if (err && err.name === 'NotAllowedError') {
      showToast('Microphone access permission was cancelled.');
    } else {
      openSecurityGuideModal();
      showToast('Microphone access was restricted by browser.');
    }
  }
}

function startSoundSync(stream) {
  appState.soundsync.active = true;
  appState.soundsync.micStream = stream;
  appState.soundsync.savedState = {
    desk: { brightness: appState.devices.desk.brightness, color: appState.devices.desk.color },
    wizLeft: { brightness: appState.devices.wizLeft.brightness, color: appState.devices.wizLeft.color, mode: appState.devices.wizLeft.mode },
    wizRight: { brightness: appState.devices.wizRight.brightness, color: appState.devices.wizRight.color, mode: appState.devices.wizRight.mode }
  };
  DOM.soundsyncBtnLabel.textContent = 'Deactivate Audio Reactive';
  DOM.btnToggleSoundSync.classList.add('active');

  const AudioCtx = window.AudioContext || window.webkitAudioContext;
  const audioCtx = new AudioCtx();
  if (audioCtx.state === 'suspended') {
    audioCtx.resume();
  }
  const analyser = audioCtx.createAnalyser();
  analyser.fftSize = 128;
  analyser.smoothingTimeConstant = 0.35; // Fast transient response for crisp beat tracking

  const source = audioCtx.createMediaStreamSource(stream);
  source.connect(analyser);

  appState.soundsync.audioCtx = audioCtx;
  appState.soundsync.analyser = analyser;

  const canvas = DOM.soundsyncCanvas;
  const ctx = canvas ? canvas.getContext('2d') : null;
  const bufferLength = analyser.frequencyBinCount;
  const dataArray = new Uint8Array(bufferLength);
  let lastSoundSyncDispatch = 0;

  // Dynamic Beat Tracking & AGC State
  let bassPulse = 0;
  let midsPulse = 0;
  let dynamicNoiseFloor = 15;
  let dynamicPeak = 130;
  let lastRaveHue = 0;

  function processSoundSyncFrame(updateUi) {
    if (!appState.soundsync.active) return;
    analyser.getByteFrequencyData(dataArray);

    const sens = appState.soundsync.sensitivity / 100;
    const mode = appState.soundsync.mode;

    // Sub-bass (bins 0-1), Low-mids (bins 2-4), Mids (bins 5-13), Highs (bins 14-28)
    const avgSubBass = (dataArray[0] * 1.5 + dataArray[1] * 1.2) / 2.7;
    let midsSum = 0;
    for (let i = 4; i < 14; i++) midsSum += dataArray[i];
    const avgMids = midsSum / 10;
    let highsSum = 0;
    for (let i = 14; i < 28; i++) highsSum += dataArray[i];
    const avgHighs = highsSum / 14;

    // Dynamic Noise Floor & Peak Adaptation (AGC)
    if (avgSubBass < dynamicNoiseFloor) {
      dynamicNoiseFloor = dynamicNoiseFloor * 0.95 + avgSubBass * 0.05;
    } else {
      dynamicNoiseFloor = dynamicNoiseFloor * 0.999 + avgSubBass * 0.001;
    }
    if (avgSubBass > dynamicPeak) {
      dynamicPeak = avgSubBass;
    } else {
      dynamicPeak = Math.max(70, dynamicPeak * 0.996);
    }

    const cleanBass = Math.max(0, avgSubBass - dynamicNoiseFloor);
    const normBass = Math.min(1.0, cleanBass / (dynamicPeak - dynamicNoiseFloor || 1.0));

    // Attack / Exponential Decay Machine (signature snappy beat response)
    if (normBass > bassPulse) {
      bassPulse = normBass; // Instant punch attack!
    } else {
      bassPulse *= 0.86; // Exponential beat decay (~130ms half-life)
    }

    const cleanMids = Math.max(0, avgMids - 10);
    const normMids = Math.min(1.0, cleanMids / 110);
    if (normMids > midsPulse) {
      midsPulse = normMids;
    } else {
      midsPulse *= 0.88;
    }

    // Audio reactive calculations based on active mode
    if (mode === 'spectrum' || mode === 'equalizer') {
      const totalEnergy = (bassPulse * 0.55 + midsPulse * 0.45) * sens;
      const hueShift = Math.round((240 - normBass * 180 + avgHighs * 0.6) + 360) % 360;
      const color = hslToHex(hueShift, 100, 50);
      const specBright = Math.min(100, Math.round(20 + 80 * totalEnergy));
      appState.devices.desk.brightness = specBright;
      appState.devices.desk.color = color;
      appState.devices.wizLeft.mode = 'rgb';
      appState.devices.wizLeft.color = color;
      appState.devices.wizLeft.brightness = specBright;
      appState.devices.wizRight.mode = 'rgb';
      appState.devices.wizRight.color = color;
      appState.devices.wizRight.brightness = specBright;
    } else if (mode === 'ambient') {
      const smoothVol = (bassPulse * 0.35 + midsPulse * 0.65) * sens;
      const ambientBright = Math.max(15, Math.min(95, Math.round(18 + 72 * smoothVol)));
      appState.devices.desk.brightness = ambientBright;
      appState.devices.wizLeft.brightness = ambientBright;
      appState.devices.wizRight.brightness = ambientBright;
    } else if (mode === 'rave') {
      const raveBright = Math.min(100, Math.round((25 + 75 * Math.max(bassPulse, midsPulse)) * sens));
      if (normBass > 0.65) {
        lastRaveHue = (lastRaveHue + 120) % 360;
      }
      const color = hslToHex(lastRaveHue, 100, 50);
      appState.devices.desk.brightness = Math.max(20, raveBright);
      appState.devices.desk.color = color;
      appState.devices.wizLeft.brightness = Math.max(20, raveBright);
      appState.devices.wizLeft.color = color;
      appState.devices.wizRight.brightness = Math.max(20, raveBright);
      appState.devices.wizRight.color = color;
    } else { // default 'pulse' - Snappy dynamic bass punch with kick decay
      const targetDeskBright = Math.min(100, Math.round((15 + 85 * bassPulse) * sens));
      const targetWizBright = Math.min(100, Math.round((10 + 90 * bassPulse) * sens));
      const pulseHue = Math.round(180 + midsPulse * 140) % 360;
      const pulseColor = hslToHex(pulseHue, 100, 50);
      appState.devices.desk.brightness = targetDeskBright;
      appState.devices.desk.color = pulseColor;
      appState.devices.wizLeft.brightness = targetWizBright;
      appState.devices.wizRight.brightness = targetWizBright;
    }

    if (updateUi && canvas && ctx) {
      ctx.fillStyle = '#090c14';
      ctx.fillRect(0, 0, canvas.width, canvas.height);
      const barWidth = (canvas.width / bufferLength) * 2;
      let x = 0;
      for (let i = 0; i < bufferLength; i++) {
        const barHeight = (dataArray[i] / 255) * canvas.height;
        const hue = (i / bufferLength) * 280 + 160;
        ctx.fillStyle = `hsl(${hue}, 100%, 55%)`;
        ctx.fillRect(x, canvas.height - barHeight, barWidth - 2, barHeight);
        x += barWidth;
      }
      renderVisualizer();
    }

    const nowAudio = performance.now();
    if (nowAudio - lastSoundSyncDispatch >= 65) {
      lastSoundSyncDispatch = nowAudio;
      dispatchStateChange({
        desk: { brightness: appState.devices.desk.brightness, color: appState.devices.desk.color },
        wizLeft: { brightness: appState.devices.wizLeft.brightness, color: appState.devices.wizLeft.color, mode: appState.devices.wizLeft.mode },
        wizRight: { brightness: appState.devices.wizRight.brightness, color: appState.devices.wizRight.color, mode: appState.devices.wizRight.mode }
      });
    }
  }

  appState.soundsync.processFrame = processSoundSyncFrame;

  function draw() {
    if (!appState.soundsync.active) return;
    if (document.visibilityState === 'visible') {
      processSoundSyncFrame(true);
    }
    appState.soundsync.rafId = requestAnimationFrame(draw);
  }

  appState.soundsync.rafId = requestAnimationFrame(draw);
  updateSyncWorkerState();
  showToast('SoundSync active: Listening to audio stream');
}

function stopSoundSync() {
  appState.soundsync.active = false;
  appState.soundsync.processFrame = null;
  if (appState.soundsync.rafId) cancelAnimationFrame(appState.soundsync.rafId);
  if (appState.soundsync.micStream) {
    appState.soundsync.micStream.getTracks().forEach(t => t.stop());
    appState.soundsync.micStream = null;
  }
  if (appState.soundsync.audioCtx) {
    appState.soundsync.audioCtx.close();
    appState.soundsync.audioCtx = null;
  }
  if (appState.soundsync.savedState) {
    const s = appState.soundsync.savedState;
    appState.devices.desk.brightness = s.desk.brightness;
    appState.devices.desk.color = s.desk.color;
    appState.devices.wizLeft.brightness = s.wizLeft.brightness;
    appState.devices.wizLeft.color = s.wizLeft.color;
    appState.devices.wizLeft.mode = s.wizLeft.mode;
    appState.devices.wizRight.brightness = s.wizRight.brightness;
    appState.devices.wizRight.color = s.wizRight.color;
    appState.devices.wizRight.mode = s.wizRight.mode;
    delete appState.soundsync.savedState;
    dispatchStateChange({
      desk: { brightness: appState.devices.desk.brightness, color: appState.devices.desk.color },
      wizLeft: { brightness: appState.devices.wizLeft.brightness, color: appState.devices.wizLeft.color, mode: appState.devices.wizLeft.mode },
      wizRight: { brightness: appState.devices.wizRight.brightness, color: appState.devices.wizRight.color, mode: appState.devices.wizRight.mode }
    });
  }
  DOM.soundsyncBtnLabel.textContent = 'Activate Microphone / Audio';
  DOM.btnToggleSoundSync.classList.remove('active');
  updateSyncWorkerState();
  renderAll();
  showToast('SoundSync deactivated.');
}

// ============================================================================
// 8. FEATURE SUITE 4: GRADIENT LAB (Dynamic Fluid Waves, Soap Bubble Iridescence & WiZ Ambient Integration)
// ============================================================================

function hexToRgb(hex) {
  if (!hex || hex.length < 7) return { r: 255, g: 255, b: 255 };
  return {
    r: parseInt(hex.slice(1, 3), 16),
    g: parseInt(hex.slice(3, 5), 16),
    b: parseInt(hex.slice(5, 7), 16)
  };
}

function interpolate4Stops(stops, t) {
  const clampedT = Math.max(0, Math.min(1, t));
  if (clampedT <= 0.333333) {
    const f = clampedT / 0.333333;
    return {
      r: stops[0].r + (stops[1].r - stops[0].r) * f,
      g: stops[0].g + (stops[1].g - stops[0].g) * f,
      b: stops[0].b + (stops[1].b - stops[0].b) * f
    };
  } else if (clampedT <= 0.666667) {
    const f = (clampedT - 0.333333) / 0.333334;
    return {
      r: stops[1].r + (stops[2].r - stops[1].r) * f,
      g: stops[1].g + (stops[2].g - stops[1].g) * f,
      b: stops[1].b + (stops[2].b - stops[1].b) * f
    };
  } else {
    const f = (clampedT - 0.666667) / 0.333333;
    return {
      r: stops[2].r + (stops[3].r - stops[2].r) * f,
      g: stops[2].g + (stops[3].g - stops[2].g) * f,
      b: stops[2].b + (stops[3].b - stops[2].b) * f
    };
  }
}

function updateGradientPreviewBar() {
  const c1 = DOM.gradColor1 ? DOM.gradColor1.value : '#a5f3fc';
  const c2 = DOM.gradColor2 ? DOM.gradColor2.value : '#f472b6';
  const c3 = DOM.gradColor3 ? DOM.gradColor3.value : '#c084fc';
  const c4 = DOM.gradColor4 ? DOM.gradColor4.value : '#fef08a';

  if (DOM.gradientPreviewBar) {
    DOM.gradientPreviewBar.style.background = `linear-gradient(90deg, ${c1} 0%, ${c2} 33%, ${c3} 67%, ${c4} 100%)`;
  }
  appState.gradient.stops = [c1, c2, c3, c4];
}

function sampleFluidPhysicsColor(motion, normX, phase, turbNorm, rgbStops) {
  if (motion === 'soap-bubble') {
    // 1. Real-world thin-film optical interference: thickness variation splits chromatic spectra
    const wave = Math.sin(normX * 6.28 * 2 - phase) * 0.5 + Math.sin(normX * 6.28 * 4.5 + phase * 0.8) * turbNorm * 0.4;
    const sampleT = (normX + wave * 0.25 + 10) % 1;
    const baseColor = interpolate4Stops(rgbStops, sampleT);
    const r = Math.min(255, Math.max(0, baseColor.r + Math.cos(wave * 3 * Math.PI) * 45 * turbNorm));
    const g = Math.min(255, Math.max(0, baseColor.g + Math.cos(wave * 3 * Math.PI - 2.09) * 45 * turbNorm));
    const b = Math.min(255, Math.max(0, baseColor.b + Math.cos(wave * 3 * Math.PI - 4.18) * 45 * turbNorm));
    return { r, g, b };
  } else if (motion === 'wave') {
    // 2. Directional Navier-Stokes advection fluid wave displacement
    const wave = Math.sin(normX * 6.28 * 1.8 - phase) * 0.15 * (0.5 + turbNorm);
    const sampleT = (normX - (phase * 0.2) + wave + 10) % 1;
    return interpolate4Stops(rgbStops, sampleT);
  } else if (motion === 'caustics') {
    // 3. Underwater Caustics: Light ray convergence through oscillating surface ripples
    const w1 = Math.sin(normX * 18 - phase * 1.8);
    const w2 = Math.sin(normX * 28 + phase * 1.3);
    const curvature = Math.cos(normX * 18 - phase * 1.8) * 18 + Math.cos(normX * 28 + phase * 1.3) * 28;
    const caustic = Math.max(0, (1.0 / Math.abs(1.0 - 0.016 * turbNorm * curvature)) - 0.75);
    const sparkle = Math.min(180, caustic * 38 * (0.4 + turbNorm * 0.8));
    const sampleT = (normX - phase * 0.08 + w1 * 0.06 + 10) % 1;
    const baseColor = interpolate4Stops(rgbStops, sampleT);
    const r = Math.min(255, Math.max(0, baseColor.r + sparkle * 0.75));
    const g = Math.min(255, Math.max(0, baseColor.g + sparkle * 0.95));
    const b = Math.min(255, Math.max(0, baseColor.b + sparkle * 1.15));
    return { r, g, b };
  } else if (motion === 'soliton') {
    // 4. Soliton Wave Packets: Non-dispersive solitary waves colliding coherently
    const p1 = (phase * 0.18) % 1.0;
    const p2 = (1.0 - (phase * 0.24) % 1.0);
    let d1 = Math.abs(normX - p1);
    if (d1 > 0.5) d1 = 1.0 - d1;
    let d2 = Math.abs(normX - p2);
    if (d2 > 0.5) d2 = 1.0 - d2;
    const e1 = 1.0 / Math.pow(Math.cosh(d1 * 24), 2);
    const e2 = 1.0 / Math.pow(Math.cosh(d2 * 28), 2);
    const totalEnergy = e1 + e2;
    const sampleT = (normX + (e1 - e2) * 0.25 * turbNorm + 10) % 1;
    const baseColor = interpolate4Stops(rgbStops, sampleT);
    const boost = totalEnergy * 160 * (0.5 + turbNorm * 0.7);
    const r = Math.min(255, Math.max(0, baseColor.r + boost * 0.9));
    const g = Math.min(255, Math.max(0, baseColor.g + boost * 0.6));
    const b = Math.min(255, Math.max(0, baseColor.b + boost * 1.2));
    return { r, g, b };
  } else if (motion === 'thermal-plume') {
    // 5. Thermal Convection Plume: Fractal Brownian heat convection & rising plasma filaments
    const h1 = Math.sin(normX * 10 - phase * 1.6);
    const h2 = Math.sin(normX * 22 + phase * 2.4) * 0.5;
    const h3 = Math.sin(normX * 42 - phase * 4.2) * 0.25;
    const fbm = (h1 + h2 + h3) / 1.75;
    const sampleT = Math.max(0, Math.min(1, normX + fbm * 0.22 * turbNorm));
    const baseColor = interpolate4Stops(rgbStops, sampleT);
    const thermalGlow = (fbm + 1.0) * 36 * turbNorm;
    const r = Math.min(255, Math.max(0, baseColor.r + thermalGlow * 1.2));
    const g = Math.min(255, Math.max(0, baseColor.g + thermalGlow * 0.6));
    const b = Math.min(255, Math.max(0, baseColor.b + thermalGlow * 0.1));
    return { r, g, b };
  } else if (motion === 'marangoni') {
    // 6. Marangoni Surface Vortices: Surface-tension gradient driven cellular convection rolls
    const cell = Math.sin(normX * Math.PI * 6);
    const roll = cell * Math.cos(phase * 1.4);
    const shear = Math.sin(normX * Math.PI * 12 + phase * 0.9) * 0.1 * turbNorm;
    const sampleT = (normX + roll * 0.16 + shear + 10) % 1;
    const baseColor = interpolate4Stops(rgbStops, sampleT);
    const vortexGlow = Math.abs(cell) * 32 * turbNorm;
    const r = Math.min(255, Math.max(0, baseColor.r + vortexGlow * 0.5));
    const g = Math.min(255, Math.max(0, baseColor.g + vortexGlow * 1.1));
    const b = Math.min(255, Math.max(0, baseColor.b + vortexGlow * 0.9));
    return { r, g, b };
  } else if (motion === 'breathe') {
    // 7. Harmonic Resonant Breathing: Sinusoidal diaphragm expansion and contraction
    const pulse = Math.sin(phase * 1.5) * 0.35;
    const sampleT = Math.max(0, Math.min(1, 0.5 + (normX - 0.5) * (1 + pulse)));
    return interpolate4Stops(rgbStops, sampleT);
  } else {
    // 8. Prismatic Shimmer: High-frequency optical dispersion & crystal scintillation
    const sparkle = Math.sin(normX * 60 + phase * 8) * turbNorm * 35;
    const sampleT = (normX - (phase * 0.15) + 10) % 1;
    const color = interpolate4Stops(rgbStops, sampleT);
    const r = Math.min(255, Math.max(0, color.r + sparkle));
    const g = Math.min(255, Math.max(0, color.g + sparkle * 0.8));
    const b = Math.min(255, Math.max(0, color.b + sparkle * 1.2));
    return { r, g, b };
  }
}

function startGradientFluidSimulation() {
  if (!DOM.gradientFluidCanvas) return;
  const canvas = DOM.gradientFluidCanvas;
  const ctx = canvas.getContext('2d');
  if (!ctx) return;

  let lastTime = performance.now();

  function renderLoop(now) {
    const dt = Math.min((now - lastTime) / 1000, 0.1);
    lastTime = now;

    const speedNorm = (appState.gradient.speed || 45) / 100;
    const turbNorm = (appState.gradient.turbulence || 50) / 100;
    appState.gradient.phase = (appState.gradient.phase + dt * speedNorm * 2.2) % (Math.PI * 20);

    const width = canvas.width;
    const height = canvas.height;
    ctx.clearRect(0, 0, width, height);

    const stops = appState.gradient.stops || ['#a5f3fc', '#f472b6', '#c084fc', '#fef08a'];
    const rgbStops = stops.map(hexToRgb);
    const motion = appState.gradient.motionMode || 'soap-bubble';
    const phase = appState.gradient.phase;

    const ribbonHeight = height * 0.62;
    const colStep = 4;

    // 1. Draw ultra-smooth fluid plasma wave ribbon
    for (let x = 0; x < width; x += colStep) {
      const normX = x / width;
      const c = sampleFluidPhysicsColor(motion, normX, phase, turbNorm, rgbStops);
      ctx.fillStyle = `rgb(${Math.round(c.r)}, ${Math.round(c.g)}, ${Math.round(c.b)})`;
      ctx.fillRect(x, 0, colStep, ribbonHeight);
    }

    // 2. Subtle glass divider
    ctx.fillStyle = 'rgba(0, 0, 0, 0.5)';
    ctx.fillRect(0, ribbonHeight, width, 1);

    // 3. 60 discrete WS2812B LED beads (1:1 physical desk fixture visualizer)
    const numLeds = 60;
    const ledStep = width / numLeds;
    const ledY = ribbonHeight + (height - ribbonHeight) / 2;

    for (let i = 0; i < numLeds; i++) {
      const ledX = (i + 0.5) * ledStep;
      const normX = i / (numLeds - 1);
      const c = sampleFluidPhysicsColor(motion, normX, phase, turbNorm, rgbStops);
      ctx.fillStyle = `rgb(${Math.round(c.r)}, ${Math.round(c.g)}, ${Math.round(c.b)})`;
      ctx.beginPath();
      ctx.arc(ledX, ledY, 2.5, 0, Math.PI * 2);
      ctx.fill();
    }

    appState.gradient.animFrameId = requestAnimationFrame(renderLoop);
  }

  if (appState.gradient.animFrameId) {
    cancelAnimationFrame(appState.gradient.animFrameId);
  }
  appState.gradient.animFrameId = requestAnimationFrame(renderLoop);
}

function applyCustomGradientToDesk() {
  const [c1, c2, c3, c4] = appState.gradient.stops;

  // 1. Desk WS2812B Strip configuration
  appState.devices.desk.color = c2;
  appState.devices.desk.effect = appState.gradient.motionMode;
  appState.devices.desk.intensity = appState.gradient.turbulence;
  appState.devices.desk.gradientStops = [c1, c2, c3, c4];

  const payload = {
    desk: {
      color: c2,
      effect: appState.gradient.motionMode,
      speed: appState.gradient.speed,
      intensity: appState.gradient.turbulence,
      turbulence: appState.gradient.turbulence,
      gradientStops: [c1, c2, c3, c4]
    }
  };

  // 2. Corner WiZ Bulbs Integration Architecture
  const syncMode = appState.gradient.bulbSyncMode || 'spatial-sync';

  if (syncMode === 'spatial-sync') {
    // Spatial Sync: Left bulb mirrors left edge (Stop 1), Right bulb mirrors right edge (Stop 4)
    appState.devices.wizLeft.mode = 'rgb';
    appState.devices.wizLeft.color = c1;
    appState.devices.wizRight.mode = 'rgb';
    appState.devices.wizRight.color = c4;

    payload.wizLeft = { mode: 'rgb', color: c1 };
    payload.wizRight = { mode: 'rgb', color: c4 };

    showToast(`Streaming ${appState.gradient.motionMode} gradient with spatial corner sync (L: ${c1}, R: ${c4}).`);
  } else if (syncMode === 'warm-white') {
    // Architectural Gallery Contrast: WiZ bulbs cast pure warm white CCT
    const kelvin = appState.gradient.warmWhiteKelvin || 2400;
    appState.devices.wizLeft.mode = 'cct';
    appState.devices.wizLeft.kelvin = kelvin;
    appState.devices.wizRight.mode = 'cct';
    appState.devices.wizRight.kelvin = kelvin;

    payload.wizLeft = { mode: 'cct', kelvin: kelvin };
    payload.wizRight = { mode: 'cct', kelvin: kelvin };

    showToast(`Streaming gradient to desk strip with ${kelvin}K gallery warm white ambient.`);
  } else if (syncMode === 'custom') {
    // Custom Ambient Bulbs: User picked custom wash
    const customColor = appState.gradient.bulbCustomColor || '#ffa757';
    appState.devices.wizLeft.mode = 'rgb';
    appState.devices.wizLeft.color = customColor;
    appState.devices.wizRight.mode = 'rgb';
    appState.devices.wizRight.color = customColor;

    payload.wizLeft = { mode: 'rgb', color: customColor };
    payload.wizRight = { mode: 'rgb', color: customColor };

    showToast(`Streaming gradient to desk strip with custom corner ambient wash (${customColor}).`);
  }

  renderAll();
  dispatchStateChange(payload);
}

// ============================================================================
// 9. FEATURE SUITE 5: FOCUS & REST RHYTHM (Pomodoro Lighting Engine)
// ============================================================================

function togglePomodoro() {
  if (appState.pomodoro.active) {
    if (appState.pomodoro.timerInterval) {
      clearInterval(appState.pomodoro.timerInterval);
      appState.pomodoro.timerInterval = null;
    }
    appState.pomodoro.active = false;
    DOM.btnTogglePomodoro.textContent = appState.pomodoro.phase === 'focus' ? 'Resume Focus Sprint' : 'Resume Break Interval';
    DOM.btnTogglePomodoro.classList.remove('active');
    if (DOM.pomoStatusLabel) DOM.pomoStatusLabel.textContent = 'PAUSED';
    showToast('Focus Sprint paused.');
  } else {
    appState.pomodoro.active = true;
    DOM.btnTogglePomodoro.textContent = appState.pomodoro.phase === 'focus' ? 'Pause Focus Sprint' : 'Pause Break Interval';
    DOM.btnTogglePomodoro.classList.add('active');

    // Only start fresh phase if at initial state or reset
    if (appState.pomodoro.secondsRemaining === 25 * 60 && appState.pomodoro.phase === 'focus') {
      startPomodoroPhase('focus');
    } else {
      if (DOM.pomoStatusLabel) {
        DOM.pomoStatusLabel.textContent = appState.pomodoro.phase === 'focus' ? 'FOCUS SPRINT • 4500K' : 'REST INTERVAL • 2400K';
      }
    }

    if (appState.pomodoro.timerInterval) clearInterval(appState.pomodoro.timerInterval);
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

function playChime(freq = 880, duration = 0.35) {
  try {
    const AudioCtx = window.AudioContext || window.webkitAudioContext;
    const ctx = new AudioCtx();
    const osc = ctx.createOscillator();
    const gain = ctx.createGain();
    osc.type = 'sine';
    osc.frequency.setValueAtTime(freq, ctx.currentTime);
    gain.gain.setValueAtTime(0.18, ctx.currentTime);
    gain.gain.exponentialRampToValueAtTime(0.001, ctx.currentTime + duration);
    osc.connect(gain);
    gain.connect(ctx.destination);
    osc.start();
    osc.stop(ctx.currentTime + duration);
  } catch (e) {
    // Audio context may require prior interaction
  }
}

function startPomodoroPhase(phase) {
  appState.pomodoro.phase = phase;
  if (phase === 'focus') {
    appState.pomodoro.secondsRemaining = 25 * 60;
    if (DOM.pomoStatusLabel) {
      DOM.pomoStatusLabel.textContent = 'FOCUS SPRINT • 4500K';
      DOM.pomoStatusLabel.classList.remove('status-break');
      DOM.pomoStatusLabel.classList.add('status-focus');
    }
    if (DOM.pomoTimerRing) {
      DOM.pomoTimerRing.classList.remove('ring-break');
      DOM.pomoTimerRing.classList.add('ring-focus');
    }
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
    playChime(880, 0.4); // High focus chime
    showToast('Entering 25m Focus Phase: Anti-distraction bias lighting engaged.');
  } else {
    appState.pomodoro.secondsRemaining = 5 * 60;
    if (DOM.pomoStatusLabel) {
      DOM.pomoStatusLabel.textContent = 'REST INTERVAL • 2400K';
      DOM.pomoStatusLabel.classList.remove('status-focus');
      DOM.pomoStatusLabel.classList.add('status-break');
    }
    if (DOM.pomoTimerRing) {
      DOM.pomoTimerRing.classList.remove('ring-focus');
      DOM.pomoTimerRing.classList.add('ring-break');
    }
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
    playChime(523, 0.5); // Warm restorative chime
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

function resetPomodoro() {
  if (appState.pomodoro.timerInterval) {
    clearInterval(appState.pomodoro.timerInterval);
    appState.pomodoro.timerInterval = null;
  }
  appState.pomodoro.active = false;
  appState.pomodoro.phase = 'focus';
  appState.pomodoro.secondsRemaining = 25 * 60;
  if (DOM.btnTogglePomodoro) {
    DOM.btnTogglePomodoro.textContent = 'Start 25m Focus Sprint';
    DOM.btnTogglePomodoro.classList.remove('active');
  }
  if (DOM.pomoStatusLabel) {
    DOM.pomoStatusLabel.textContent = 'READY FOR SPRINT';
    DOM.pomoStatusLabel.classList.remove('status-break');
    DOM.pomoStatusLabel.classList.add('status-focus');
  }
  if (DOM.pomoTimerRing) {
    DOM.pomoTimerRing.classList.remove('ring-break');
    DOM.pomoTimerRing.classList.add('ring-focus');
  }
  if (DOM.pomoTimeLeft) DOM.pomoTimeLeft.textContent = '25:00';
  renderAll();
  showToast('Focus Interval reset.');
}

// ============================================================================
// 10. EVENT BINDING & INTERACTIVITY (With Haptic Tactile Feedback)
// ============================================================================

function triggerHaptic(type = 'light') {
  if (typeof navigator !== 'undefined' && 'vibrate' in navigator) {
    try {
      if (type === 'light') navigator.vibrate(10);
      else if (type === 'medium') navigator.vibrate(22);
      else if (type === 'heavy') navigator.vibrate(40);
      else if (type === 'double') navigator.vibrate([12, 35, 12]);
    } catch (_) {}
  }
}

function bindEvents() {
  // Navigation Tabs
  DOM.suiteTabs.forEach(tab => {
    tab.addEventListener('click', () => {
      triggerHaptic('light');
      const target = tab.dataset.tab;
      appState.activeTab = target;
      DOM.suiteTabs.forEach(t => t.classList.toggle('active', t === tab));
      DOM.tabViews.forEach(v => v.classList.toggle('active', v.id === `view-${target}`));
    });
  });

  // Transition Timing Speed
  DOM.selectTransitionSpeed.addEventListener('change', (e) => {
    triggerHaptic('light');
    appState.transitionDuration = parseInt(e.target.value, 10);
    localStorage.setItem('aura_transition_duration', appState.transitionDuration);
    document.documentElement.style.setProperty('--lighting-fade-speed', `${appState.transitionDuration / 1000}s`);
    showToast(`Fade transition set to ${e.target.options[e.target.selectedIndex].text}`);
    dispatchStateChange({ transitionDuration: appState.transitionDuration });
  });

  // Master Room Power (Both Top Bar and Master Controls Card)
  const toggleRoomPower = () => {
    triggerHaptic('medium');
    appState.isMasterOn = !appState.isMasterOn;
    renderAll();
    dispatchStateChange({ isMasterOn: appState.isMasterOn });
    showToast(appState.isMasterOn ? 'Room lighting turned ON' : 'Room lighting turned OFF');
  };

  if (DOM.btnMasterPower) {
    DOM.btnMasterPower.addEventListener('click', toggleRoomPower);
  }
  if (DOM.btnRoomPowerCard) {
    DOM.btnRoomPowerCard.addEventListener('click', toggleRoomPower);
  }

  // Master Brightness Slider
  DOM.sliderMasterBrightness.addEventListener('input', (e) => {
    appState.masterBrightness = parseInt(e.target.value, 10);
    renderAll();
  });
  DOM.sliderMasterBrightness.addEventListener('change', () => {
    triggerHaptic('light');
    dispatchStateChange({ masterBrightness: appState.masterBrightness });
  });

  // Contrast Slider
  DOM.sliderMasterContrast.addEventListener('input', (e) => {
    appState.contrastRatio = parseInt(e.target.value, 10);
    renderAll();
  });
  DOM.sliderMasterContrast.addEventListener('change', () => {
    triggerHaptic('light');
    dispatchStateChange({ contrastRatio: appState.contrastRatio });
  });

  // Presets
  document.querySelectorAll('.scene-card').forEach(card => {
    card.addEventListener('click', () => {
      triggerHaptic('light');
      applyPreset(card.dataset.preset);
    });
  });

  // Desk Controls
  DOM.toggleDeskPower.addEventListener('change', (e) => {
    triggerHaptic('light');
    appState.devices.desk.power = e.target.checked;
    renderAll();
    dispatchStateChange({ desk: { power: appState.devices.desk.power } });
  });

  DOM.sliderDeskBrightness.addEventListener('input', (e) => {
    appState.devices.desk.brightness = parseInt(e.target.value, 10);
    renderAll();
  });
  DOM.sliderDeskBrightness.addEventListener('change', () => {
    triggerHaptic('light');
    dispatchStateChange({ desk: { brightness: appState.devices.desk.brightness } });
  });

  DOM.pickerDeskColor.addEventListener('input', (e) => {
    appState.devices.desk.color = e.target.value;
    renderAll();
  });
  DOM.pickerDeskColor.addEventListener('change', () => {
    triggerHaptic('light');
    dispatchStateChange({ desk: { color: appState.devices.desk.color } });
  });

  if (DOM.sliderDeskHue) {
    DOM.sliderDeskHue.addEventListener('input', (e) => {
      const hue = parseInt(e.target.value, 10);
      const hex = hslToHex(hue, 100, 50);
      appState.devices.desk.color = hex;
      renderAll();
    });
    DOM.sliderDeskHue.addEventListener('change', () => {
      triggerHaptic('light');
      dispatchStateChange({ desk: { color: appState.devices.desk.color } });
    });
  }

  document.querySelectorAll('#effects-selector .effect-chip').forEach(chip => {
    chip.addEventListener('click', () => {
      triggerHaptic('light');
      appState.devices.desk.effect = chip.dataset.effect;
      renderAll();
      dispatchStateChange({ desk: { effect: appState.devices.desk.effect } });
    });
  });

  DOM.deskSwatches.addEventListener('click', (e) => {
    const dot = e.target.closest('.swatch-dot');
    if (dot) {
      triggerHaptic('light');
      appState.devices.desk.color = dot.dataset.color;
      renderAll();
      dispatchStateChange({ desk: { color: appState.devices.desk.color } });
    }
  });

  DOM.sliderDeskSpeed.addEventListener('input', (e) => {
    appState.devices.desk.speed = parseInt(e.target.value, 10);
    renderAll();
  });
  DOM.sliderDeskSpeed.addEventListener('change', () => {
    triggerHaptic('light');
    dispatchStateChange({ desk: { speed: appState.devices.desk.speed } });
  });

  DOM.sliderDeskIntensity.addEventListener('input', (e) => {
    appState.devices.desk.intensity = parseInt(e.target.value, 10);
    renderAll();
  });
  DOM.sliderDeskIntensity.addEventListener('change', () => {
    triggerHaptic('light');
    dispatchStateChange({ desk: { intensity: appState.devices.desk.intensity } });
  });

  // WiZ Left Controls
  DOM.toggleWizLeftPower.addEventListener('change', (e) => {
    triggerHaptic('light');
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
  DOM.sliderWizLeftBrightness.addEventListener('change', () => {
    triggerHaptic('light');
    dispatchStateChange({
      wizLeft: { brightness: appState.devices.wizLeft.brightness },
      ...(appState.syncCorners ? { wizRight: { brightness: appState.devices.wizRight.brightness } } : {})
    });
  });

  DOM.pickerWizLeftColor.addEventListener('input', (e) => {
    appState.devices.wizLeft.color = e.target.value;
    appState.devices.wizLeft.mode = 'rgb';
    if (appState.syncCorners) {
      appState.devices.wizRight.color = e.target.value;
      appState.devices.wizRight.mode = 'rgb';
    }
    renderAll();
  });
  DOM.pickerWizLeftColor.addEventListener('change', () => {
    triggerHaptic('light');
    dispatchStateChange({
      wizLeft: { color: appState.devices.wizLeft.color, mode: 'rgb' },
      ...(appState.syncCorners ? { wizRight: { color: appState.devices.wizRight.color, mode: 'rgb' } } : {})
    });
  });

  if (DOM.sliderWizLeftHue) {
    DOM.sliderWizLeftHue.addEventListener('input', (e) => {
      const hue = parseInt(e.target.value, 10);
      const hex = hslToHex(hue, 100, 50);
      appState.devices.wizLeft.color = hex;
      appState.devices.wizLeft.mode = 'rgb';
      if (appState.syncCorners) {
        appState.devices.wizRight.color = hex;
        appState.devices.wizRight.mode = 'rgb';
      }
      renderAll();
    });
    DOM.sliderWizLeftHue.addEventListener('change', () => {
      triggerHaptic('light');
      dispatchStateChange({
        wizLeft: { color: appState.devices.wizLeft.color, mode: 'rgb' },
        ...(appState.syncCorners ? { wizRight: { color: appState.devices.wizRight.color, mode: 'rgb' } } : {})
      });
    });
  }

  DOM.sliderWizLeftKelvin.addEventListener('input', (e) => {
    appState.devices.wizLeft.kelvin = parseInt(e.target.value, 10);
    appState.devices.wizLeft.mode = 'cct';
    if (appState.syncCorners) {
      appState.devices.wizRight.kelvin = appState.devices.wizLeft.kelvin;
      appState.devices.wizRight.mode = 'cct';
    }
    renderAll();
  });
  DOM.sliderWizLeftKelvin.addEventListener('change', () => {
    triggerHaptic('light');
    dispatchStateChange({
      wizLeft: { kelvin: appState.devices.wizLeft.kelvin, mode: 'cct' },
      ...(appState.syncCorners ? { wizRight: { kelvin: appState.devices.wizRight.kelvin, mode: 'cct' } } : {})
    });
  });

  // WiZ Right Controls
  DOM.toggleWizRightPower.addEventListener('change', (e) => {
    triggerHaptic('light');
    appState.devices.wizRight.power = e.target.checked;
    if (appState.syncCorners) appState.devices.wizLeft.power = e.target.checked;
    renderAll();
    dispatchStateChange({
      wizRight: { power: appState.devices.wizRight.power },
      ...(appState.syncCorners ? { wizLeft: { power: appState.devices.wizLeft.power } } : {})
    });
  });

  DOM.sliderWizRightBrightness.addEventListener('input', (e) => {
    appState.devices.wizRight.brightness = parseInt(e.target.value, 10);
    if (appState.syncCorners) appState.devices.wizLeft.brightness = appState.devices.wizRight.brightness;
    renderAll();
  });
  DOM.sliderWizRightBrightness.addEventListener('change', () => {
    triggerHaptic('light');
    dispatchStateChange({
      wizRight: { brightness: appState.devices.wizRight.brightness },
      ...(appState.syncCorners ? { wizLeft: { brightness: appState.devices.wizLeft.brightness } } : {})
    });
  });

  DOM.pickerWizRightColor.addEventListener('input', (e) => {
    appState.devices.wizRight.color = e.target.value;
    appState.devices.wizRight.mode = 'rgb';
    if (appState.syncCorners) {
      appState.devices.wizLeft.color = e.target.value;
      appState.devices.wizLeft.mode = 'rgb';
    }
    renderAll();
  });
  DOM.pickerWizRightColor.addEventListener('change', () => {
    triggerHaptic('light');
    dispatchStateChange({
      wizRight: { color: appState.devices.wizRight.color, mode: 'rgb' },
      ...(appState.syncCorners ? { wizLeft: { color: appState.devices.wizLeft.color, mode: 'rgb' } } : {})
    });
  });

  if (DOM.sliderWizRightHue) {
    DOM.sliderWizRightHue.addEventListener('input', (e) => {
      const hue = parseInt(e.target.value, 10);
      const hex = hslToHex(hue, 100, 50);
      appState.devices.wizRight.color = hex;
      appState.devices.wizRight.mode = 'rgb';
      if (appState.syncCorners) {
        appState.devices.wizLeft.color = hex;
        appState.devices.wizLeft.mode = 'rgb';
      }
      renderAll();
    });
    DOM.sliderWizRightHue.addEventListener('change', () => {
      triggerHaptic('light');
      dispatchStateChange({
        wizRight: { color: appState.devices.wizRight.color, mode: 'rgb' },
        ...(appState.syncCorners ? { wizLeft: { color: appState.devices.wizLeft.color, mode: 'rgb' } } : {})
      });
    });
  }

  DOM.sliderWizRightKelvin.addEventListener('input', (e) => {
    appState.devices.wizRight.kelvin = parseInt(e.target.value, 10);
    appState.devices.wizRight.mode = 'cct';
    if (appState.syncCorners) {
      appState.devices.wizLeft.kelvin = appState.devices.wizRight.kelvin;
      appState.devices.wizLeft.mode = 'cct';
    }
    renderAll();
  });
  DOM.sliderWizRightKelvin.addEventListener('change', () => {
    triggerHaptic('light');
    dispatchStateChange({
      wizRight: { kelvin: appState.devices.wizRight.kelvin, mode: 'cct' },
      ...(appState.syncCorners ? { wizLeft: { kelvin: appState.devices.wizLeft.kelvin, mode: 'cct' } } : {})
    });
  });

  // Mode Tabs (RGB vs White Kelvin)
  document.querySelectorAll('.mode-tab').forEach(tab => {
    tab.addEventListener('click', () => {
      triggerHaptic('light');
      const dev = tab.dataset.device;
      const mode = tab.dataset.mode;
      if (dev === 'wiz-left') {
        appState.devices.wizLeft.mode = mode;
        if (appState.syncCorners) appState.devices.wizRight.mode = mode;
      } else {
        appState.devices.wizRight.mode = mode;
        if (appState.syncCorners) appState.devices.wizLeft.mode = mode;
      }
      renderAll();
      dispatchStateChange({
        wizLeft: {
          mode: appState.devices.wizLeft.mode,
          ...(appState.devices.wizLeft.mode === 'cct' ? { kelvin: appState.devices.wizLeft.kelvin } : { color: appState.devices.wizLeft.color })
        },
        wizRight: {
          mode: appState.devices.wizRight.mode,
          ...(appState.devices.wizRight.mode === 'cct' ? { kelvin: appState.devices.wizRight.kelvin } : { color: appState.devices.wizRight.color })
        }
      });
    });
  });

  DOM.btnSyncCorners.addEventListener('click', () => {
    triggerHaptic('medium');
    appState.syncCorners = !appState.syncCorners;
    if (appState.syncCorners) {
      Object.assign(appState.devices.wizRight, JSON.parse(JSON.stringify(appState.devices.wizLeft)));
      showToast('Corner bulbs linked. Right mirrors Left.');
    } else {
      showToast('Corner bulbs unlinked. Independent control active.');
    }
    renderAll();
    dispatchStateChange({
      syncCorners: appState.syncCorners,
      ...(appState.syncCorners ? { wizRight: appState.devices.wizRight } : {})
    });
  });

  // Circadian Controls
  DOM.toggleCircadianAuto.addEventListener('change', (e) => {
    triggerHaptic('light');
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
    const { targetKelvin, targetLux } = calculateCircadianSolar(appState.circadian.simulatedMinutes);
    appState.masterBrightness = targetLux;
    appState.devices.wizLeft.mode = 'cct';
    appState.devices.wizLeft.kelvin = targetKelvin;
    appState.devices.wizRight.mode = 'cct';
    appState.devices.wizRight.kelvin = targetKelvin;
    appState.devices.desk.color = kelvinToRgbHex(targetKelvin);
    appState.devices.desk.effect = 'breathe';
    renderAll();
  });
  DOM.sliderCircadianTime.addEventListener('change', () => {
    triggerHaptic('light');
    applyCircadianToRoom();
  });

  DOM.btnCircadianNow.addEventListener('click', () => {
    triggerHaptic('light');
    const now = new Date();
    appState.circadian.simulatedMinutes = now.getHours() * 60 + now.getMinutes();
    DOM.sliderCircadianTime.value = appState.circadian.simulatedMinutes;
    applyCircadianToRoom();
    showToast('Synchronized to current solar clock.');
  });

  document.querySelectorAll('.phase-card').forEach(btn => {
    btn.addEventListener('click', () => {
      triggerHaptic('light');
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
  DOM.btnToggleAmbilight.addEventListener('click', () => {
    triggerHaptic('medium');
    toggleAmbilight();
  });
  DOM.sliderAmbiSmoothing.addEventListener('input', (e) => {
    appState.ambilight.smoothing = parseInt(e.target.value, 10) / 100;
    DOM.valAmbiSmoothing.textContent = `${e.target.value}% (Cinematic)`;
  });
  DOM.sliderAmbiBoost.addEventListener('input', (e) => {
    appState.ambilight.saturationBoost = parseInt(e.target.value, 10) / 10;
    DOM.valAmbiBoost.textContent = `${appState.ambilight.saturationBoost}× Vibrant`;
  });
  DOM.sliderAmbiBlackfloor.addEventListener('input', (e) => {
    appState.ambilight.blackFloor = parseInt(e.target.value, 10);
    DOM.valAmbiBlackfloor.textContent = `${e.target.value}% Brightness`;
  });

  // SoundSync
  DOM.btnToggleSoundSync.addEventListener('click', () => {
    triggerHaptic('medium');
    toggleSoundSync();
  });
  DOM.sliderAudioSens.addEventListener('input', (e) => {
    appState.soundsync.sensitivity = parseInt(e.target.value, 10);
    DOM.valAudioSens.textContent = `${e.target.value}%`;
  });

  if (DOM.audioModesSelector) {
    DOM.audioModesSelector.addEventListener('click', (e) => {
      const chip = e.target.closest('.effect-chip');
      if (chip && chip.dataset.audiomode) {
        triggerHaptic('light');
        appState.soundsync.mode = chip.dataset.audiomode;
        DOM.audioModesSelector.querySelectorAll('.effect-chip').forEach(c => c.classList.toggle('active', c === chip));
        showToast(`SoundSync profile: ${chip.textContent}`);
      }
    });
  }

  // Gradient Lab Color Stops
  [DOM.gradColor1, DOM.gradColor2, DOM.gradColor3, DOM.gradColor4].forEach(picker => {
    if (picker) {
      picker.addEventListener('input', () => {
        updateGradientPreviewBar();
      });
      picker.addEventListener('change', () => {
        triggerHaptic('light');
        updateGradientPreviewBar();
      });
    }
  });

  // Motion physics selector chips
  if (DOM.gradientMotionSelector) {
    DOM.gradientMotionSelector.querySelectorAll('.effect-chip').forEach(btn => {
      btn.addEventListener('click', () => {
        triggerHaptic('light');
        DOM.gradientMotionSelector.querySelectorAll('.effect-chip').forEach(b => b.classList.remove('active'));
        btn.classList.add('active');
        appState.gradient.motionMode = btn.dataset.motion;
      });
    });
  }

  // Flow velocity slider
  if (DOM.sliderGradientSpeed) {
    DOM.sliderGradientSpeed.addEventListener('input', (e) => {
      const val = parseInt(e.target.value, 10);
      appState.gradient.speed = val;
      let label = `${val}% (Gentle Fluid)`;
      if (val < 30) label = `${val}% (Deep Calm)`;
      else if (val > 65) label = `${val}% (High Flow)`;
      if (DOM.valGradientSpeed) DOM.valGradientSpeed.textContent = label;
      syncRangeProgress(DOM.sliderGradientSpeed);
    });
    DOM.sliderGradientSpeed.addEventListener('change', () => {
      triggerHaptic('light');
      applyCustomGradientToDesk();
    });
  }

  // Wave turbulence slider
  if (DOM.sliderGradientTurbulence) {
    DOM.sliderGradientTurbulence.addEventListener('input', (e) => {
      const val = parseInt(e.target.value, 10);
      appState.gradient.turbulence = val;
      let label = `${val}% (Organic Flow)`;
      if (val < 30) label = `${val}% (Laminar Smooth)`;
      else if (val > 70) label = `${val}% (Vortex Plasma)`;
      if (DOM.valGradientTurbulence) DOM.valGradientTurbulence.textContent = label;
      syncRangeProgress(DOM.sliderGradientTurbulence);
    });
    DOM.sliderGradientTurbulence.addEventListener('change', () => {
      triggerHaptic('light');
      applyCustomGradientToDesk();
    });
  }

  // Apply Gradient Button
  if (DOM.btnApplyGradient) {
    DOM.btnApplyGradient.addEventListener('click', () => {
      triggerHaptic('medium');
      applyCustomGradientToDesk();
    });
  }

  // Curated Luxury Designer Palettes (10 Palettes)
  document.querySelectorAll('.palette-chip').forEach(chip => {
    chip.addEventListener('click', () => {
      triggerHaptic('light');
      document.querySelectorAll('.palette-chip').forEach(c => c.classList.remove('active'));
      chip.classList.add('active');

      const stops = (chip.dataset.stops || '').split(',');
      if (stops.length >= 4) {
        if (DOM.gradColor1) DOM.gradColor1.value = stops[0];
        if (DOM.gradColor2) DOM.gradColor2.value = stops[1];
        if (DOM.gradColor3) DOM.gradColor3.value = stops[2];
        if (DOM.gradColor4) DOM.gradColor4.value = stops[3];
      }

      if (chip.dataset.motion) {
        appState.gradient.motionMode = chip.dataset.motion;
        if (DOM.gradientMotionSelector) {
          DOM.gradientMotionSelector.querySelectorAll('.effect-chip').forEach(b => {
            b.classList.toggle('active', b.dataset.motion === chip.dataset.motion);
          });
        }
      }

      updateGradientPreviewBar();
      applyCustomGradientToDesk();
    });
  });

  // Corner WiZ Bulbs Integration Mode Switching
  if (DOM.gradientBulbSyncModes) {
    DOM.gradientBulbSyncModes.querySelectorAll('.effect-chip').forEach(btn => {
      btn.addEventListener('click', () => {
        triggerHaptic('light');
        DOM.gradientBulbSyncModes.querySelectorAll('.effect-chip').forEach(b => b.classList.remove('active'));
        btn.classList.add('active');
        const mode = btn.dataset.bulbsync;
        appState.gradient.bulbSyncMode = mode;

        if (DOM.wrapWarmWhiteCct) {
          DOM.wrapWarmWhiteCct.classList.toggle('hidden', mode !== 'warm-white');
        }
        if (DOM.wrapCustomBulbColor) {
          DOM.wrapCustomBulbColor.classList.toggle('hidden', mode !== 'custom');
        }
        applyCustomGradientToDesk();
      });
    });
  }

  // Warm White CCT select
  if (DOM.selectGradientWarmCct) {
    DOM.selectGradientWarmCct.addEventListener('change', (e) => {
      triggerHaptic('light');
      appState.gradient.warmWhiteKelvin = parseInt(e.target.value, 10);
      if (appState.gradient.bulbSyncMode === 'warm-white') {
        applyCustomGradientToDesk();
      }
    });
  }

  // Custom Bulb Color picker
  if (DOM.pickerGradientBulbColor) {
    DOM.pickerGradientBulbColor.addEventListener('input', (e) => {
      appState.gradient.bulbCustomColor = e.target.value;
    });
    DOM.pickerGradientBulbColor.addEventListener('change', (e) => {
      triggerHaptic('light');
      appState.gradient.bulbCustomColor = e.target.value;
      if (appState.gradient.bulbSyncMode === 'custom') {
        applyCustomGradientToDesk();
      }
    });
  }

  // Focus Rhythm (Pomodoro)
  if (DOM.btnTogglePomodoro) {
    DOM.btnTogglePomodoro.addEventListener('click', () => {
      triggerHaptic('medium');
      togglePomodoro();
    });
  }
  if (DOM.btnResetPomodoro) {
    DOM.btnResetPomodoro.addEventListener('click', () => {
      triggerHaptic('light');
      resetPomodoro();
    });
  }

  // Settings Dialog
  DOM.btnOpenSettings.addEventListener('click', () => {
    triggerHaptic('light');
    DOM.modalSettings.showModal();
    DOM.inputWizLeftIp.value = appState.network.wizLeftIp;
    DOM.inputWizRightIp.value = appState.network.wizRightIp;
    if (DOM.inputHubHost) {
      DOM.inputHubHost.value = localStorage.getItem('aura_hub_host') || 
        ((window.location.hostname === 'localhost' || window.location.hostname === '127.0.0.1') ? 'aura-hub.local' : window.location.host);
    }
    updateSecurityContextUI();
  });

  // Settings Dialog Category Tabs
  const settingsTabBtns = document.querySelectorAll('.settings-tab-btn');
  const settingsTabPanels = document.querySelectorAll('.settings-tab-panel');
  function switchSettingsTab(target) {
    settingsTabBtns.forEach(b => {
      const isMatch = (b.dataset.settingsTab === target);
      b.classList.toggle('active', isMatch);
      b.setAttribute('aria-selected', isMatch ? 'true' : 'false');
    });
    settingsTabPanels.forEach(panel => {
      const isMatch = (panel.id === `settings-panel-${target}`);
      panel.classList.toggle('active', isMatch);
      if (isMatch) {
        panel.removeAttribute('hidden');
      } else {
        panel.setAttribute('hidden', '');
      }
    });
  }
  settingsTabBtns.forEach(btn => {
    btn.addEventListener('click', () => {
      triggerHaptic('light');
      switchSettingsTab(btn.dataset.settingsTab);
    });
  });

  DOM.btnCloseSettings.addEventListener('click', () => {
    triggerHaptic('light');
    DOM.modalSettings.close();
  });
  DOM.modalSettings.addEventListener('click', (e) => {
    const d = DOM.modalSettings.getBoundingClientRect();
    if (e.clientX < d.left || e.clientX > d.right || e.clientY < d.top || e.clientY > d.bottom) {
      DOM.modalSettings.close();
    }
  });

  // Browser Security & Permissions Guide Dialog
  if (DOM.btnOpenSecurityGuide) {
    DOM.btnOpenSecurityGuide.addEventListener('click', () => {
      triggerHaptic('light');
      openSecurityGuideModal();
    });
  }

  if (DOM.btnCloseSecurityGuide) {
    DOM.btnCloseSecurityGuide.addEventListener('click', () => {
      triggerHaptic('light');
      closeSecurityGuideModal();
    });
  }

  if (DOM.btnDismissSecurityGuide) {
    DOM.btnDismissSecurityGuide.addEventListener('click', () => {
      triggerHaptic('light');
      closeSecurityGuideModal();
    });
  }

  if (DOM.modalSecurityGuide) {
    DOM.modalSecurityGuide.addEventListener('click', (e) => {
      const d = DOM.modalSecurityGuide.getBoundingClientRect();
      if (e.clientX < d.left || e.clientX > d.right || e.clientY < d.top || e.clientY > d.bottom) {
        closeSecurityGuideModal();
      }
    });
  }

  if (DOM.btnCopyChromeFlag) {
    DOM.btnCopyChromeFlag.addEventListener('click', () => {
      triggerHaptic('light');
      copyToClipboard('chrome://flags/#unsafely-treat-insecure-origin-as-secure', DOM.btnCopyChromeFlag);
    });
  }

  if (DOM.btnCopyChromeOrigin) {
    DOM.btnCopyChromeOrigin.addEventListener('click', () => {
      triggerHaptic('light');
      const originText = (DOM.valChromeOrigin && DOM.valChromeOrigin.textContent) || 'http://aura-hub.local';
      copyToClipboard(originText, DOM.btnCopyChromeOrigin);
    });
  }

  if (DOM.btnCopyPythonCmd) {
    DOM.btnCopyPythonCmd.addEventListener('click', () => {
      triggerHaptic('light');
      copyToClipboard('python3 scripts/run_local.py', DOM.btnCopyPythonCmd);
    });
  }

  DOM.btnScanWiz.addEventListener('click', () => {
    triggerHaptic('medium');
    runWizDiscovery();
  });
  DOM.btnSaveSettings.addEventListener('click', () => {
    triggerHaptic('medium');
    const leftIp = DOM.inputWizLeftIp.value.trim();
    const rightIp = DOM.inputWizRightIp.value.trim();
    const ipRegex = /^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/;
    if (!ipRegex.test(leftIp) || !ipRegex.test(rightIp)) {
      showToast('Invalid IPv4 address format. Please check bulb IPs.');
      return;
    }
    appState.network.wizLeftIp = leftIp;
    appState.network.wizRightIp = rightIp;
    localStorage.setItem('aura_wiz_left_ip', appState.network.wizLeftIp);
    localStorage.setItem('aura_wiz_right_ip', appState.network.wizRightIp);
    DOM.ipDisplayWizLeft.textContent = appState.network.wizLeftIp;
    DOM.ipDisplayWizRight.textContent = appState.network.wizRightIp;

    if (DOM.inputHubHost) {
      const hubVal = DOM.inputHubHost.value.trim() || 'aura-hub.local';
      const prevHub = localStorage.getItem('aura_hub_host');
      localStorage.setItem('aura_hub_host', hubVal);
      if (prevHub !== hubVal && (window.location.hostname === 'localhost' || window.location.hostname === '127.0.0.1')) {
        if (socket) socket.close();
        connectWebSocket();
      }
    }

    DOM.modalSettings.close();
    showToast('Network settings saved.');
    dispatchStateChange({ network: appState.network });
  });

  DOM.toggleGamma.addEventListener('change', (e) => {
    triggerHaptic('light');
    appState.useGammaCorrection = e.target.checked;
    localStorage.setItem('aura_gamma_correction', appState.useGammaCorrection);
    renderAll();
    showToast(appState.useGammaCorrection ? 'CIE 1931 Perceptual Gamma enabled.' : 'Linear dimming active.');
  });

  // --------------------------------------------------------------------------
  // Wireless OTA Firmware Upload
  // --------------------------------------------------------------------------
  let selectedOtaFile = null;
  const MAX_OTA_SIZE = 1507328; // 0x170000 bytes (dual-partition OTA app0/app1 budget)

  function handleOtaFileSelection(file) {
    if (!file) return;
    if (!file.name.endsWith('.bin')) {
      showToast('Please select a valid .bin firmware file.');
      if (DOM.btnStartOtaUpload) DOM.btnStartOtaUpload.disabled = true;
      selectedOtaFile = null;
      return;
    }
    if (file.size > MAX_OTA_SIZE) {
      showToast(`Firmware exceeds partition capacity (${(file.size / 1024).toFixed(0)} KB > ${(MAX_OTA_SIZE / 1024).toFixed(0)} KB).`);
      if (DOM.btnStartOtaUpload) DOM.btnStartOtaUpload.disabled = true;
      selectedOtaFile = null;
      if (DOM.otaFileSize) DOM.otaFileSize.textContent = 'File exceeds partition budget';
      return;
    }
    selectedOtaFile = file;
    if (DOM.otaFileLabel) DOM.otaFileLabel.textContent = file.name;
    const sizeMb = (file.size / (1024 * 1024)).toFixed(2);
    const lowerName = file.name.toLowerCase();
    let targetLabel = 'Dual OTA Ready';
    if (lowerName.includes('littlefs') || lowerName.includes('spiffs') || lowerName.includes('filesystem') || lowerName.includes('data')) {
      targetLabel = 'Web Dashboard (LittleFS)';
    } else {
      targetLabel = 'C++ Firmware (Dual OTA)';
    }
    if (DOM.otaFileSize) DOM.otaFileSize.textContent = `${sizeMb} MB • ${targetLabel}`;
    if (DOM.btnStartOtaUpload) DOM.btnStartOtaUpload.disabled = false;
    showToast(`Loaded ${targetLabel}: ${file.name} (${sizeMb} MB)`);
  }

  if (DOM.btnSelectOtaFile && DOM.inputOtaFile) {
    DOM.btnSelectOtaFile.addEventListener('click', (e) => {
      e.stopPropagation();
      DOM.inputOtaFile.click();
    });
  }

  if (DOM.otaDropZone && DOM.inputOtaFile) {
    DOM.otaDropZone.addEventListener('click', () => {
      DOM.inputOtaFile.click();
    });

    ['dragenter', 'dragover'].forEach(evtName => {
      DOM.otaDropZone.addEventListener(evtName, (e) => {
        e.preventDefault();
        e.stopPropagation();
        DOM.otaDropZone.classList.add('dragover');
      });
    });

    ['dragleave', 'drop'].forEach(evtName => {
      DOM.otaDropZone.addEventListener(evtName, (e) => {
        e.preventDefault();
        e.stopPropagation();
        DOM.otaDropZone.classList.remove('dragover');
      });
    });

    DOM.otaDropZone.addEventListener('drop', (e) => {
      const dt = e.dataTransfer;
      if (dt && dt.files && dt.files[0]) {
        handleOtaFileSelection(dt.files[0]);
      }
    });
  }

  if (DOM.inputOtaFile) {
    DOM.inputOtaFile.addEventListener('change', (e) => {
      if (e.target.files && e.target.files[0]) {
        handleOtaFileSelection(e.target.files[0]);
      }
    });
  }

  if (DOM.btnStartOtaUpload) {
    DOM.btnStartOtaUpload.addEventListener('click', () => {
      if (!selectedOtaFile) {
        showToast('Please choose a .bin file first.');
        return;
      }
      triggerHaptic('heavy');
      DOM.btnStartOtaUpload.disabled = true;
      if (DOM.otaProgressContainer) DOM.otaProgressContainer.classList.remove('hidden');
      if (DOM.otaProgressBar) DOM.otaProgressBar.style.width = '0%';
      if (DOM.otaPctText) DOM.otaPctText.textContent = '0%';
      if (DOM.otaStatusText) DOM.otaStatusText.textContent = 'Uploading firmware... 0%';

      if (typeof FormData === 'undefined' || typeof XMLHttpRequest === 'undefined') {
        showToast('OTA Upload simulation mode active.');
        return;
      }

      const formData = new FormData();
      formData.append('update', selectedOtaFile);

      const xhr = new XMLHttpRequest();
      xhr.open('POST', '/update', true);

      xhr.upload.onprogress = (e) => {
        if (e.lengthComputable) {
          const pct = Math.round((e.loaded / e.total) * 100);
          if (DOM.otaProgressBar) DOM.otaProgressBar.style.width = pct + '%';
          if (DOM.otaPctText) DOM.otaPctText.textContent = pct + '%';
          if (DOM.otaStatusText) {
            DOM.otaStatusText.textContent = pct < 100 
              ? `Uploading firmware... ${pct}%`
              : 'Verifying image & writing to partition...';
          }
        }
      };

      xhr.onload = () => {
        if (xhr.status === 200 && xhr.responseText.includes('OK')) {
          if (DOM.otaProgressBar) DOM.otaProgressBar.style.width = '100%';
          if (DOM.otaPctText) DOM.otaPctText.textContent = '100%';
          let countdown = 6;
          if (DOM.otaStatusText) {
            DOM.otaStatusText.style.color = '#00f2fe';
            DOM.otaStatusText.textContent = `Flash complete! Rebooting in ${countdown}s...`;
          }
          const timer = setInterval(() => {
            countdown--;
            if (countdown > 0) {
              if (DOM.otaStatusText) {
                DOM.otaStatusText.textContent = `Flash complete! Rebooting in ${countdown}s...`;
              }
            } else {
              clearInterval(timer);
              if (DOM.otaStatusText) {
                DOM.otaStatusText.textContent = 'Reconnecting to Aura Hub...';
              }
              setTimeout(() => {
                window.location.reload();
              }, 1500);
            }
          }, 1000);
        } else {
          if (DOM.otaStatusText) {
            DOM.otaStatusText.style.color = '#ef4444';
            DOM.otaStatusText.textContent = `Update Failed: ${xhr.responseText || 'Server error'}`;
          }
          DOM.btnStartOtaUpload.disabled = false;
          showToast('OTA Update failed. Check serial monitor.');
        }
      };

      xhr.onerror = () => {
        // In some browsers, rebooting closes TCP socket which may trigger onerror
        if (DOM.otaProgressBar && DOM.otaProgressBar.style.width === '100%') {
          if (DOM.otaStatusText) {
            DOM.otaStatusText.style.color = '#00f2fe';
            DOM.otaStatusText.textContent = 'Upload complete! Reconnecting in 5s...';
          }
          setTimeout(() => {
            window.location.reload();
          }, 5000);
        } else {
          if (DOM.otaStatusText) {
            DOM.otaStatusText.style.color = '#ef4444';
            DOM.otaStatusText.textContent = 'Network error during upload.';
          }
          DOM.btnStartOtaUpload.disabled = false;
        }
      };

      xhr.send(formData);
    });
  }

  if (DOM.btnDismissRollback) {
    DOM.btnDismissRollback.addEventListener('click', () => {
      triggerHaptic('light');
      if (DOM.otaRollbackBanner) DOM.otaRollbackBanner.classList.add('hidden');
      sendWsAction({ action: 'dismissRollback' });
    });
  }

  // --------------------------------------------------------------------------
  // Hardware & Pixel Diagnostics
  // --------------------------------------------------------------------------
  function sendPixelInspect(index, r = 255, g = 255, b = 255) {
    if (isSimulatedMode) {
      if (index === -1) {
        showToast('Diagnostics: Restored all LEDs.');
      } else {
        showToast(`Diagnostics: Inspecting LED #${index} [${r}, ${g}, ${b}].`);
      }
      return;
    }
    fetch(`/api/led/inspect?index=${index}&r=${r}&g=${g}&b=${b}`)
      .then(res => res.json())
      .then(data => {
        if (index === -1) {
          showToast('Hardware restored to normal animation mode.');
        } else {
          showToast(`Inspecting LED #${index}`);
        }
      })
      .catch(err => {
        console.warn('Inspect error:', err);
      });
  }

  if (DOM.sliderPixelDiag) {
    DOM.sliderPixelDiag.addEventListener('input', (e) => {
      const val = parseInt(e.target.value, 10);
      if (DOM.valPixelDiag) {
        DOM.valPixelDiag.textContent = val === -1 ? 'None (Normal Mode)' : `LED #${val}`;
        if (val >= 0) {
          DOM.valPixelDiag.title = val <= 29
            ? `LED ${val + 1} of 60: Left Zone (Near Left WiZ Lamp)`
            : `LED ${val + 1} of 60: Right Zone (Near Right WiZ Lamp)`;
        } else {
          DOM.valPixelDiag.title = 'Normal Mode';
        }
      }
    });

    DOM.sliderPixelDiag.addEventListener('change', (e) => {
      triggerHaptic('light');
      const val = parseInt(e.target.value, 10);
      sendPixelInspect(val, 255, 255, 255);
    });
  }

  if (DOM.selectOnOffAnim) {
    const savedAnim = localStorage.getItem('aura_onoff_anim');
    if (savedAnim) appState.onOffAnimation = savedAnim;
    DOM.selectOnOffAnim.value = appState.onOffAnimation || 'gaussian-ripple';
    DOM.selectOnOffAnim.addEventListener('change', (e) => {
      triggerHaptic('medium');
      appState.onOffAnimation = e.target.value;
      localStorage.setItem('aura_onoff_anim', appState.onOffAnimation);
      dispatchStateChange({ onOffAnimation: appState.onOffAnimation });
      const opt = e.target.options && e.target.selectedIndex >= 0 ? e.target.options[e.target.selectedIndex] : null;
      const optName = opt ? opt.text : e.target.value;
      showToast(`Desk Transition: ${optName}`);
    });
  }

  if (DOM.btnPixelDiagReset) {
    DOM.btnPixelDiagReset.addEventListener('click', () => {
      triggerHaptic('medium');
      if (DOM.sliderPixelDiag) DOM.sliderPixelDiag.value = '-1';
      if (DOM.valPixelDiag) DOM.valPixelDiag.textContent = 'None (Normal Mode)';
      sendPixelInspect(-1);
    });
  }

  // --------------------------------------------------------------------------
  // Wi-Fi & Network Provisioning (SoftAP & Dynamic Reconfiguration)
  // --------------------------------------------------------------------------
  if (DOM.btnToggleWifiChange) {
    DOM.btnToggleWifiChange.addEventListener('click', () => {
      triggerHaptic('light');
      if (DOM.wifiChangePanel) {
        DOM.wifiChangePanel.classList.toggle('hidden');
        if (!DOM.wifiChangePanel.classList.contains('hidden')) {
          if (DOM.selectWifiNetworks && DOM.selectWifiNetworks.children.length <= 1) {
            triggerWifiScan();
          }
        }
      }
    });
  }

  function triggerWifiScan() {
    if (!DOM.btnScanWifi) return;
    triggerHaptic('medium');
    DOM.btnScanWifi.disabled = true;
    DOM.btnScanWifi.innerHTML = '<span>Scanning airwaves...</span>';
    if (DOM.wifiStatusFeedback) {
      DOM.wifiStatusFeedback.style.display = 'block';
      DOM.wifiStatusFeedback.style.color = 'var(--accent-cyan)';
      DOM.wifiStatusFeedback.textContent = 'Scanning 2.4GHz channels for nearby networks...';
    }

    const host = getTargetHubHost();
    const scanUrl = (window.location.protocol === 'file:') ? '/api/wifi/scan' : `http://${host}/api/wifi/scan`;

    fetch(scanUrl)
      .then(res => res.json())
      .then(data => {
        DOM.btnScanWifi.disabled = false;
        DOM.btnScanWifi.innerHTML = '<span>⚡ Scan Nearby Networks</span>';
        if (DOM.selectWifiNetworks) {
          DOM.selectWifiNetworks.innerHTML = '<option value="">Select a network or enter below...</option>';
          if (data && data.networks && data.networks.length > 0) {
            data.networks.forEach(net => {
              const opt = document.createElement('option');
              opt.value = net.ssid;
              const lock = net.secure ? '🔒 ' : '';
              opt.textContent = `${lock}${net.ssid} (${net.rssi} dBm)`;
              DOM.selectWifiNetworks.appendChild(opt);
            });
            if (DOM.wifiStatusFeedback) {
              DOM.wifiStatusFeedback.textContent = `Found ${data.networks.length} networks nearby.`;
            }
          } else {
            if (DOM.wifiStatusFeedback) {
              DOM.wifiStatusFeedback.style.color = 'var(--accent-coral)';
              DOM.wifiStatusFeedback.textContent = 'No networks discovered. Enter SSID manually below.';
            }
          }
        }
      })
      .catch(err => {
        DOM.btnScanWifi.disabled = false;
        DOM.btnScanWifi.innerHTML = '<span>⚡ Scan Nearby Networks</span>';
        if (DOM.wifiStatusFeedback) {
          DOM.wifiStatusFeedback.style.color = 'var(--accent-coral)';
          DOM.wifiStatusFeedback.textContent = 'Could not scan Wi-Fi airwaves. Enter SSID manually.';
        }
      });
  }

  if (DOM.btnScanWifi) {
    DOM.btnScanWifi.addEventListener('click', triggerWifiScan);
  }

  if (DOM.selectWifiNetworks) {
    DOM.selectWifiNetworks.addEventListener('change', (e) => {
      triggerHaptic('light');
      if (DOM.inputWifiSsid && e.target.value) {
        DOM.inputWifiSsid.value = e.target.value;
      }
    });
  }

  if (DOM.btnToggleWifiPassVisibility) {
    DOM.btnToggleWifiPassVisibility.addEventListener('click', () => {
      triggerHaptic('light');
      if (DOM.inputWifiPass) {
        DOM.inputWifiPass.type = (DOM.inputWifiPass.type === 'password') ? 'text' : 'password';
      }
    });
  }

  if (DOM.btnSubmitWifiChange) {
    DOM.btnSubmitWifiChange.addEventListener('click', () => {
      triggerHaptic('medium');
      const ssid = DOM.inputWifiSsid ? DOM.inputWifiSsid.value.trim() : '';
      const pass = DOM.inputWifiPass ? DOM.inputWifiPass.value : '';

      if (!ssid) {
        showToast('Please enter or select a Wi-Fi SSID.');
        return;
      }

      DOM.btnSubmitWifiChange.disabled = true;
      if (DOM.wifiStatusFeedback) {
        DOM.wifiStatusFeedback.style.display = 'block';
        DOM.wifiStatusFeedback.style.color = 'var(--accent-cyan)';
        DOM.wifiStatusFeedback.textContent = `Saving credentials for "${ssid}" & rebooting hub...`;
      }

      const host = getTargetHubHost();
      const saveUrl = (window.location.protocol === 'file:') ? '/api/wifi/save' : `http://${host}/api/wifi/save`;

      fetch(saveUrl, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ ssid: ssid, password: pass })
      })
      .then(res => res.json())
      .then(data => {
        showToast(`Credentials saved! Hub reconnecting to ${ssid}...`);
        if (DOM.wifiStatusFeedback) {
          DOM.wifiStatusFeedback.style.color = 'var(--accent-green)';
          DOM.wifiStatusFeedback.textContent = `Rebooting! Connect your device to "${ssid}" and reload http://aura-hub.local`;
        }
        setTimeout(() => {
          DOM.btnSubmitWifiChange.disabled = false;
        }, 5000);
      })
      .catch(err => {
        showToast(`Saved! Hub restarting to join ${ssid}.`);
        if (DOM.wifiStatusFeedback) {
          DOM.wifiStatusFeedback.style.color = 'var(--accent-green)';
          DOM.wifiStatusFeedback.textContent = `Hub restarting. Reconnect to "${ssid}" in 10 seconds.`;
        }
        setTimeout(() => {
          DOM.btnSubmitWifiChange.disabled = false;
        }, 5000);
      });
    });
  }

  if (DOM.btnResetWifi) {
    DOM.btnResetWifi.addEventListener('click', () => {
      triggerHaptic('heavy');
      const confirmed = confirm('Reset Wi-Fi credentials to factory Setup Mode? The Hub will spin up the "Aura-Studio-Setup" hotspot.');
      if (!confirmed) return;

      DOM.btnResetWifi.disabled = true;
      if (DOM.wifiStatusFeedback) {
        DOM.wifiStatusFeedback.style.display = 'block';
        DOM.wifiStatusFeedback.style.color = 'var(--accent-coral)';
        DOM.wifiStatusFeedback.textContent = 'Clearing Wi-Fi NVS and launching Aura-Studio-Setup hotspot...';
      }

      const host = getTargetHubHost();
      const resetUrl = (window.location.protocol === 'file:') ? '/api/wifi/reset' : `http://${host}/api/wifi/reset`;

      fetch(resetUrl, { method: 'POST' })
        .then(res => res.json())
        .then(data => {
          showToast('Wi-Fi reset! Hub launching Aura-Studio-Setup...');
        })
        .catch(err => {
          showToast('Hub restarting into Setup Hotspot.');
        });
    });
  }

  // Interactive 2.5D Room Viewport Fixture Click Toggles
  const fixtureLeft = document.getElementById('fixture-wiz-left');
  const fixtureRight = document.getElementById('fixture-wiz-right');
  const fixtureDesk = document.getElementById('fixture-desk');

  if (fixtureLeft) {
    fixtureLeft.addEventListener('click', () => {
      triggerHaptic('medium');
      appState.devices.wizLeft.power = !appState.devices.wizLeft.power;
      if (appState.syncCorners) appState.devices.wizRight.power = appState.devices.wizLeft.power;
      renderAll();
      dispatchStateChange({
        wizLeft: { power: appState.devices.wizLeft.power },
        ...(appState.syncCorners ? { wizRight: { power: appState.devices.wizRight.power } } : {})
      });
      showToast(`Left Corner WiZ: ${appState.devices.wizLeft.power ? 'ON' : 'OFF'}`);
    });
  }

  if (fixtureRight) {
    fixtureRight.addEventListener('click', () => {
      triggerHaptic('medium');
      appState.devices.wizRight.power = !appState.devices.wizRight.power;
      if (appState.syncCorners) appState.devices.wizLeft.power = appState.devices.wizRight.power;
      renderAll();
      dispatchStateChange({
        wizRight: { power: appState.devices.wizRight.power },
        ...(appState.syncCorners ? { wizLeft: { power: appState.devices.wizLeft.power } } : {})
      });
      showToast(`Right Corner WiZ: ${appState.devices.wizRight.power ? 'ON' : 'OFF'}`);
    });
  }

  if (fixtureDesk) {
    fixtureDesk.addEventListener('click', () => {
      triggerHaptic('medium');
      appState.devices.desk.power = !appState.devices.desk.power;
      renderAll();
      dispatchStateChange({ desk: { power: appState.devices.desk.power } });
      showToast(`Desk WS2812B Strip: ${appState.devices.desk.power ? 'ON' : 'OFF'}`);
    });
  }

  setupBulbSwatches('wiz-left-swatches', 'wizLeft');
  setupBulbSwatches('wiz-right-swatches', 'wizRight');
}

function setupBulbSwatches(containerId, deviceKey) {
  const container = document.getElementById(containerId);
  if (!container) return;
  container.addEventListener('click', (e) => {
    const dot = e.target.closest('.swatch-dot');
    if (dot) {
      triggerHaptic('light');
      const color = dot.dataset.color;
      appState.devices[deviceKey].color = color;
      appState.devices[deviceKey].mode = 'rgb';
      if (appState.syncCorners) {
        const otherKey = deviceKey === 'wizLeft' ? 'wizRight' : 'wizLeft';
        appState.devices[otherKey].color = color;
        appState.devices[otherKey].mode = 'rgb';
      }
      renderAll();
      dispatchStateChange({
        [deviceKey]: { color: color, mode: 'rgb' },
        ...(appState.syncCorners ? { [deviceKey === 'wizLeft' ? 'wizRight' : 'wizLeft']: { color: color, mode: 'rgb' } } : {})
      });
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
        <button class="assign-btn" data-assign="left" data-ip="${dev.ip}" data-mac="${dev.mac || ''}">Set Left</button>
        <button class="assign-btn" data-assign="right" data-ip="${dev.ip}" data-mac="${dev.mac || ''}">Set Right</button>
        <button class="assign-btn test-wiz-btn" data-ip="${dev.ip}" style="background:rgba(0,242,254,0.12); border-color:rgba(0,242,254,0.4); color:var(--accent-cyan)">⚡ Ping</button>
      </div>
    `;

    item.querySelectorAll('.test-wiz-btn').forEach(btn => {
      btn.addEventListener('click', async () => {
        triggerHaptic('medium');
        btn.textContent = 'Pinging...';
        try {
          const res = await fetch(`/api/wiz/test?ip=${btn.dataset.ip}&action=on`);
          const data = await res.json();
          if (data.rawBulbResponse && data.rawBulbResponse.includes('success')) {
            showToast(`WiZ Bulb (${btn.dataset.ip}): Responded SUCCESS!`);
            btn.textContent = '✓ Active';
            btn.style.borderColor = 'var(--accent-emerald)';
            btn.style.color = 'var(--accent-emerald)';
          } else {
            showToast(`WiZ Bulb: ${data.rawBulbResponse || 'No response'}`);
            btn.textContent = 'Check App';
            btn.style.borderColor = 'var(--accent-rose)';
            btn.style.color = 'var(--accent-rose)';
          }
        } catch (e) {
          showToast(`Offline mode / Simulator: Bulb ${btn.dataset.ip} ping simulated.`);
          btn.textContent = '✓ Simulated';
          btn.style.borderColor = 'var(--accent-amber)';
          btn.style.color = 'var(--accent-amber)';
        }
      });
    });

    item.querySelectorAll('.assign-btn:not(.test-wiz-btn)').forEach(btn => {
      btn.addEventListener('click', () => {
        triggerHaptic('medium');
        const targetIp = btn.dataset.ip;
        const targetMac = btn.dataset.mac || '';
        if (btn.dataset.assign === 'left') {
          appState.network.wizLeftIp = targetIp;
          if (targetMac) appState.network.wizLeftMac = targetMac;
          appState.devices.wizLeft.power = true;
          appState.devices.wizLeft.brightness = 80;
          DOM.inputWizLeftIp.value = targetIp;
          if (DOM.ipDisplayWizLeft) DOM.ipDisplayWizLeft.textContent = targetIp;
          localStorage.setItem('aura_wiz_left_ip', targetIp);
          if (targetMac) localStorage.setItem('aura_wiz_left_mac', targetMac);
          dispatchStateChange({
            network: {
              wizLeftIp: targetIp,
              wizLeftMac: appState.network.wizLeftMac || '',
              wizRightIp: appState.network.wizRightIp,
              wizRightMac: appState.network.wizRightMac || ''
            }
          });
          // Immediately power on & sync bulb as instant feedback
          dispatchStateChange({ wizLeft: { power: true, brightness: 80 } });
          showToast(`Assigned ${targetIp} to Left WiZ & Synced!`);
        } else {
          appState.network.wizRightIp = targetIp;
          if (targetMac) appState.network.wizRightMac = targetMac;
          appState.devices.wizRight.power = true;
          appState.devices.wizRight.brightness = 80;
          DOM.inputWizRightIp.value = targetIp;
          if (DOM.ipDisplayWizRight) DOM.ipDisplayWizRight.textContent = targetIp;
          localStorage.setItem('aura_wiz_right_ip', targetIp);
          if (targetMac) localStorage.setItem('aura_wiz_right_mac', targetMac);
          dispatchStateChange({
            network: {
              wizLeftIp: appState.network.wizLeftIp,
              wizLeftMac: appState.network.wizLeftMac || '',
              wizRightIp: targetIp,
              wizRightMac: appState.network.wizRightMac || ''
            }
          });
          // Immediately power on & sync bulb as instant feedback
          dispatchStateChange({ wizRight: { power: true, brightness: 80 } });
          showToast(`Assigned ${targetIp} to Right WiZ & Synced!`);
        }
        renderAll();
        btn.textContent = '✓ Assigned';
        btn.style.background = 'var(--accent-emerald)';
        btn.style.color = '#fff';
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

function hslToHex(h, s, l) {
  l /= 100;
  const a = (s * Math.min(l, 1 - l)) / 100;
  const f = n => {
    const k = (n + h / 30) % 12;
    const color = l - a * Math.max(Math.min(k - 3, 9 - k, 1), -1);
    return Math.round(255 * color).toString(16).padStart(2, '0');
  };
  return `#${f(0)}${f(8)}${f(4)}`;
}

function hexToHue(hex) {
  if (!hex || hex.length < 7) return 0;
  const r = parseInt(hex.slice(1, 3), 16) / 255;
  const g = parseInt(hex.slice(3, 5), 16) / 255;
  const b = parseInt(hex.slice(5, 7), 16) / 255;
  const max = Math.max(r, g, b), min = Math.min(r, g, b);
  let h = 0;
  const d = max - min;
  if (d === 0) return 0;
  if (max === r) h = ((g - b) / d + (g < b ? 6 : 0)) * 60;
  else if (max === g) h = ((b - r) / d + 2) * 60;
  else if (max === b) h = ((r - g) / d + 4) * 60;
  return Math.round(h);
}

function syncActiveSwatches(container, activeColor) {
  if (!container) return;
  const target = (activeColor || '').toLowerCase();
  container.querySelectorAll('.swatch-dot').forEach(dot => {
    dot.classList.toggle('active', (dot.dataset.color || '').toLowerCase() === target);
  });
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
  const savedGamma = localStorage.getItem('aura_gamma_correction');
  if (savedGamma !== null) {
    appState.useGammaCorrection = savedGamma === 'true';
  }
  const savedTransition = localStorage.getItem('aura_transition_duration');
  if (savedTransition !== null) {
    appState.transitionDuration = parseInt(savedTransition, 10);
    document.documentElement.style.setProperty('--lighting-fade-speed', `${appState.transitionDuration / 1000}s`);
  }

  DOM.ipDisplayWizLeft.textContent = appState.network.wizLeftIp;
  DOM.ipDisplayWizRight.textContent = appState.network.wizRightIp;

  bindEvents();
  initCommunications();
  updateSecurityContextUI();
  renderAll();
  startGradientFluidSimulation();

  // Background ticker for Circadian HCL auto-update
  setInterval(() => {
    if (appState.circadian.autoEnabled) {
      const now = new Date();
      appState.circadian.simulatedMinutes = now.getHours() * 60 + now.getMinutes();
      applyCircadianToRoom();
    }
  }, 30000);
});
