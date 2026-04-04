import unreal

paths = [
    '/Game/Textures/M_triangulation',
    '/Game/Textures/M_AimProjector',
    '/Niagara/DefaultAssets/Templates/Emitters/SimpleSpriteBurst',
    '/Niagara/DefaultAssets/Templates/Emitters/SingleLoopingParticle',
]
for path in paths:
    obj = unreal.EditorAssetLibrary.load_asset(path)
    print(path, '->', obj.get_class().get_name() if obj else 'None')

factory = unreal.NiagaraSystemFactoryNew()
print('factory class', factory.get_class().get_name())
print('factory properties', [p for p in dir(factory) if 'system' in p.lower() or 'emitter' in p.lower()][:40])
