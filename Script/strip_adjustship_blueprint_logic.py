import unreal


REPORT_PATH = "/Volumes/Micro-Data-Transfer/Zhoenus/zhoenus/Saved/strip_adjustship_blueprint_logic.txt"

WIDGETS = {
    "/Game/Blueprints/UI/AdjustShip": {
        "remove_event_graph": True,
        "function_graphs": [
            "BP_OnActivated",
            "GetStatValue",
            "InitStatValue",
        ],
    },
    "/Game/Blueprints/UI/PowerUpStatWidget": {
        "remove_event_graph": True,
        "function_graphs": [
            "GetStatValue",
            "SetStatValue",
            "InitStatValue",
        ],
    },
}


lines = []


def emit(message):
    lines.append(message)
    unreal.log_warning(message)


def remove_named_graph(blueprint, graph_name, asset_path):
    graph = unreal.BlueprintEditorLibrary.find_graph(blueprint, graph_name)
    if not graph:
        emit(f"GRAPH_NOT_FOUND asset={asset_path} graph={graph_name}")
        return

    unreal.BlueprintEditorLibrary.remove_graph(blueprint, graph)
    emit(f"REMOVED_GRAPH asset={asset_path} graph={graph_name}")


def remove_event_graph_if_present(blueprint, asset_path):
    event_graph = unreal.BlueprintEditorLibrary.find_event_graph(blueprint)
    if not event_graph:
        emit(f"EVENT_GRAPH_NOT_FOUND asset={asset_path}")
        return

    unreal.BlueprintEditorLibrary.remove_graph(blueprint, event_graph)
    emit(f"REMOVED_EVENT_GRAPH asset={asset_path} graph={event_graph.get_name()}")


def strip_widget(asset_path, config):
    blueprint = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not blueprint:
        raise RuntimeError(f"Failed to load widget blueprint: {asset_path}")

    if config.get("remove_event_graph"):
        remove_event_graph_if_present(blueprint, asset_path)

    for graph_name in config.get("function_graphs", []):
        remove_named_graph(blueprint, graph_name, asset_path)

    unreal.BlueprintEditorLibrary.remove_unused_nodes(blueprint)
    unreal.BlueprintEditorLibrary.remove_unused_variables(blueprint)
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    unreal.EditorAssetLibrary.save_loaded_asset(blueprint)
    emit(f"COMPILED_AND_SAVED asset={asset_path}")


for asset_path, config in WIDGETS.items():
    strip_widget(asset_path, config)


with open(REPORT_PATH, "w", encoding="utf-8") as report:
    report.write("\n".join(lines) + "\n")
