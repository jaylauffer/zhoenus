import unreal


ADJUST_SHIP_ASSET = "/Game/Blueprints/UI/AdjustShip"
STAT_WIDGET_ASSET = "/Game/Blueprints/UI/PowerUpStatWidget"
REPORT_PATH = "/Volumes/Micro-Data-Transfer/Zhoenus/zhoenus/Saved/patch_adjustship_runtime.txt"

ADJUST_SHIP_REMOVE_GRAPHS = [
    "BP_OnActivated",
]

STAT_WIDGET_REMOVE_GRAPHS = [
    "GetStatValue",
    "SetStatValue",
]

lines = []


def emit(message):
    lines.append(message)
    unreal.log_warning(message)


def remove_graph_if_present(widget_blueprint, graph_name, asset_path):
    graph = unreal.BlueprintEditorLibrary.find_graph(widget_blueprint, graph_name)
    if graph:
        unreal.BlueprintEditorLibrary.remove_function_graph(widget_blueprint, graph_name)
        emit(f"REMOVED_GRAPH asset={asset_path} graph={graph_name}")
    else:
        emit(f"GRAPH_NOT_FOUND asset={asset_path} graph={graph_name}")


def patch_widget(asset_path, graphs_to_remove):
    widget_blueprint = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not widget_blueprint:
        raise RuntimeError(f"Failed to load widget blueprint: {asset_path}")

    for graph_name in graphs_to_remove:
        remove_graph_if_present(widget_blueprint, graph_name, asset_path)

    unreal.BlueprintEditorLibrary.compile_blueprint(widget_blueprint)
    unreal.EditorAssetLibrary.save_loaded_asset(widget_blueprint)
    emit(f"COMPILED_AND_SAVED asset={asset_path}")


patch_widget(ADJUST_SHIP_ASSET, ADJUST_SHIP_REMOVE_GRAPHS)
patch_widget(STAT_WIDGET_ASSET, STAT_WIDGET_REMOVE_GRAPHS)

with open(REPORT_PATH, "w", encoding="utf-8") as report:
    report.write("\n".join(lines) + "\n")
