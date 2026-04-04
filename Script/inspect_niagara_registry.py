import unreal

out = []
reg = unreal.AssetRegistryHelpers.get_asset_registry()

for path in ['/Game/Textures/M_triangulation', '/Game/Textures/M_AimProjector']:
    obj = unreal.EditorAssetLibrary.load_asset(path)
    out.append(f'{path} -> {(obj.get_class().get_name() if obj else "None")}')

all_assets = reg.get_all_assets()
for needle in ['SimpleSpriteBurst', 'SingleLoopingParticle', 'M_AimProjector', 'M_triangulation']:
    hits = [a for a in all_assets if needle.lower() in str(a.asset_name).lower() or needle.lower() in a.object_path.string.lower()]
    out.append(f'search {needle}: {[h.object_path.string + " class=" + str(h.asset_class_path.asset_name) for h in hits[:20]]}')

with open('/tmp/zhoenus_niagara_registry.txt', 'w') as f:
    f.write('\n'.join(out))
for line in out:
    unreal.log(line)
