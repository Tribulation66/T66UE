import unreal

names = [name for name in dir(unreal) if "Niagara" in name]
for name in sorted(names):
    unreal.log(f"[NiagaraPythonApi] {name}")
