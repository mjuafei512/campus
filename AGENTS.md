# Campus - Unreal Engine 5.7 Project

## Project Type
- Unreal Engine 5.7 Blueprint-based project (no C++ Source folder yet)
- Originally based on TP_BlankBP template (redirects in DefaultEngine.ini)

## Critical Paths (gitignored - do not commit)
- `Binaries/` - compiled binaries
- `Build/` - build output
- `DerivedDataCache/` - cached derived data
- `Intermediate/` - intermediate build files
- `Saved/` - logs, saved games, editor state
- `.vs/` - Visual Studio files

## Asset Naming Conventions
| Type | Prefix | Example |
|------|--------|---------|
| Blueprint | `BP_` | `BP_PlayerCharacter` |
| Static Mesh | `SM_` | `SM_Desk_01` |
| Material | `M_` | `M_Wood_Floor` |
| Texture | `T_` | `T_Brick_Wall_D` |
| Animation | `A_` | `A_Player_Run` |
| Sound | `S_` | `S_Footstep_Concrete` |
| UI Widget | `W_` | `W_MainMenuButton` |

## Key Config
- Engine: 5.7 with DX12, ray tracing, Substrate enabled
- Default map: `/Engine/Maps/Templates/OpenWorld`
- Plugin: `ModelingToolsEditorMode` (Editor-only)

## Content Structure
- `Content/` - assets organized by domain (Characters, UI, Audio, etc.)
- `Config/` - engine/game configuration
- `Content/Developers/` - per-developer sandbox (afei, etc.)
- `Content/Collections/` - asset collections
- `Content/接口说明文档/` - Chinese interface documentation

## Development Notes
- No C++ source code yet - purely Blueprint workflow
- Standard UE5 asset workflow (uasset files in Content/)
- CI/CD not configured (no .github/workflows)