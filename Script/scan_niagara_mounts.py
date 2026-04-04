import unreal
reg = unreal.AssetRegistryHelpers.get_asset_registry()
for p in ['/Niagara', '/Engine', '/NiagaraEditor', '/FX', '/NiagaraExtras']:
    try:
        reg.scan_paths_synchronous([p], True, False)
    except Exception as e:
        pass
lines = []
for needle in ['SimpleSpriteBurst', 'SingleLoopingParticle', 'LocationBasedRibbon']:
    lines.append(f'-- {needle} --')
    for a in reg.get_all_assets():
        an = str(a.asset_name)
        if needle.lower() in an.lower():
            lines.append(f'name={an} package_name={a.package_name} package_path={a.package_path} class={a.asset_class_path.asset_name}')
with open('/var/folders/x0/vq6dbxcs3wn5cf9lhgr03g180000gn/T/scan_niagara_mounts.txt', 'w') as f:
    f.write('\n'.join(lines))
