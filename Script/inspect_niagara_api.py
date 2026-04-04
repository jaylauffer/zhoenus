import pathlib
import unreal


OUTPUT_PATH = pathlib.Path("/tmp/inspect_niagara_api.txt")
lines = []


def emit(text):
    lines.append(text)
    unreal.log_warning(text)


def dump_editor_properties(label, obj):
    emit(f"== {label} ==")
    if obj is None:
        emit("None")
        return
    emit(f"class: {obj.get_class().get_name()}")
    try:
        emit(f"editor props: {obj.get_editor_property_names()}")
    except Exception as exc:
        emit(f"editor props error: {exc}")
    emit(
        "dir sample: "
        + str([name for name in dir(obj) if "emit" in name.lower() or "render" in name.lower() or "material" in name.lower()][:100])
    )


factory = unreal.NiagaraSystemFactoryNew()
dump_editor_properties("factory", factory)
for factory_property in [
    "EmittersToAddToNewSystem",
    "emitters_to_add_to_new_system",
    "SystemToCopy",
    "system_to_copy",
]:
    try:
        emit(f"factory property {factory_property}: {factory.get_editor_property(factory_property)}")
    except Exception as exc:
        emit(f"factory property {factory_property} error: {exc}")

asset = unreal.EditorAssetLibrary.load_asset("/Game/Effects/NS_AimProjection")
dump_editor_properties("system", asset)

if asset:
    try:
        emitter_handles = None
        for property_name in ["EmitterHandles", "emitter_handles"]:
            try:
                emitter_handles = asset.get_editor_property(property_name)
                emit(f"system property {property_name} resolved")
                break
            except Exception as exc:
                emit(f"system property {property_name} error: {exc}")
        if emitter_handles is None:
            raise RuntimeError("no emitter handles property resolved")
        emit(f"emitter handles count: {len(emitter_handles)}")
        for index, emitter_handle in enumerate(emitter_handles):
            emit(f"-- handle {index}: {type(emitter_handle)}")
            for property_name in ["Name", "name", "Id", "id", "VersionedInstance", "versioned_instance"]:
                try:
                    value = emitter_handle.get_editor_property(property_name)
                except Exception as exc:
                    emit(f"handle {index} property {property_name} error: {exc}")
                    continue
                emit(f"handle {index} property {property_name}: {value}")
            try:
                versioned_instance = None
                for property_name in ["VersionedInstance", "versioned_instance"]:
                    try:
                        versioned_instance = emitter_handle.get_editor_property(property_name)
                        emit(f"handle {index} versioned instance property {property_name} resolved")
                        break
                    except Exception as exc:
                        emit(f"handle {index} versioned instance property {property_name} error: {exc}")
                if versioned_instance is None:
                    raise RuntimeError("no versioned instance property resolved")

                emitter = None
                for property_name in ["Emitter", "emitter"]:
                    try:
                        emitter = versioned_instance.get_editor_property(property_name)
                        emit(f"versioned instance emitter property {property_name} resolved")
                        break
                    except Exception as exc:
                        emit(f"versioned instance emitter property {property_name} error: {exc}")
                if emitter is None:
                    raise RuntimeError("no emitter property resolved")
                emit(f"handle {index} emitter: {emitter}")
                version_data = None
                for property_name in ["VersionData", "version_data"]:
                    try:
                        version_data = emitter.get_editor_property(property_name)
                        emit(f"emitter version data property {property_name} resolved")
                        break
                    except Exception as exc:
                        emit(f"emitter version data property {property_name} error: {exc}")
                if version_data is None:
                    raise RuntimeError("no version data property resolved")
                emit(f"handle {index} version_data count: {len(version_data)}")
                for version_index, emitter_data in enumerate(version_data):
                    renderer_properties = None
                    for property_name in ["RendererProperties", "renderer_properties"]:
                        try:
                            renderer_properties = emitter_data.get_editor_property(property_name)
                            emit(f"emitter_data renderer properties {property_name} resolved")
                            break
                        except Exception as exc:
                            emit(f"emitter_data renderer properties {property_name} error: {exc}")
                    if renderer_properties is None:
                        raise RuntimeError("no renderer properties resolved")
                    emit(
                        f"handle {index} version {version_index} renderer count: {len(renderer_properties)}"
                    )
                    for renderer_index, renderer in enumerate(renderer_properties):
                        emit(
                            f"renderer {renderer_index} class: {renderer.get_class().get_name()}"
                        )
                        for renderer_property in [
                            "Material",
                            "material",
                            "MaterialUserParamBinding",
                            "material_user_param_binding",
                            "Alignment",
                            "alignment",
                            "FacingMode",
                            "facing_mode",
                            "PivotInUVSpace",
                            "pivot_in_uv_space",
                        ]:
                            try:
                                value = renderer.get_editor_property(renderer_property)
                            except Exception as exc:
                                emit(
                                    f"renderer {renderer_index} property {renderer_property} error: {exc}"
                                )
                                continue
                            emit(
                                f"renderer {renderer_index} property {renderer_property}: {value}"
                            )
            except Exception as exc:
                emit(f"handle {index} nested access error: {exc}")
    except Exception as exc:
        emit(f"system emitter_handles error: {exc}")

OUTPUT_PATH.write_text("\n".join(lines))
