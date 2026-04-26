# Loading Screen Layout List

Canvas: 1920x1080.

Fixed structure:
- Full-screen loading surface with no global top bar.
- Centered loading panel.
- Large centered loading text.
- Optional thin progress/activity rail below the loading text.
- Optional small status/tip line below the rail.

Runtime-owned regions:
- Loading text, loading status/tip text, progress or activity state, and any build/session values.

Visual restyle target:
- Use the accepted V2 charcoal/purple UI language: clean planar dark surfaces, crisp purple borders, subtle glow, and restrained readable typography.
- Do not bake dynamic progress, tips, session names, build values, or localized copy into runtime art.
