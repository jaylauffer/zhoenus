import unreal


BUTTON_WIDGET_ASSETS = [
    ("/Game/Blueprints/UI/ZhoenusButton", "ZhoenusButton_C"),
    ("/Game/Blueprints/UI/Z_RepeatButton", "Z_RepeatButton_C"),
]
BUTTON_STYLE_CLASS_PATH = "/Game/Blueprints/UI/ZhoenusRepeatButtonStyle.ZhoenusRepeatButtonStyle_C"
BUTTON_TEXT_STYLE_CLASS_PATH = "/Game/Blueprints/UI/ZhoenusCommonTextStyle.ZhoenusCommonTextStyle_C"
ROOT_WIDGET_ASSET = "/Game/Blueprints/UI/PowerUpRoot"
ROOT_PARENT_CLASS_PATH = "/Script/Zhoenus.PowerUpRootUI"
STAT_WIDGET_ASSET = "/Game/Blueprints/UI/PowerUpStatWidget"
STAT_PARENT_CLASS_PATH = "/Script/Zhoenus.PowerUpStatWidgetUI"
REPORT_PATH = "/Volumes/Micro-Data-Transfer/Zhoenus/zhoenus/Saved/patch_menu_focus_and_button_style.txt"

WIDGET_PATCHES = {
    "/Game/Blueprints/UI/PowerUpScreenWidget": [
        "ZhoenusButton_Again",
        "ZhoenusButton_Convert",
        "ZhoenusButton_MainMenu",
        "ZhoenusButton_PowerUp",
    ],
    "/Game/Blueprints/UI/AdjustShip": [
        "ZhoenusSaveButton",
        "ZhoenusBackButton",
    ],
    "/Game/Blueprints/UI/ConvertSpeed": [
        "ZhoenusButton_Decrement_Forward",
        "ZhoenusButton_Increment_Forward",
        "ZhoenusButton_Decrement_Reverse",
        "ZhoenusButton_Increment_Reverse",
    ],
}


lines = []


def emit(message):
    lines.append(message)
    unreal.log_warning(message)


def save_report():
    with open(REPORT_PATH, "w", encoding="utf-8") as report:
        report.write("\n".join(lines) + "\n")


def configure_button_instance(button):
    button.set_is_focusable(True)
    button.set_is_selectable(True)
    button.set_is_interactable_when_selected(True)
    button.set_should_select_upon_receiving_focus(True)


def make_slate_color(r, g, b, a=1.0):
    return unreal.SlateColor(specified_color=unreal.LinearColor(r, g, b, a))


def configure_brush(brush, fill_rgba, outline_rgba, outline_width):
    brush.set_editor_property("draw_as", unreal.SlateBrushDrawType.ROUNDED_BOX)
    brush.set_editor_property("tint_color", make_slate_color(*fill_rgba))

    outline_settings = brush.get_editor_property("outline_settings")
    outline_settings.set_editor_property("color", make_slate_color(*outline_rgba))
    outline_settings.set_editor_property("width", outline_width)
    brush.set_editor_property("outline_settings", outline_settings)
    return brush


def patch_widget_blueprint(asset_path, widget_names):
    widget_blueprint = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not widget_blueprint:
        raise RuntimeError(f"Failed to load widget blueprint: {asset_path}")

    cdo = unreal.get_default_object(unreal.load_object(None, asset_path + "." + asset_path.split("/")[-1] + "_C"))
    if cdo:
        try:
            cdo.set_editor_property("is_focusable", True)
            emit(f"WIDGET_FOCUSABLE asset={asset_path} value=True")
        except Exception as exc:
            emit(f"WIDGET_FOCUSABLE_ERROR asset={asset_path} error={exc}")

    focus_graph_name = "BP_GetDesiredFocusTarget"
    focus_graph = unreal.BlueprintEditorLibrary.find_graph(widget_blueprint, focus_graph_name)
    if focus_graph:
        unreal.BlueprintEditorLibrary.remove_function_graph(widget_blueprint, focus_graph_name)
        emit(f"REMOVED_FOCUS_GRAPH asset={asset_path}")
    else:
        emit(f"NO_FOCUS_GRAPH asset={asset_path}")

    for widget_name in widget_names:
        widget = unreal.EditorUtilityLibrary.find_source_widget_by_name(widget_blueprint, widget_name)
        if not widget:
            emit(f"MISSING_WIDGET asset={asset_path} name={widget_name}")
            continue

        configure_button_instance(widget)
        emit(f"PATCHED_WIDGET asset={asset_path} name={widget_name}")

    unreal.BlueprintEditorLibrary.compile_blueprint(widget_blueprint)
    unreal.EditorAssetLibrary.save_loaded_asset(widget_blueprint)
    emit(f"COMPILED_AND_SAVED asset={asset_path}")


def patch_button_widget():
    style_class = unreal.load_class(None, BUTTON_STYLE_CLASS_PATH)
    text_style_class = unreal.load_class(None, BUTTON_TEXT_STYLE_CLASS_PATH)
    if not style_class:
        raise RuntimeError(f"Failed to load button style class: {BUTTON_STYLE_CLASS_PATH}")

    if not text_style_class:
        emit(f"MISSING_TEXT_STYLE class={BUTTON_TEXT_STYLE_CLASS_PATH}")

    style_cdo = unreal.get_default_object(style_class)
    if not style_cdo:
        raise RuntimeError(f"Failed to load button style CDO: {BUTTON_STYLE_CLASS_PATH}")

    style_cdo.set_editor_property(
        "normal_base",
        configure_brush(style_cdo.get_editor_property("normal_base"), (0.02, 0.05, 0.10, 0.98), (0.12, 0.77, 1.0, 1.0), 3.0),
    )
    style_cdo.set_editor_property(
        "normal_hovered",
        configure_brush(style_cdo.get_editor_property("normal_hovered"), (0.08, 0.18, 0.30, 1.0), (0.38, 0.92, 1.0, 1.0), 4.0),
    )
    style_cdo.set_editor_property(
        "normal_pressed",
        configure_brush(style_cdo.get_editor_property("normal_pressed"), (0.01, 0.03, 0.08, 1.0), (0.42, 0.97, 1.0, 1.0), 4.0),
    )
    style_cdo.set_editor_property(
        "selected_base",
        configure_brush(style_cdo.get_editor_property("selected_base"), (0.99, 0.97, 0.58, 1.0), (0.01, 0.05, 0.12, 1.0), 8.0),
    )
    style_cdo.set_editor_property(
        "selected_hovered",
        configure_brush(style_cdo.get_editor_property("selected_hovered"), (1.0, 0.99, 0.74, 1.0), (0.01, 0.05, 0.12, 1.0), 8.0),
    )
    style_cdo.set_editor_property(
        "selected_pressed",
        configure_brush(style_cdo.get_editor_property("selected_pressed"), (0.94, 0.84, 0.22, 1.0), (0.01, 0.05, 0.12, 1.0), 8.0),
    )

    if text_style_class:
        style_cdo.set_editor_property("normal_text_style", text_style_class)
        style_cdo.set_editor_property("selected_text_style", text_style_class)
        style_cdo.set_editor_property("disabled_text_style", text_style_class)
    emit(f"PATCHED_BUTTON_STYLE class={style_class.get_path_name()}")

    for button_widget_asset, button_class_name in BUTTON_WIDGET_ASSETS:
        widget_blueprint = unreal.EditorAssetLibrary.load_asset(button_widget_asset)
        if not widget_blueprint:
            raise RuntimeError(f"Failed to load widget blueprint: {button_widget_asset}")

        button_class = unreal.load_object(None, button_widget_asset + "." + button_class_name)
        cdo = unreal.get_default_object(button_class) if button_class else None
        if not cdo:
            raise RuntimeError(f"Failed to load button CDO: {button_widget_asset}")

        cdo.set_editor_property("style", style_class)
        cdo.set_editor_property("is_focusable", True)
        cdo.set_is_selectable(True)
        cdo.set_is_interactable_when_selected(True)
        cdo.set_should_select_upon_receiving_focus(True)
        unreal.BlueprintEditorLibrary.compile_blueprint(widget_blueprint)
        unreal.EditorAssetLibrary.save_loaded_asset(widget_blueprint)
        emit(f"PATCHED_BUTTON_WIDGET asset={button_widget_asset} style={style_class.get_path_name()}")

    unreal.EditorAssetLibrary.save_loaded_asset(unreal.EditorAssetLibrary.load_asset("/Game/Blueprints/UI/ZhoenusRepeatButtonStyle"))
    emit("COMPILED_AND_SAVED asset=/Game/Blueprints/UI/ZhoenusRepeatButtonStyle")


def patch_root_widget():
    widget_blueprint = unreal.EditorAssetLibrary.load_asset(ROOT_WIDGET_ASSET)
    if not widget_blueprint:
        raise RuntimeError(f"Failed to load widget blueprint: {ROOT_WIDGET_ASSET}")

    new_parent_class = unreal.load_class(None, ROOT_PARENT_CLASS_PATH)
    if not new_parent_class:
        raise RuntimeError(f"Failed to load root parent class: {ROOT_PARENT_CLASS_PATH}")

    unreal.BlueprintEditorLibrary.reparent_blueprint(widget_blueprint, new_parent_class)

    cdo = unreal.get_default_object(unreal.load_object(None, ROOT_WIDGET_ASSET + ".PowerUpRoot_C"))
    if cdo:
        try:
            cdo.set_editor_property("is_focusable", True)
            emit(f"ROOT_FOCUSABLE asset={ROOT_WIDGET_ASSET} value=True")
        except Exception as exc:
            emit(f"ROOT_FOCUSABLE_ERROR asset={ROOT_WIDGET_ASSET} error={exc}")
        try:
            cdo.set_editor_property("auto_activate", True)
            emit(f"ROOT_AUTO_ACTIVATE asset={ROOT_WIDGET_ASSET} value=True")
        except Exception as exc:
            emit(f"ROOT_AUTO_ACTIVATE_ERROR asset={ROOT_WIDGET_ASSET} error={exc}")

    focus_graph_name = "BP_GetDesiredFocusTarget"
    focus_graph = unreal.BlueprintEditorLibrary.find_graph(widget_blueprint, focus_graph_name)
    if focus_graph:
        unreal.BlueprintEditorLibrary.remove_function_graph(widget_blueprint, focus_graph_name)
        emit(f"REMOVED_ROOT_FOCUS_GRAPH asset={ROOT_WIDGET_ASSET}")
    else:
        emit(f"NO_ROOT_FOCUS_GRAPH asset={ROOT_WIDGET_ASSET}")

    unreal.BlueprintEditorLibrary.compile_blueprint(widget_blueprint)
    unreal.EditorAssetLibrary.save_loaded_asset(widget_blueprint)
    emit(f"REPARENTED_ROOT asset={ROOT_WIDGET_ASSET} parent={new_parent_class.get_path_name()}")
    emit(f"COMPILED_AND_SAVED asset={ROOT_WIDGET_ASSET}")


def patch_stat_widget():
    widget_blueprint = unreal.EditorAssetLibrary.load_asset(STAT_WIDGET_ASSET)
    if not widget_blueprint:
        raise RuntimeError(f"Failed to load widget blueprint: {STAT_WIDGET_ASSET}")

    new_parent_class = unreal.load_class(None, STAT_PARENT_CLASS_PATH)
    if not new_parent_class:
        raise RuntimeError(f"Failed to load stat parent class: {STAT_PARENT_CLASS_PATH}")

    unreal.BlueprintEditorLibrary.reparent_blueprint(widget_blueprint, new_parent_class)

    cdo = unreal.get_default_object(unreal.load_object(None, STAT_WIDGET_ASSET + ".PowerUpStatWidget_C"))
    if cdo:
        try:
            cdo.set_editor_property("is_focusable", True)
            emit(f"STAT_FOCUSABLE asset={STAT_WIDGET_ASSET} value=True")
        except Exception as exc:
            emit(f"STAT_FOCUSABLE_ERROR asset={STAT_WIDGET_ASSET} error={exc}")

    focus_graph_name = "BP_GetDesiredFocusTarget"
    focus_graph = unreal.BlueprintEditorLibrary.find_graph(widget_blueprint, focus_graph_name)
    if focus_graph:
        unreal.BlueprintEditorLibrary.remove_function_graph(widget_blueprint, focus_graph_name)
        emit(f"REMOVED_STAT_FOCUS_GRAPH asset={STAT_WIDGET_ASSET}")
    else:
        emit(f"NO_STAT_FOCUS_GRAPH asset={STAT_WIDGET_ASSET}")

    for widget_name in ["RB_Decrement", "RB_Increment"]:
        widget = unreal.EditorUtilityLibrary.find_source_widget_by_name(widget_blueprint, widget_name)
        if not widget:
            emit(f"MISSING_WIDGET asset={STAT_WIDGET_ASSET} name={widget_name}")
            continue

        configure_button_instance(widget)
        emit(f"PATCHED_WIDGET asset={STAT_WIDGET_ASSET} name={widget_name}")

    unreal.BlueprintEditorLibrary.compile_blueprint(widget_blueprint)
    unreal.EditorAssetLibrary.save_loaded_asset(widget_blueprint)
    emit(f"REPARENTED_STAT asset={STAT_WIDGET_ASSET} parent={new_parent_class.get_path_name()}")
    emit(f"COMPILED_AND_SAVED asset={STAT_WIDGET_ASSET}")


patch_button_widget()
patch_root_widget()
patch_stat_widget()
for asset_path, widget_names in WIDGET_PATCHES.items():
    patch_widget_blueprint(asset_path, widget_names)

save_report()
