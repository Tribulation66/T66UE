import importlib.util
import json
from pathlib import Path


ROOT = Path(r"C:\UE\T66\Model Generation\Experiments\Pixal3D_Experiment2")
EXP1_SCRIPT = Path(
    r"C:\UE\T66\Model Generation\Experiments\Pixal3D_Goblin_Characterization\Analysis\blender_render_extract.py"
)
VARIANTS = ("D", "E")


spec = importlib.util.spec_from_file_location("exp1_blender_render_extract", EXP1_SCRIPT)
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)

all_metadata = {}
for variant in VARIANTS:
    all_metadata[variant] = module.process_variant(ROOT, variant, 1024)

(ROOT / "Analysis" / "blender_batch_metadata.json").write_text(
    json.dumps(all_metadata, indent=2), encoding="ascii"
)
