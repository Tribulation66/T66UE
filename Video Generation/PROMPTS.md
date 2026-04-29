# Video Generation Prompts

## Kling Video 3.0 Web Prompt

Use with the same image as start frame and end frame.

```text
Locked camera cinematic game main menu background. The golden idol statue and gold altar remain perfectly still, centered, and unchanged. The composition stays identical to the input image. Only the environment animates: tiny stars softly twinkle in place across the black sky, and the fiery eclipse ring behind the idol flickers like solar plasma with slow flame licking and glowing corona motion. Subtle warm gold glints shimmer on the polished gold surfaces. Slow elegant ambient motion, seamless loop feeling.
```

Negative prompt, if Kling exposes a negative prompt box:

```text
camera movement, zoom, pan, statue movement, face morphing, altar deformation, changing geometry, melting gold, water, text, UI, logo, smoke covering idol, glitch artifacts, distortion, blur, extra objects
```

## Motion Brush / Static Brush Prompt

Use when the idol/altar is protected by a static mask.

```text
Locked camera cinematic game main menu background. The golden idol statue, black eclipse center, and gold altar remain perfectly still, centered, and unchanged. Only the unmasked environment animates: tiny stars softly twinkle in place across the black sky, and the fiery eclipse corona flickers like solar plasma with slow flame licking and glowing heat. Subtle warm gold reflections shimmer gently without changing the statue shape. Seamless loop feeling, slow elegant ambient motion.
```

Negative prompt:

```text
camera movement, zoom, pan, statue movement, face morphing, altar deformation, changing geometry, melting gold, water, text, UI, logo, smoke covering idol, glitch artifacts, distortion, blur, extra objects
```

## Prompt Rules For This Asset

- Say "locked camera" explicitly.
- Say "idol statue and altar remain perfectly still" explicitly.
- Do not ask for big cinematic camera motion.
- Do not ask for water.
- Do not use "sun rises", "camera pushes in", "orbit", "reveal", or "parallax".
- Keep duration short during tests: 5 seconds.
- Prefer subtle motion over dramatic motion.
