/**
 * AURA // Smart Light Room Hub
 * Production JavaScript Engine & Unified State Orchestrator
 * Supports: Live ESP32 WebSockets + Zero-Config Simulated Mode
 */

'use strict';

// ============================================================================
// 1. STATE DEFINITION & DEFAULT PRESETS
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
  contrastRatio: 0, // -50 (More Ambient) to +50 (More Bias)
  syncCorners: true,
  activePreset: 'cyberpunk',
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
      mode: 'rgb', // 'rgb' or 'cct'
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
// 2. DOM ELEMENT CACHE
// ============================================================================

const DOM = {
  // Connection & Master
  connectionBadge: document.getElementById('connection-badge'),
  connectionText: document.getElementById('connection-text'),
  btnMasterPower: document.getElementById('btn-master-power'),
  sliderMasterBrightness: document.getElementById('slider-master-brightness'),
  valMasterBrightness: document.getElementById('val-master-brightness'),
  sliderMasterContrast: document.getElementById('slider-master-contrast'),
  valMasterContrast: document.getElementById('val-master-contrast'),

  // Spatial Visualizer elements
  glowCornerLeft: document.getElementById('glow-corner-left'),
  glowCornerRight: document.getElementById('glow-corner-right'),
  glowDeskBias: document.getElementById('glow-desk-bias'),
  virtualLedStrip: document.getElementById('virtual-led-strip'),
  fixtureWizLeft: document.getElementById('fixture-wiz-left'),
  fixtureWizRight: document.getElementById('fixture-wiz-right'),
  badgeWizLeftVal: document.getElementById('badge-wiz-left-val'),
  badgeWizRightVal: document.getElementById('badge-wiz-right-val'),
  badgeDeskVal: document.getElementById('badge-desk-val'),
  telemetryLux: document.getElementById('telemetry-lux'),
  telemetryContrast: document.getElementById('telemetry-contrast'),

  // Cards
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

  // Toast
  toastContainer: document.getElementById('toast-container')
};

// ============================================================================
// 3. WEBSOCKET & COMMUNICATIONS MANAGER
// ============================================================================

let socket = null;
let isSimulatedMode = false;
let socketReconnectTimer = null;

function initCommunications() {
  const isEsp32Host = window.location.protocol.startsWith('http') &&
    window.location.hostname !== 'localhost' &&
    window.location.hostname !== '127.0.0.1';

  if (!isEsp32Host) {
    // Running in local preview / test environment -> Use Simulated Mode
    activateSimulatedMode();
    return;
  }

  // Connect to ESP32 WebSocket
  connectWebSocket();
}

function activateSimulatedMode() {
  isSimulatedMode = true;
  DOM.connectionBadge.className = 'status-chip simulated';
  DOM.connectionText.textContent = 'SIMULATOR (OFFLINE)';
  showToast('Running in local simulated mode with virtual WiZ & WS2812B.');
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
      // Request initial state from ESP32
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

/**
 * Dispatches state changes to ESP32 over WebSocket or simulated logger
 */
function dispatchStateChange(payload) {
  if (socket && socket.readyState === WebSocket.OPEN) {
    socket.send(JSON.stringify({
      type: 'update',
      payload: payload
    }));
  } else {
    // Simulated behavior
    // console.log('[Simulator Dispatch]', payload);
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

/**
 * Updates CSS Variables, Range Sliders, and Spatial 2.5D visualizer
 */
function renderAll() {
  renderVisualizer();
  syncAllInputs();
  renderScenesDeck();
  renderDeviceCards();
}

function renderVisualizer() {
  const root = document.documentElement;

  // Compute actual output colors & brightness with master dimming and contrast applied
  const masterFactor = appState.isMasterOn ? (appState.masterBrightness / 100) : 0;

  // Contrast ratio adjustment:
  // Contrast > 0: bias desk higher, corner bulbs lower
  // Contrast < 0: bias corner bulbs higher, desk lower
  const contrastFactor = appState.contrastRatio / 100;
  const deskFactor = Math.min(1, Math.max(0, 1 + contrastFactor));
  const cornerFactor = Math.min(1, Math.max(0, 1 - contrastFactor));

  // Desk effective brightness
  const effBrightDesk = (appState.devices.desk.power && appState.isMasterOn)
    ? (appState.devices.desk.brightness / 100) * masterFactor * deskFactor
    : 0;

  // WiZ Left effective brightness
  const effBrightWizLeft = (appState.devices.wizLeft.power && appState.isMasterOn)
    ? (appState.devices.wizLeft.brightness / 100) * masterFactor * cornerFactor
    : 0;

  // WiZ Right effective brightness
  const effBrightWizRight = (appState.devices.wizRight.power && appState.isMasterOn)
    ? (appState.devices.wizRight.brightness / 100) * masterFactor * cornerFactor
    : 0;

  // Colors
  const deskColor = appState.devices.desk.color;
  const wizLeftColor = appState.devices.wizLeft.mode === 'rgb'
    ? appState.devices.wizLeft.color
    : kelvinToRgbHex(appState.devices.wizLeft.kelvin);
  const wizRightColor = appState.devices.wizRight.mode === 'rgb'
    ? appState.devices.wizRight.color
    : kelvinToRgbHex(appState.devices.wizRight.kelvin);

  // Apply to CSS tokens
  root.style.setProperty('--color-desk', deskColor);
  root.style.setProperty('--color-wiz-left', wizLeftColor);
  root.style.setProperty('--color-wiz-right', wizRightColor);
  root.style.setProperty('--bright-desk', effBrightDesk.toFixed(2));
  root.style.setProperty('--bright-wiz-left', effBrightWizLeft.toFixed(2));
  root.style.setProperty('--bright-wiz-right', effBrightWizRight.toFixed(2));

  // Visualizer Badges
  DOM.badgeDeskVal.textContent = appState.devices.desk.power && appState.isMasterOn
    ? `${Math.round(effBrightDesk * 100)}% • ${capitalize(appState.devices.desk.effect)}`
    : 'OFF';

  DOM.badgeWizLeftVal.textContent = appState.devices.wizLeft.power && appState.isMasterOn
    ? `${Math.round(effBrightWizLeft * 100)}%`
    : 'OFF';

  DOM.badgeWizRightVal.textContent = appState.devices.wizRight.power && appState.isMasterOn
    ? `${Math.round(effBrightWizRight * 100)}%`
    : 'OFF';

  // Overall Room Lux & Contrast telemetry
  const totalLux = Math.round(((effBrightDesk + effBrightWizLeft + effBrightWizRight) / 3) * 100);
  DOM.telemetryLux.innerHTML = `TOTAL LUMENS: <strong>${totalLux}%</strong>`;

  const ratioVal = effBrightWizLeft > 0 ? (effBrightDesk / Math.max(0.1, effBrightWizLeft)).toFixed(1) : '∞';
  DOM.telemetryContrast.innerHTML = `CONTRAST RATIO: <strong>${ratioVal}:1</strong>`;
}

function syncAllInputs() {
  // Master Switch & Sliders
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

  // Desk Effects Chips
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

  // WiZ Left Mode Tabs
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

  // Sync Corners Button
  DOM.btnSyncCorners.classList.toggle('active', appState.syncCorners);
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

/**
 * Modern Web Guidance: Updates CSS custom property for custom-styled sliders
 */
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
// 5. EVENT LISTENERS & USER INTERACTIONS
// ============================================================================

function bindEvents() {
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

  // Presets Click
  document.querySelectorAll('.scene-card').forEach(card => {
    card.addEventListener('click', () => {
      const presetKey = card.dataset.preset;
      applyPreset(presetKey);
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

  // Desk Effects
  document.querySelectorAll('#effects-selector .effect-chip').forEach(chip => {
    chip.addEventListener('click', () => {
      appState.devices.desk.effect = chip.dataset.effect;
      renderAll();
      dispatchStateChange({ desk: { effect: appState.devices.desk.effect } });
    });
  });

  // Desk Swatches
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
  DOM.sliderDeskSpeed.addEventListener('change', () => {
    dispatchStateChange({ desk: { speed: appState.devices.desk.speed } });
  });

  DOM.sliderDeskIntensity.addEventListener('input', (e) => {
    appState.devices.desk.intensity = parseInt(e.target.value, 10);
    renderAll();
  });
  DOM.sliderDeskIntensity.addEventListener('change', () => {
    dispatchStateChange({ desk: { intensity: appState.devices.desk.intensity } });
  });

  // WiZ Left Controls
  DOM.toggleWizLeftPower.addEventListener('change', (e) => {
    appState.devices.wizLeft.power = e.target.checked;
    if (appState.syncCorners) {
      appState.devices.wizRight.power = e.target.checked;
    }
    renderAll();
    dispatchStateChange({
      wizLeft: { power: appState.devices.wizLeft.power },
      ...(appState.syncCorners ? { wizRight: { power: appState.devices.wizRight.power } } : {})
    });
  });

  DOM.sliderWizLeftBrightness.addEventListener('input', (e) => {
    appState.devices.wizLeft.brightness = parseInt(e.target.value, 10);
    if (appState.syncCorners) {
      appState.devices.wizRight.brightness = appState.devices.wizLeft.brightness;
    }
    renderAll();
  });
  DOM.sliderWizLeftBrightness.addEventListener('change', () => {
    dispatchStateChange({
      wizLeft: { brightness: appState.devices.wizLeft.brightness },
      ...(appState.syncCorners ? { wizRight: { brightness: appState.devices.wizRight.brightness } } : {})
    });
  });

  DOM.pickerWizLeftColor.addEventListener('input', (e) => {
    appState.devices.wizLeft.color = e.target.value;
    if (appState.syncCorners) {
      appState.devices.wizRight.color = e.target.value;
    }
    renderAll();
  });
  DOM.pickerWizLeftColor.addEventListener('change', () => {
    dispatchStateChange({
      wizLeft: { color: appState.devices.wizLeft.color },
      ...(appState.syncCorners ? { wizRight: { color: appState.devices.wizRight.color } } : {})
    });
  });

  DOM.sliderWizLeftKelvin.addEventListener('input', (e) => {
    appState.devices.wizLeft.kelvin = parseInt(e.target.value, 10);
    if (appState.syncCorners) {
      appState.devices.wizRight.kelvin = appState.devices.wizLeft.kelvin;
    }
    renderAll();
  });
  DOM.sliderWizLeftKelvin.addEventListener('change', () => {
    dispatchStateChange({
      wizLeft: { kelvin: appState.devices.wizLeft.kelvin },
      ...(appState.syncCorners ? { wizRight: { kelvin: appState.devices.wizRight.kelvin } } : {})
    });
  });

  // WiZ Right Controls (Independent adjustments)
  DOM.toggleWizRightPower.addEventListener('change', (e) => {
    appState.devices.wizRight.power = e.target.checked;
    renderAll();
    dispatchStateChange({ wizRight: { power: appState.devices.wizRight.power } });
  });

  DOM.sliderWizRightBrightness.addEventListener('input', (e) => {
    appState.devices.wizRight.brightness = parseInt(e.target.value, 10);
    renderAll();
  });
  DOM.sliderWizRightBrightness.addEventListener('change', () => {
    dispatchStateChange({ wizRight: { brightness: appState.devices.wizRight.brightness } });
  });

  DOM.pickerWizRightColor.addEventListener('input', (e) => {
    appState.devices.wizRight.color = e.target.value;
    renderAll();
  });
  DOM.pickerWizRightColor.addEventListener('change', () => {
    dispatchStateChange({ wizRight: { color: appState.devices.wizRight.color } });
  });

  DOM.sliderWizRightKelvin.addEventListener('input', (e) => {
    appState.devices.wizRight.kelvin = parseInt(e.target.value, 10);
    renderAll();
  });
  DOM.sliderWizRightKelvin.addEventListener('change', () => {
    dispatchStateChange({ wizRight: { kelvin: appState.devices.wizRight.kelvin } });
  });

  // WiZ Corner Mode Tabs (RGB vs CCT)
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
      dispatchStateChange({
        wizLeft: { mode: appState.devices.wizLeft.mode },
        wizRight: { mode: appState.devices.wizRight.mode }
      });
    });
  });

  // Sync Corners Toggle Button
  DOM.btnSyncCorners.addEventListener('click', () => {
    appState.syncCorners = !appState.syncCorners;
    if (appState.syncCorners) {
      // Immediately mirror Left to Right
      Object.assign(appState.devices.wizRight, JSON.parse(JSON.stringify(appState.devices.wizLeft)));
      showToast('Corner bulbs linked. Right mirrors Left.');
    } else {
      showToast('Corner bulbs unlinked. Independent control active.');
    }
    renderAll();
    dispatchStateChange({ syncCorners: appState.syncCorners });
  });

  // Settings Modal Dialog
  DOM.btnOpenSettings.addEventListener('click', () => {
    DOM.modalSettings.showModal();
    DOM.inputWizLeftIp.value = appState.network.wizLeftIp;
    DOM.inputWizRightIp.value = appState.network.wizRightIp;
  });

  DOM.btnCloseSettings.addEventListener('click', () => {
    DOM.modalSettings.close();
  });

  // Close on backdrop click (Light Dismiss pattern per Modern Web Guidance)
  DOM.modalSettings.addEventListener('click', (e) => {
    const dialogDimensions = DOM.modalSettings.getBoundingClientRect();
    if (
      e.clientX < dialogDimensions.left ||
      e.clientX > dialogDimensions.right ||
      e.clientY < dialogDimensions.top ||
      e.clientY > dialogDimensions.bottom
    ) {
      DOM.modalSettings.close();
    }
  });

  // WiZ Scanner Button
  DOM.btnScanWiz.addEventListener('click', runWizDiscovery);

  // Save Settings
  DOM.btnSaveSettings.addEventListener('click', () => {
    appState.network.wizLeftIp = DOM.inputWizLeftIp.value.trim();
    appState.network.wizRightIp = DOM.inputWizRightIp.value.trim();
    localStorage.setItem('aura_wiz_left_ip', appState.network.wizLeftIp);
    localStorage.setItem('aura_wiz_right_ip', appState.network.wizRightIp);
    DOM.ipDisplayWizLeft.textContent = appState.network.wizLeftIp;
    DOM.ipDisplayWizRight.textContent = appState.network.wizRightIp;
    DOM.modalSettings.close();
    showToast('Network IPs updated and saved.');
    dispatchStateChange({ network: appState.network });
  });

  // Quick Swatches for WiZ Left & Right
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
      dispatchStateChange({
        [deviceKey]: { color: dot.dataset.color, mode: 'rgb' }
      });
    }
  });
}

// ============================================================================
// 6. PRESET APPLICATION & LOGIC
// ============================================================================

function applyPreset(presetKey) {
  const preset = PRESETS[presetKey];
  if (!preset) return;

  appState.activePreset = presetKey;
  appState.masterBrightness = preset.masterBrightness;
  appState.contrastRatio = preset.contrast;
  appState.isMasterOn = true;

  // Clone properties into devices
  Object.assign(appState.devices.desk, preset.desk);
  Object.assign(appState.devices.wizLeft, preset.wizLeft);
  Object.assign(appState.devices.wizRight, preset.wizRight);

  renderAll();
  dispatchStateChange({
    activePreset: presetKey,
    state: appState
  });

  showToast(`Activated scene: ${preset.name}`);
}

// ============================================================================
// 7. WIZ AUTO-DISCOVERY SCANNER
// ============================================================================

function runWizDiscovery() {
  DOM.discoveryResults.innerHTML = `
    <div class="empty-state">
      <span class="status-dot" style="display:inline-block; margin-right:8px; background:var(--accent-cyan); animation:pulse 1s infinite"></span>
      Broadcasting UDP discovery packet to 255.255.255.255:38899...
    </div>
  `;

  if (socket && socket.readyState === WebSocket.OPEN) {
    socket.send(JSON.stringify({ type: 'scanWiz' }));
  } else {
    // Simulated discovery results for immediate testing
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
    DOM.discoveryResults.innerHTML = `<div class="empty-state">No WiZ bulbs responded. Ensure bulbs are powered on and on the same Wi-Fi.</div>`;
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
        const target = btn.dataset.assign;
        const ip = btn.dataset.ip;
        if (target === 'left') {
          DOM.inputWizLeftIp.value = ip;
          showToast(`Assigned ${ip} to Left Corner WiZ`);
        } else {
          DOM.inputWizRightIp.value = ip;
          showToast(`Assigned ${ip} to Right Corner WiZ`);
        }
      });
    });

    DOM.discoveryResults.appendChild(item);
  });
}

// ============================================================================
// 8. UTILITIES & COLOR CONVERSIONS
// ============================================================================

/**
 * Converts Kelvin Color Temperature (2000K - 6500K) to RGB Hex
 * Uses Tanner Helland's algorithm for accurate warm blackbody radiation
 */
function kelvinToRgbHex(kelvin) {
  const temp = kelvin / 100;
  let red, green, blue;

  // Red
  if (temp <= 66) {
    red = 255;
  } else {
    red = temp - 60;
    red = 329.698727446 * Math.pow(red, -0.1332047592);
    if (red < 0) red = 0;
    if (red > 255) red = 255;
  }

  // Green
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

  // Blue
  if (temp >= 66) {
    blue = 255;
  } else if (temp <= 19) {
    blue = 0;
  } else {
    blue = temp - 10;
    blue = 138.5177312231 * Math.log(blue) - 305.0447927307;
    if (blue < 0) blue = 0;
    if (blue > 255) blue = 255;
  }

  const r = Math.round(red).toString(16).padStart(2, '0');
  const g = Math.round(green).toString(16).padStart(2, '0');
  const b = Math.round(blue).toString(16).padStart(2, '0');
  return `#${r}${g}${b}`;
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
// 9. INITIALIZATION
// ============================================================================

window.addEventListener('DOMContentLoaded', () => {
  // Sync loaded IPs to display
  DOM.ipDisplayWizLeft.textContent = appState.network.wizLeftIp;
  DOM.ipDisplayWizRight.textContent = appState.network.wizRightIp;

  bindEvents();
  initCommunications();
  renderAll();
});
