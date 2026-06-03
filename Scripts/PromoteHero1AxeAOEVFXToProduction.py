"""
Rebuild the accepted Hero 1 axe AOE VFX material/texture assets under /Game/VFX.

This script intentionally reuses the lab setup code through Unreal asset APIs
instead of copying .uasset files. Run the C++ T66Hero1AxeAOEVFX commandlet with
-T66Hero1AxeAOEProduction afterward to rebuild the production mesh/Niagara asset.
"""

import os
import sys

try:
    import unreal
except ImportError:
    unreal = None


def main():
    if unreal is None:
        raise RuntimeError("Unreal Python module is required to promote Hero 1 axe AOE VFX assets.")

    script_dir = os.path.dirname(os.path.abspath(__file__))
    if script_dir not in sys.path:
        sys.path.insert(0, script_dir)

    os.environ["T66_HERO1_AXE_AOE_TARGET"] = "production"
    unreal.log("=== PromoteHero1AxeAOEVFXToProduction ===")

    import SetupHero1AxeAOELabVFX

    SetupHero1AxeAOELabVFX.main()
    unreal.log("=== PromoteHero1AxeAOEVFXToProduction DONE ===")


if __name__ == "__main__":
    main()
