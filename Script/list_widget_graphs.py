import unreal

ASSETS = [
    "/Game/Blueprints/UI/AdjustShip",
    "/Game/Blueprints/UI/PowerUpStatWidget",
]

for asset_path in ASSETS:
    blueprint = unreal.EditorAssetLibrary.load_asset(asset_path)
    unreal.log_warning(f"ASSET {asset_path}")
    if not blueprint:
        unreal.log_warning("  FAILED_TO_LOAD")
        continue
    graphs = unreal.BlueprintEditorLibrary.get_all_graphs(blueprint)
    for graph in graphs:
        unreal.log_warning(f"  GRAPH {graph.get_name()}")
