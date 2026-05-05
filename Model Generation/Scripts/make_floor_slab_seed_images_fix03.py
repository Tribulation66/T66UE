import importlib.util
from pathlib import Path


SCRIPT_DIR = Path(__file__).resolve().parent
FIX02_PATH = SCRIPT_DIR / "make_floor_slab_seed_images_fix02.py"

spec = importlib.util.spec_from_file_location("floor_fix02", FIX02_PATH)
floor_fix02 = importlib.util.module_from_spec(spec)
spec.loader.exec_module(floor_fix02)

floor_fix02.OUTPUT_DIR = (
    floor_fix02.RUN_ROOT / "Inputs" / "floor_slab_seed_images_fix03"
)
floor_fix02.MANIFEST_PATH = (
    floor_fix02.OUTPUT_DIR / "floor_slab_seed_manifest_fix03.json"
)
floor_fix02.MONTAGE_PATH = floor_fix02.OUTPUT_DIR / "floor_slab_seed_montage_fix03.png"
floor_fix02.FIX_TAG = "FloorFix03"
floor_fix02.FLOOR_MODULES = [
    "HellFloor_Obsidian_A",
    "HellFloor_EmberFissure_A",
]


if __name__ == "__main__":
    floor_fix02.main()
