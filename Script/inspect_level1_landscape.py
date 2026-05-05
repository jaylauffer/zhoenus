import os

import unreal


LEVEL_ASSET_PATH = "/Game/Map/Level-1"
REPORT_PATH = os.path.join(unreal.Paths.project_saved_dir(), "inspect_level1_landscape.txt")


def emit(lines, message):
    lines.append(message)
    unreal.log(message)


def safe_prop(obj, name):
    try:
        return obj.get_editor_property(name)
    except Exception:
        return None


def main():
    lines = []

    loaded = unreal.EditorLevelLibrary.load_level(LEVEL_ASSET_PATH)
    emit(lines, f"LOAD level={LEVEL_ASSET_PATH} success={loaded}")
    if not loaded:
        raise RuntimeError(f"Failed to load level: {LEVEL_ASSET_PATH}")

    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    emit(lines, f"ACTORS total={len(actors)}")

    landscape_actors = []
    for actor in actors:
        class_name = actor.get_class().get_name()
        if "Landscape" in class_name:
            landscape_actors.append(actor)

    emit(lines, f"LANDSCAPES total={len(landscape_actors)}")

    for actor in landscape_actors:
        location = actor.get_actor_location()
        rotation = actor.get_actor_rotation()
        scale = actor.get_actor_scale3d()
        origin, extent = actor.get_actor_bounds(False)

        emit(lines, f"LANDSCAPE name={actor.get_name()} class={actor.get_class().get_name()}")
        emit(lines, f"  location={location}")
        emit(lines, f"  rotation={rotation}")
        emit(lines, f"  scale={scale}")
        emit(lines, f"  bounds_origin={origin} bounds_extent={extent}")

        for prop_name in (
            "component_size_quads",
            "subsection_size_quads",
            "component_num_subsections",
            "negative_z_bounds_extension",
            "positive_z_bounds_extension",
        ):
            emit(lines, f"  {prop_name}={safe_prop(actor, prop_name)}")

        components = safe_prop(actor, "landscape_components") or []
        emit(lines, f"  landscape_components={len(components)}")

        section_bases = []
        for component in components:
            section_base_x = safe_prop(component, "section_base_x")
            section_base_y = safe_prop(component, "section_base_y")
            if section_base_x is not None and section_base_y is not None:
                section_bases.append((section_base_x, section_base_y))

        if section_bases:
            xs = [point[0] for point in section_bases]
            ys = [point[1] for point in section_bases]
            unique_x = sorted(set(xs))
            unique_y = sorted(set(ys))
            emit(lines, f"  section_base_x_range=({min(xs)}, {max(xs)}) unique={len(unique_x)}")
            emit(lines, f"  section_base_y_range=({min(ys)}, {max(ys)}) unique={len(unique_y)}")
            emit(lines, f"  unique_x={unique_x}")
            emit(lines, f"  unique_y={unique_y}")

    with open(REPORT_PATH, "w", encoding="utf-8") as handle:
        handle.write("\n".join(lines) + "\n")


if __name__ == "__main__":
    main()
