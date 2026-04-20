const REFERENCE_WIDTH = 1920;
const REFERENCE_HEIGHT = 1080;
const root = document.getElementById("screen-root");
const TAU = Math.PI * 2;

const backgroundLayers = {
  backlight: document.querySelector(".screen__bg-backlight"),
  fog: document.querySelector(".screen__bg-fog"),
  pyramid: document.querySelector(".screen__bg-layer--pyramid"),
};

function fitScreenToViewport() {
  const width = window.innerWidth;
  const height = window.innerHeight;
  const scale = Math.min(width / REFERENCE_WIDTH, height / REFERENCE_HEIGHT);
  root.style.setProperty("--screen-scale", scale.toString());
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
    swayAmplitudeY: 3,
    swayFrequency: 0.08,
    scalePulseAmplitude: 0.022,
    scalePulseFrequency: 0.12,
    opacityMin: 0.76,
    opacityMax: 0.92,
    opacityFrequency: 0.22,
    phaseOffset: 0.18,
  }, timeSeconds);

  applyLayerMotion(backgroundLayers.pyramid, {
    swayAmplitudeX: 0.8,
    swayAmplitudeY: 0.8,
    swayFrequency: 0.045,
    scalePulseAmplitude: 0.003,
    scalePulseFrequency: 0.08,
    opacityMin: 0.97,
    opacityMax: 1.00,
    opacityFrequency: 0.06,
    phaseOffset: 0.41,
  }, timeSeconds);

  applyLayerMotion(backgroundLayers.fog, {
    swayAmplitudeX: 9,
    swayAmplitudeY: 3,
    swayFrequency: 0.035,
    scalePulseAmplitude: 0.016,
    scalePulseFrequency: 0.06,
    opacityMin: 0.44,
    opacityMax: 0.66,
    opacityFrequency: 0.09,
    phaseOffset: 0.63,
  }, timeSeconds);

  window.requestAnimationFrame(animateBackground);
}

fitScreenToViewport();
window.addEventListener("resize", fitScreenToViewport);
window.requestAnimationFrame(animateBackground);
