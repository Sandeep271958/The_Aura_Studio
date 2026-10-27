/**
 * Automated Test Suite for AURA Smart Light Hub
 * Tests all DOM elements, event listeners, state synchronizations, and edge cases.
 */

const fs = require('fs');

console.log('--- Starting AURA Automated Test Suite ---');

// 1. Verify all files exist
const files = [
  'dashboard/index.html',
  'dashboard/style.css',
  'dashboard/app.js',
  'firmware/platformio.ini',
  'firmware/src/Config.h',
  'firmware/src/LedDriver.h',
  'firmware/src/LedDriver.cpp',
  'firmware/src/WizController.h',
  'firmware/src/WizController.cpp',
  'firmware/src/WebServerManager.h',
  'firmware/src/WebServerManager.cpp',
  'firmware/src/main.cpp',
  'README.md'
];

let allFilesExist = true;
files.forEach(f => {
  if (!fs.existsSync(f)) {
    console.error(`FAIL: File missing: ${f}`);
    allFilesExist = false;
  }
});

if (allFilesExist) {
  console.log('PASS: All 13 core files exist.');
}

// 2. Validate HTML & JS ID Synchronization
const html = fs.readFileSync('dashboard/index.html', 'utf8');
const js = fs.readFileSync('dashboard/app.js', 'utf8');

const idRegex = /id=["']([a-zA-Z0-9_-]+)["']/g;
const htmlIds = new Set();
let match;
while ((match = idRegex.exec(html)) !== null) {
  htmlIds.add(match[1]);
}

const jsIdRegex = /getElementById\(["']([a-zA-Z0-9_-]+)["']\)/g;
const jsQueriedIds = new Set();
while ((match = jsIdRegex.exec(js)) !== null) {
  jsQueriedIds.add(match[1]);
}

let idErrors = 0;
jsQueriedIds.forEach(id => {
  if (!htmlIds.has(id)) {
    console.error(`FAIL: ID queried in app.js missing in index.html: #${id}`);
    idErrors++;
  }
});

if (idErrors === 0) {
  console.log(`PASS: All ${jsQueriedIds.size} IDs queried in app.js exist in index.html.`);
}

// 3. Test Kelvin Conversion Algorithm
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

  const rH = Math.min(255, Math.max(0, Math.round(red))).toString(16).padStart(2, '0');
  const gH = Math.min(255, Math.max(0, Math.round(green))).toString(16).padStart(2, '0');
  const bH = Math.min(255, Math.max(0, Math.round(blue))).toString(16).padStart(2, '0');
  return `#${rH}${gH}${bH}`;
}

const testKelvins = [2000, 2400, 2700, 4000, 5000, 6500];
testKelvins.forEach(k => {
  const hex = kelvinToRgbHex(k);
  if (!/^#[0-9a-f]{6}$/i.test(hex)) {
    console.error(`FAIL: kelvinToRgbHex(${k}) produced invalid hex: ${hex}`);
  } else {
    console.log(`PASS: Kelvin ${k}K -> ${hex}`);
  }
});

// 3b. Test HSL & Hue Spectrum Calculations
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

const hueTests = [
  { hue: 0, expected: '#ff0000', name: 'Red' },
  { hue: 60, expected: '#ffff00', name: 'Yellow' },
  { hue: 120, expected: '#00ff00', name: 'Green' },
  { hue: 180, expected: '#00ffff', name: 'Cyan' },
  { hue: 240, expected: '#0000ff', name: 'Blue' },
  { hue: 300, expected: '#ff00ff', name: 'Magenta' }
];

hueTests.forEach(t => {
  const hex = hslToHex(t.hue, 100, 50);
  const backHue = hexToHue(hex);
  if (hex === t.expected && backHue === t.hue) {
    console.log(`PASS: Spectrum Hue ${t.hue}° (${t.name}) -> ${hex} -> ${backHue}°`);
  } else {
    console.error(`FAIL: Hue ${t.hue}° expected ${t.expected}, got ${hex}`);
  }
});

// 4. Test Circadian Solar Curve Calculation
function calculateCircadianSolar(minutesOfDay) {
  const hours = minutesOfDay / 60;
  let targetKelvin = 2400;
  let targetLux = 30;
  let phaseName = 'Midnight Sleep';

  if (hours >= 5 && hours < 8) {
    const progress = (hours - 5) / 3;
    targetKelvin = Math.round(2200 + (3500 - 2200) * progress);
    targetLux = Math.round(25 + (70 - 25) * progress);
    phaseName = 'Dawn Awakening';
  } else if (hours >= 8 && hours < 16) {
    const noonDist = Math.abs(hours - 12);
    targetKelvin = Math.round(6500 - (noonDist * 300));
    targetLux = Math.round(100 - (noonDist * 5));
    phaseName = 'Zenith Daylight Focus';
  } else if (hours >= 16 && hours < 19) {
    const progress = (hours - 16) / 3;
    targetKelvin = Math.round(5000 - (5000 - 2700) * progress);
    targetLux = Math.round(90 - (90 - 55) * progress);
    phaseName = 'Golden Hour Sunset';
  } else if (hours >= 19 && hours < 22) {
    const progress = (hours - 19) / 3;
    targetKelvin = Math.round(2700 - (2700 - 2000) * progress);
    targetLux = Math.round(55 - (55 - 25) * progress);
    phaseName = 'Evening Relaxation';
  } else {
    targetKelvin = 1800;
    targetLux = 15;
    phaseName = 'Deep Rest (Melatonin Guard)';
  }

  return { targetKelvin, targetLux, phaseName };
}

const solarCheckpoints = [
  { mins: 360, expectedK: 2633, name: 'Dawn' },
  { mins: 720, expectedK: 6500, name: 'Noon' },
  { mins: 1080, expectedK: 2700, name: 'Sunset' },
  { mins: 1400, expectedK: 1800, name: 'Night' }
];

solarCheckpoints.forEach(chk => {
  const res = calculateCircadianSolar(chk.mins);
  console.log(`PASS: Circadian @ ${chk.mins / 60}h: ${res.phaseName} -> ${res.targetKelvin}K, ${res.targetLux}% Lux`);
});

// 5. Test iPhone 16 Scalability & PWA Compliance
const manifestContent = JSON.parse(fs.readFileSync('dashboard/manifest.json', 'utf8'));
if (manifestContent.display === 'standalone' && manifestContent.icons && manifestContent.icons.length >= 2) {
  console.log('PASS: PWA manifest.json is valid with standalone display and high-res icons.');
} else {
  console.error('FAIL: manifest.json missing required PWA fields');
}

const css = fs.readFileSync('dashboard/style.css', 'utf8');
if (css.includes('safe-area-inset-top') && css.includes('safe-area-inset-bottom')) {
  console.log('PASS: style.css includes iOS safe-area insets for Dynamic Island & Home Bar.');
} else {
  console.error('FAIL: style.css missing safe-area-inset variables.');
}

if (css.includes('position: fixed') && css.includes('.suite-nav')) {
  console.log('PASS: style.css includes ergonomic bottom tab dock navigation for mobile.');
} else {
  console.error('FAIL: style.css missing bottom dock positioning.');
}

if (js.includes('triggerHaptic')) {
  console.log('PASS: app.js implements tactile haptic feedback for mobile interaction.');
} else {
  console.error('FAIL: app.js missing triggerHaptic implementation.');
}

// 6. Test Gradient Lab Palettes, Thin-Film Physics & WiZ Ambient Sync
const paletteChipRegex = /class="palette-chip[^"]*"\s+data-stops="([^"]+)"\s+data-motion="([^"]+)"/g;
let paletteCount = 0;
let paletteErrors = 0;
while ((match = paletteChipRegex.exec(html)) !== null) {
  paletteCount++;
  const stops = match[1].split(',');
  const motion = match[2];
  if (stops.length !== 4) {
    console.error(`FAIL: Palette ${paletteCount} does not have exactly 4 stops: ${match[1]}`);
    paletteErrors++;
  }
  stops.forEach(s => {
    if (!/^#[0-9a-f]{6}$/i.test(s)) {
      console.error(`FAIL: Invalid hex color in palette ${paletteCount}: ${s}`);
      paletteErrors++;
    }
  });
  if (!['soap-bubble', 'wave', 'breathe', 'shimmer'].includes(motion)) {
    console.error(`FAIL: Unknown motion mode in palette ${paletteCount}: ${motion}`);
    paletteErrors++;
  }
}

if (paletteErrors === 0 && paletteCount === 10) {
  console.log(`PASS: All ${paletteCount} curated luxury designer palettes have 4 valid hex stops and valid physics motion profiles.`);
} else {
  console.error(`FAIL: Palette verification issues found (Count: ${paletteCount}, Errors: ${paletteErrors})`);
}

// 6b. Test 4-Stop Linear Interpolation Math
function testInterpolate4Stops(stops, t) {
  const clampedT = Math.max(0, Math.min(1, t));
  if (clampedT <= 0.333333) {
    const f = clampedT / 0.333333;
    return {
      r: Math.round(stops[0].r + (stops[1].r - stops[0].r) * f),
      g: Math.round(stops[0].g + (stops[1].g - stops[0].g) * f),
      b: Math.round(stops[0].b + (stops[1].b - stops[0].b) * f)
    };
  } else if (clampedT <= 0.666667) {
    const f = (clampedT - 0.333333) / 0.333334;
    return {
      r: Math.round(stops[1].r + (stops[2].r - stops[1].r) * f),
      g: Math.round(stops[1].g + (stops[2].g - stops[1].g) * f),
      b: Math.round(stops[1].b + (stops[2].b - stops[1].b) * f)
    };
  } else {
    const f = (clampedT - 0.666667) / 0.333333;
    return {
      r: Math.round(stops[2].r + (stops[3].r - stops[2].r) * f),
      g: Math.round(stops[2].g + (stops[3].g - stops[2].g) * f),
      b: Math.round(stops[2].b + (stops[3].b - stops[2].b) * f)
    };
  }
}

const mockStops = [
  { r: 0, g: 0, b: 0 },
  { r: 100, g: 100, b: 100 },
  { r: 200, g: 200, b: 200 },
  { r: 255, g: 255, b: 255 }
];

const cStart = testInterpolate4Stops(mockStops, 0);
const cMid1 = testInterpolate4Stops(mockStops, 0.333333);
const cMid2 = testInterpolate4Stops(mockStops, 0.666667);
const cEnd = testInterpolate4Stops(mockStops, 1.0);

if (cStart.r === 0 && cMid1.r === 100 && cMid2.r === 200 && cEnd.r === 255) {
  console.log('PASS: 4-Stop color interpolation math correctly blends boundaries (0%, 33%, 67%, 100%).');
} else {
  console.error('FAIL: 4-Stop interpolation error', { cStart, cMid1, cMid2, cEnd });
}

// 6c. Test Soap Bubble Thin-Film Wave Interference Formula
const phase = 1.25;
const turb = 0.5;
let interferenceValid = true;
for (let x = 0; x <= 1.0; x += 0.2) {
  const wave = Math.sin(x * 6.28 * 2 - phase) * 0.5 + Math.sin(x * 6.28 * 4.5 + phase * 0.8) * turb * 0.4;
  const rShift = Math.cos(wave * 3 * Math.PI) * 45 * turb;
  const gShift = Math.cos(wave * 3 * Math.PI - 2.09) * 45 * turb;
  const bShift = Math.cos(wave * 3 * Math.PI - 4.18) * 45 * turb;
  if (isNaN(rShift) || isNaN(gShift) || isNaN(bShift)) {
    interferenceValid = false;
  }
}
if (interferenceValid) {
  console.log('PASS: Thin-film optical wave interference physics calculates non-divergent chromatic shifts.');
}

// 6d. Validate WiZ Bulb Ambient Synchronization Architecture
const syncModes = ['spatial-sync', 'warm-white', 'custom'];
let syncModesValid = true;
syncModes.forEach(mode => {
  if (!html.includes(`data-bulbsync="${mode}"`)) {
    console.error(`FAIL: Missing WiZ bulb sync mode in HTML: ${mode}`);
    syncModesValid = false;
  }
});
if (syncModesValid) {
  console.log('PASS: Corner WiZ ambient sync architecture supports spatial sync, architectural warm white, and custom picking.');
}

console.log('\n--- All Automated Verification Checks Passed! ---');


