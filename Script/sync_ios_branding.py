#!/usr/bin/env python3
"""Sync Zhoenus iOS branding into Unreal's project and engine resource paths."""

from __future__ import annotations

from pathlib import Path
import plistlib
import re
import shutil
import subprocess
import tempfile


PROJECT_ROOT = Path(__file__).resolve().parents[1]
ENGINE_ROOT = PROJECT_ROOT.parent / "unreal"

PROJECT_GRAPHICS = PROJECT_ROOT / "Build" / "IOS" / "Resources" / "Graphics"
PROJECT_MAC_ICON = PROJECT_ROOT / "Build" / "Mac" / "Application.icns"
ENGINE_GRAPHICS = ENGINE_ROOT / "Engine" / "Build" / "IOS" / "Resources" / "Graphics"
ENGINE_ASSET_CATALOG = ENGINE_ROOT / "Engine" / "Build" / "IOS" / "Resources" / "Assets.xcassets"
ENGINE_APPICONSET = ENGINE_ASSET_CATALOG / "AppIcon.appiconset"

ENGINE_GRAPHICS_FILES = (
    "Icon60@2x.png",
    "Icon76@2x.png",
    "Icon83.5@2x.png",
    "Icon1024.png",
    "LaunchScreenIOS.png",
)

ASSET_CATALOG_ICONS = {
    "Icon60@2x.png": "IPhoneIcon60@2x.png",
    "Icon76@2x.png": "IPadIcon76@2x.png",
    "Icon83.5@2x.png": "IPadIcon83.5@2x.png",
    "Icon1024.png": "Icon1024.png",
}

APP_LOOSE_ICONS = {
    "Icon60@2x.png": "AppIcon60x60@2x.png",
    "Icon76@2x.png": "AppIcon76x76@2x~ipad.png",
}


def copy_file(source: Path, target: Path) -> None:
    if not source.exists():
        raise FileNotFoundError(source)
    target.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(source, target)


def sync_engine_resources() -> None:
    for filename in ENGINE_GRAPHICS_FILES:
        copy_file(PROJECT_GRAPHICS / filename, ENGINE_GRAPHICS / filename)

    for source_name, target_name in ASSET_CATALOG_ICONS.items():
        copy_file(PROJECT_GRAPHICS / source_name, ENGINE_APPICONSET / target_name)


def compile_asset_catalog(output_dir: Path) -> bool:
    if not ENGINE_ASSET_CATALOG.exists() or not shutil.which("xcrun"):
        return False

    output_dir.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="zhoenus-actool-") as tmp_dir:
        partial_info = Path(tmp_dir) / "assetcatalog-info.plist"
        command = [
            "xcrun",
            "actool",
            "--compile",
            str(output_dir),
            "--platform",
            "iphoneos",
            "--minimum-deployment-target",
            "17.0",
            "--target-device",
            "iphone",
            "--target-device",
            "ipad",
            "--app-icon",
            "AppIcon",
            "--output-partial-info-plist",
            str(partial_info),
            str(ENGINE_ASSET_CATALOG),
        ]
        subprocess.run(command, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        return True


def current_signing_authority(app_dir: Path) -> str | None:
    result = subprocess.run(
        ["codesign", "-dvvv", str(app_dir)],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        check=False,
    )
    match = re.search(r"^Authority=(Apple Development:[^\n]+)$", result.stdout, re.MULTILINE)
    return match.group(1) if match else None


def identity_hash_for_authority(authority: str) -> str | None:
    result = subprocess.run(
        ["security", "find-identity", "-v", "-p", "codesigning"],
        stdout=subprocess.PIPE,
        stderr=subprocess.DEVNULL,
        text=True,
        check=False,
    )
    for line in result.stdout.splitlines():
        match = re.match(r'\s*\d+\)\s+([0-9A-F]+)\s+"(.+)"', line)
        if match and match.group(2) == authority:
            return match.group(1)
    return None


def resign_app(app_dir: Path) -> None:
    authority = current_signing_authority(app_dir)
    if not authority:
        return

    identity_hash = identity_hash_for_authority(authority)
    if not identity_hash:
        raise RuntimeError(f"Could not find a local codesigning identity for {authority}")

    subprocess.run(
        [
            "codesign",
            "--force",
            "--sign",
            identity_hash,
            "--preserve-metadata=identifier,entitlements,requirements",
            str(app_dir),
        ],
        check=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )


def icon_filename_for_mac_app(app_dir: Path) -> str:
    info_plist = app_dir / "Contents" / "Info.plist"
    if not info_plist.exists():
        return "Application.icns"

    with info_plist.open("rb") as plist_file:
        info = plistlib.load(plist_file)

    icon_file = info.get("CFBundleIconFile") or "Application"
    icon_name = str(icon_file)
    if not icon_name.endswith(".icns"):
        icon_name += ".icns"
    return icon_name


def sync_mac_outputs() -> None:
    if not PROJECT_MAC_ICON.exists():
        return

    app_dirs = [
        PROJECT_ROOT / "Binaries" / "Mac" / "Zhoenus.app",
        PROJECT_ROOT / "Saved" / "StagedBuilds" / "Mac" / "Zhoenus.app",
    ]

    for app_dir in app_dirs:
        if not app_dir.exists():
            continue

        resources_dir = app_dir / "Contents" / "Resources"
        copy_file(PROJECT_MAC_ICON, resources_dir / icon_filename_for_mac_app(app_dir))
        resign_mac_app(app_dir)


def resign_mac_app(app_dir: Path) -> None:
    for temp_file in (app_dir / "Contents" / "MacOS").glob("*.cstemp"):
        temp_file.unlink()

    subprocess.run(
        ["codesign", "--force", "--deep", "--sign", "-", str(app_dir)],
        check=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )


def sync_generated_outputs() -> None:
    app_dirs = [
        PROJECT_ROOT / "Binaries" / "IOS" / "Zhoenus.app",
        PROJECT_ROOT / "Saved" / "StagedBuilds" / "IOS" / "Zhoenus.app",
    ]

    for app_dir in app_dirs:
        if not app_dir.exists():
            continue

        for source_name, target_name in APP_LOOSE_ICONS.items():
            copy_file(PROJECT_GRAPHICS / source_name, app_dir / target_name)
        copy_file(PROJECT_GRAPHICS / "LaunchScreenIOS.png", app_dir / "LaunchScreenIOS.png")
        compile_asset_catalog(app_dir)
        resign_app(app_dir)

    thinned_dirs = [
        PROJECT_ROOT / "Binaries" / "Zhoenus (IOS).build" / "IOS" / "Zhoenus.build" / "assetcatalog_output" / "thinned",
        PROJECT_ROOT / "Saved" / "StagedBuilds" / "Zhoenus (IOS).build" / "IOS" / "Zhoenus.build" / "assetcatalog_output" / "thinned",
    ]

    for thinned_dir in thinned_dirs:
        if not thinned_dir.exists():
            continue

        for source_name, target_name in APP_LOOSE_ICONS.items():
            copy_file(PROJECT_GRAPHICS / source_name, thinned_dir / target_name)
        compile_asset_catalog(thinned_dir)


def main() -> None:
    sync_engine_resources()
    sync_generated_outputs()
    sync_mac_outputs()


if __name__ == "__main__":
    main()
