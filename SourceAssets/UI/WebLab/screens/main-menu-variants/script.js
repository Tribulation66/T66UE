const REFERENCE_WIDTH = 1920;
const REFERENCE_HEIGHT = 1080;
const root = document.getElementById("screen-root");
const screen = document.getElementById("main-menu-screen");
const TAU = Math.PI * 2;

const backgroundLayers = {
  backlight: document.querySelector(".screen__bg-backlight"),
  fog: document.querySelector(".screen__bg-fog"),
  figure: document.querySelector(".screen__bg-layer--figure"),
  floor: document.querySelector(".screen__bg-floor"),
};

const THEMES = {
  "01": {
    name: "Screen 1",
    label: "Megabonk Bastion",
    assetPath: "./assets/screen-05",
    chromePath: "./assets/family-01",
  },
  "02": {
    name: "Screen 2",
    label: "Gilded Reliquary",
    assetPath: "./assets/screen-01",
    chromePath: "./assets/family-02",
  },
  "03": {
    name: "Screen 3",
    label: "Reactor Scrapboard",
    assetPath: "./assets/screen-06",
    chromePath: "./assets/family-03",
  },
  "04": {
    name: "Screen 4",
    label: "Velvet Catacomb",
    assetPath: "./assets/screen-04",
    chromePath: "./assets/family-04",
  },
  "05": {
    name: "Screen 5",
    label: "Frost Ossuary",
    assetPath: "./assets/screen-10",
    chromePath: "./assets/family-05",
  },
};

function fitScreenToViewport() {
  const width = window.innerWidth;
  const height = window.innerHeight;
  const scale = Math.min(width / REFERENCE_WIDTH, height / REFERENCE_HEIGHT);
  root.style.setProperty("--screen-scale", scale.toString());
}

function getThemeId() {
  const params = new URLSearchParams(window.location.search);
  const requested = params.get("screen");

  if (!requested) {
    return "01";
  }

  const normalized = requested.padStart(2, "0");
  return THEMES[normalized] ? normalized : "01";
}

function setCssImageVariable(name, value) {
  screen.style.setProperty(name, `url("${value}")`);
}

function applyTheme(themeId) {
  const theme = THEMES[themeId];
  screen.className = "screen main-menu-screen";
  screen.classList.add(`theme-screen-${themeId}`);
  screen.dataset.screen = themeId;

  setCssImageVariable("--bg-image", `${theme.assetPath}/background.png`);
  setCssImageVariable("--figure-image", `${theme.assetPath}/figure.png`);
  setCssImageVariable("--surface-image", `${theme.assetPath}/surface.png`);
  setCssImageVariable("--topbar-plate", `${theme.chromePath}/topbar.svg`);
  setCssImageVariable("--shell-plate", `${theme.chromePath}/shell.svg`);
  setCssImageVariable("--button-plate", `${theme.chromePath}/button.svg`);
  setCssImageVariable("--title-plate", `${theme.chromePath}/title.svg`);

  document.title = `T66 Main Menu ${theme.name} - ${theme.label}`;
}

function applyLayerMotion(layer, motion, timeSeconds) {
  if (!layer) {
    return;
  }

  const swayX = Math.sin((timeSeconds * motion.swayFrequency + motion.phaseOffset) * TAU) * motion.swayAmplitudeX;
  const swayY = Math.cos((timeSeconds * motion.swayFrequency + motion.phaseOffset) * TAU) * motion.swayAmplitudeY;
  const scale = 1 + Math.sin((timeSeconds * motion.scalePulseFrequency + motion.phaseOffset) * TAU) * motion.scalePulseAmplitude;
  const opacityBlend = (Math.sin((timeSeconds * motion.opacityFrequency + motion.phaseOffset) * TAU) + 1) * 0.5;
  const opacity = motion.opacityMin + opacityBlend * (motion.opacityMax - motion.opacityMin);

  layer.style.transform = `translate3d(${swayX.toFixed(2)}px, ${swayY.toFixed(2)}px, 0) scale(${scale.toFixed(4)})`;
  layer.style.opacity = opacity.toFixed(3);
}

function animateBackground(timestamp) {
  const timeSeconds = timestamp / 1000;

  applyLayerMotion(backgroundLayers.backlight, {
    swayAmplitudeX: 0,
    swayAmplitudeY: 5,
    swayFrequency: 0.05,
    scalePulseAmplitude: 0.05,
    scalePulseFrequency: 0.07,
    opacityMin: 0.56,
    opacityMax: 0.9,
    opacityFrequency: 0.12,
    phaseOffset: 0.18,
  }, timeSeconds);

  applyLayerMotion(backgroundLayers.figure, {
    swayAmplitudeX: 1.2,
    swayAmplitudeY: 1.6,
    swayFrequency: 0.03,
    scalePulseAmplitude: 0.012,
    scalePulseFrequency: 0.045,
    opacityMin: 0.9,
    opacityMax: 1,
    opacityFrequency: 0.08,
    phaseOffset: 0.41,
  }, timeSeconds);

  applyLayerMotion(backgroundLayers.fog, {
    swayAmplitudeX: 14,
    swayAmplitudeY: 5,
    swayFrequency: 0.02,
    scalePulseAmplitude: 0.03,
    scalePulseFrequency: 0.03,
    opacityMin: 0.24,
    opacityMax: 0.62,
    opacityFrequency: 0.05,
    phaseOffset: 0.63,
  }, timeSeconds);

  applyLayerMotion(backgroundLayers.floor, {
    swayAmplitudeX: 0,
    swayAmplitudeY: 2,
    swayFrequency: 0.018,
    scalePulseAmplitude: 0.012,
    scalePulseFrequency: 0.03,
    opacityMin: 0.72,
    opacityMax: 0.94,
    opacityFrequency: 0.04,
    phaseOffset: 0.27,
  }, timeSeconds);

  window.requestAnimationFrame(animateBackground);
}

applyTheme(getThemeId());
fitScreenToViewport();
window.addEventListener("resize", fitScreenToViewport);
window.requestAnimationFrame(animateBackground);
