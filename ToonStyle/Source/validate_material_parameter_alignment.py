"""Validate ToonStyle material parameter alignment for Pixal3D imports.

Run through Unreal Python so the live material asset can be queried:

UnrealEditor-Cmd.exe C:/UE/T66/T66.uproject -ExecutePythonScript=C:/UE/T66/ToonStyle/Source/validate_material_parameter_alignment.py -NullRHI -unattended -nop4 -nosplash
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

import unreal


SOURCE_DIR = Path(__file__).resolve().parent
REPO_ROOT = SOURCE_DIR.parents[1]
MATERIAL_SPEC = REPO_ROOT / "ToonStyle" / "Docs" / "MaterialSpec.md"
DEFAULT_OUTPUT = REPO_ROOT / "Saved" / "Codex" / "ToonStyle" / "StandardProcessAudit" / "MaterialParameterAlignment.json"

if str(SOURCE_DIR) not in sys.path:
    sys.path.insert(0, str(SOURCE_DIR))

from ImportPixal3DAsset_Phase1C import (  # noqa: E402
    CHARACTER_PARENT_MATERIAL,
    _TOON_CHARACTER_PARAMETERS,
    get_material_parameter_names,
)


def parse_args() -> argparse.Namespace:
    argv = list(sys.argv[1:])
    if "--" in argv:
        argv = argv[argv.index("--") + 1 :]
    parser = argparse.ArgumentParser(description="Validate M_Toon_Character parameter alignment.")
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    args, _unknown = parser.parse_known_args(argv)
    return args


def main() -> int:
    args = parse_args()
    live = get_material_parameter_names(CHARACTER_PARENT_MATERIAL)
    errors: list[str] = []

    for group, configured_names in _TOON_CHARACTER_PARAMETERS.items():
        live_group = set(live.get(group, []))
        for name in configured_names:
            if name not in live_group:
                errors.append(f"_TOON_CHARACTER_PARAMETERS[{group}] includes {name}, but the live master does not expose it")

    spec_text = MATERIAL_SPEC.read_text(encoding="utf-8")
    spec_missing: dict[str, list[str]] = {"textures": [], "scalars": [], "vectors": []}
    for group, live_names in live.items():
        for name in live_names:
            if f"`{name}`" not in spec_text:
                spec_missing.setdefault(group, []).append(name)
                errors.append(f"MaterialSpec.md does not list live M_Toon_Character {group[:-1]} parameter {name}")

    report = {
        "ok": not errors,
        "parent_material": CHARACTER_PARENT_MATERIAL,
        "configured_importer_parameters": _TOON_CHARACTER_PARAMETERS,
        "live_material_parameters": live,
        "material_spec_path": str(MATERIAL_SPEC),
        "material_spec_missing": spec_missing,
        "errors": errors,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    unreal.log(f"[validate_material_parameter_alignment] Wrote {args.output}")
    if errors:
        raise RuntimeError("Material parameter alignment failed:\n" + "\n".join(errors))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
