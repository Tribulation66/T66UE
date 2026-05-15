# HY-World Research Boundary

This is the compact surviving world-generation research note after the `World Generation` root folder cleanup.

## Current T66 Decision

- Treat HY-World 2.0 as an offline world-generation and reconstruction research pipeline, not as a replacement for T66 runtime procedural terrain.
- Do not build production architecture around unreleased Tencent modules.
- The current production-facing path is modular environment-kit generation: generate wall, floor, ceiling, trim, arch, pillar, grate, rubble, and relief modules, then let the existing runtime assemble rooms from authored modules.
- Generated visual meshes stay separate from gameplay collision. T66 uses explicit hidden simple collision proxies for generated dungeon surfaces.

## Release Reality Snapshot

As of the prior April 2026 research pass:

- WorldMirror 2.0 code and weights were the public runnable path.
- The full world-generation inference stack, HY-Pano 2.0, WorldNav, and WorldStereo 2.0 were still marked as coming soon in the official repo.
- Secondary summaries may overstate public availability; prefer official source checks before planning implementation.

## Official Sources To Recheck

- [Tencent-Hunyuan/HY-World-2.0](https://github.com/Tencent-Hunyuan/HY-World-2.0)
- [HY-World 2.0 technical report](https://3d-models.hunyuan.tencent.com/world/world2_0/HY_World_2_0.pdf)
- [Project page](https://3d-models.hunyuan.tencent.com/world/)
- [Scene-to-3D page](https://3d.hunyuan.tencent.com/sceneTo3D)
- [Hugging Face model page](https://huggingface.co/tencent/HY-World-2.0)

## Local T66 Docs

- [MODULAR_DUNGEON_KIT_INSTRUCTIONS.md](MODULAR_DUNGEON_KIT_INSTRUCTIONS.md): current environment-kit production process.
- [Model Generation routing instructions](../../Model%20Generation/Instructions/00_MODEL_GENERATION_ROUTING_INSTRUCTIONS.md): model-generation decision tree.
- [TRELLIS RunPod setup instructions](../../Model%20Generation/Instructions/01_TRELLIS_RUNPOD_SETUP_INSTRUCTIONS.md): TRELLIS/RunPod baseline.
- [Unreal import and validation instructions](../../Model%20Generation/Instructions/05_UNREAL_IMPORT_AND_VALIDATION_INSTRUCTIONS.md): import, cook, and standalone validation rules.
