import unreal
reg = unreal.AssetRegistryHelpers.get_asset_registry()
for needle in ['SimpleSpriteBurst', 'SingleLoopingParticle', 'M_AimProjector', 'M_triangulation']:
    unreal.log(f'-- {needle} --')
    for a in reg.get_all_assets():
        an = str(a.asset_name)
        if needle.lower() in an.lower():
            unreal.log(f'name={an} package={a.package_name} path={a.package_path} class={a.asset_class_path.asset_name}')
