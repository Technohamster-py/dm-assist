# DM Assist 🎲

**A tool for Masters (DM/GM) of tabletop role-playing games**

[![Version](https://img.shields.io/github/v/release/Technohamster-py/dm-assist)](https://github.com/Technohamster-py/dm-assist/releases)
[![C++ Standard](https://img.shields.io/badge/C%2B%2B-17-blue)](https://github.com/Technohamster-py/dm-assist)
[![Qt Version](https://img.shields.io/badge/Qt-6.0%2B-brightgreen.svg)](https://www.qt.io/)

**DM Assist** is a modern, intuitive application designed to make gaming easier and more exciting. The project started as a convenient music player for changing tracks in one click, but has grown into a full-fledged system for managing music, combat, maps, and campaigns.

> 🚀 **Current version:** v1.4.1 (last released on March 11, 2026)

---

## 📚 Table of Contents

- [Main Features](#-main-features)
- [Supported systems](#-supported-gaming-systems)
- [Installation](#-installation)
- [Quick start](#-quick-start tutorial)
- [Planned functions](#-planned-functions)
- [Known Issues](#-known-issues)
- [Project Support](#-project-support)
- [Acknowledgements](#-acknowledgements)

---

## ✨ Main features

| Module                      | Description                                                                                                                                            |
|-----------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------|
| 🎵 **Music**                | Multi-channel player with independent volume, playlists and hotkeys (`Ctrl + playlist number').                                                        |
| ⚔️ **Initiative Tracker**   | Battle management: hits, AC, statuses, sorting, player sharing, and field arithmetic (for example, `10+5-2`).                                          |
| 🗺️ **A map with tools**    | Fog of War, dynamic lighting, ruler (taking into account heights!), grid, spell shapes, drawing and tokens.                                            |
| 📁 **Campaigns**            | All in one place: maps, characters, encounters, and playlists. Just open the campaign folder and continue playing.                                     |
| 📄 **Character Editor**     | Compatible with [LSS](https://longstoryshort.app /), automatic calculation of bonuses, resources (spells/cells), notes with formatting (`Ctrl+B/I/U`). |
| 🎲 **The Throws Widget**    | Support for the notation `ndX+Y' (in Russian and English), cube grouping mode, integration with the character sheet.                                   |
| 🌍 **Shared access**        | Separate windows for players with tracker and map (synchronization with the wizard).                                                                   |


## 🎮 Supported game systems

The application (in particular, the bestiary and character sheets) is designed for **D&D 5e**, but flexible tools (statuses, map, throws) allow it to be used for most TTRPGS.

---

## , Installation

**Supported OS:** Windows 10/11, Linux (built for Ubuntu).

1. Go to [releases page](https://github.com/Technohamster-py/dm-assist/releases ).
2. Download the latest stable version:
- For Windows: '.exe` installer.
- For Linux: `.tar.gz `or `zip` archive.
3. Install the downloaded file:
- **Windows:** Run the installer and follow the instructions.
- **Linux:** Unzip the archive and run the `DM-assist` executable file.

---

# Quick Start (Tutorial)

### 🎛️ Settings ('Ctrl+Alt+S' or 'Campaign → Settings`)

**General:**
- **Language:** Russian / English (you can [add your own localization](https://github.com/Technohamster-py/dm-assist/wiki/Loacalization )).
- **Audio output:** Select a device. Use a virtual cable to stream music to Discord.
- **Check for updates** at startup.
- **Launch action:** Choose whether to open an empty window, last campaign, or a list of recent campaigns.

**Appearance:**
- **Theme:** You can choose from the pre-installed ones or download your own `.xml` theme.
- **Style:** You can also add your own Qss style files. [More details](https://github.com/Technohamster-py/AdaptiveThemeLib )
> It is not recommended to use the windows/ windows 11 style in conjunction with a dark theme due to incorrect color processing

**Initiative Tracker:**
- Configure which fields are visible to the players (shared access).
- Select the format for displaying health to players
- Enable automatic throws and automatic sorting.
- Adjust the backlight color of the active character or leave the preset theme

**The map**
- Configure the display of tokens
- Adjust the transparency and color of the fog
- Adjust the transparency of textures
- Determine the size of one grid cell by default
- Decide whether to open the latest maps automatically when loading the campaign

**Keyboard shortcuts:** Adjust the keys for the map tools.

### 🎵 Music

![music-widget.png](resources/illustrations/music-widget.png)

- **Launch:** the ▶️ button or `Ctrl + playlist number'.
- **Volume:** separately under each playlist + a shared master slider.
- **Playlist editing:** button ✏️ - add, delete, change the order of tracks. The files are copied to the working folder.

![playlist-edit.png](resources/illustrations/playlist-edit.png)

### ⚔️ Initiative Tracker

![tracker.png](resources/illustrations/tracker.png)

- **Columns:** Initiative, speed, AC, current/maximum HP, statuses.
- **HP input:** you can write expressions `10+5-2` — they will be calculated automatically.
- **Management:** `PageUp` / `PageDown' to change the active character, **Sort** to sort, **Add** to add.
- **Statuses:** Double-click on a cell. There are standard D&D statuses and their own (with SVG icons).

![status-edit.png](resources/illustrations/status-edit.png)
- **Shared access:** Share button — a window opens for players with customizable fields and the HP display mode (numbers / status/ progress bar).

### 🗺️ Map and tools

![map-main.png](resources/illustrations/map-main.png)

- **Open a card:** `Campaign → Add → Map' or the '+` button on the tabs. Formats: `JPG, PNG, BMP, DAM'.
- **Navigation:** LKM (without tools) or SCM — movement, wheel — zoom.
- **Tools** (deactivation — `Esc', removal of `PCM` elements):


| Tool                                                     | Action                                                                                          |
|----------------------------------------------------------|-------------------------------------------------------------------------------------------------|
| 📏 **Ruler**                                             | Calibration (PCM → set the segment and its length), then measuring distances based on heights.  |
| 💡 **Light**                                             | Add sources with color and radius. The "update fog" option automatically opens the area.        |
| 🔲 **Forms**                                             | Circles, lines, squares, triangles (like spells in D&D). They are drawn with two clicks.        |
| ✏️ **Brush** / Free drawing with color and transparency. |
| ✂️ **Lasso**                                             | Closed contours of any shape.                                                                   |
| 🗻 **Elevation map**                                     | Areas with heights from -100 to 100 (gradient blue→red). Affects the measurements of the ruler. |
| 🔲 **Grid**                                              | Square or hexagonal, with adjustable cell size.                                                 |

- **Shared access:** Click on the maps → "share" tab. The player's window syncs with your actions.

> The card is automatically saved in the `dam` format when closed if it was opened as part of a campaign. You can also manually save the map by clicking on the maps tab and selecting save.

### 📁 Campaigns

![campaign-tree.png](resources/illustrations/campaign-tree.png)

**Create:** `Campaign → New' (`Ctrl+N') → specify a name and an empty folder.  
**Open:** `Campaign → Open` (`Ctrl+O') → select the folder with `campaign.json`.

After opening:
- The campaign tree appears on the left (characters, maps, encounters, playlists).
- Buttons for the elements: add to encounter, open editor, open map.
- **Tokens:** Drag the character from the tree onto the map. Take the images from the `Tokens` folder (you can replace them manually).
- **Reload** campaign: `Campaign → Reload from disk' (after adding files manually).

### 📄 Character Editor

![charsheet.png](resources/illustrations/charsheet.png)

- Opens from the campaign tree ( ✏️ button).
- Compatible with the LSS format (not backwards — the character from DM Assist may not open on LSS).
- Stats, attack and damage bonuses are calculated automatically.
- **Resources** (spell cells, etc.): a table with a recovery mode (short/long rest).
- **The rest buttons** restore resources according to the settings.
- **Text formatting** in notes: `Ctrl+B' (bold), `Ctrl+I` (italics), `Ctrl+U` (underlined).

### 📖 Bestiary

![bestiary.png](resources/illustrations/bestiary.png)

Opens monster stats in **FVTT11** and **FVTT12** format (can be downloaded, for example, from [TTG Club](https://ttg.club /)).
The information on the TTG website may differ from the file.

### 🎲 The Throws widget

![roll-widget.png](resources/illustrations/roll-widget.png)

- Standard notation is supported: `2d6+4', `1k20+5-1d4'.
- You can use the characters `d` and `k` instead of `d'.
- **Compact mode:** Groups identical dice (`1d4+3d4 → 4d4`).
  -**Character integration:** click on **the name** of a characteristic, skill, attack/damage bonus — the roll is automatically sent to the widget.

---

## 📅 Planned functions

### D&D 5e
- [ ] Character storage with fast leveling (selection of upgrades, as in BG3).
- [ ] Multiclass support.

### Other systems
The plans include special modes for:
- [ ] Call of Cthulhu
- [ ] Cyberpunk (RED, 2020)
- [ ] World of Darkness (MtA, VtM, etc.)
- [ ] Pathfinder / Starfinder

---

## 💖 Project support

You can support the development with a voluntary donation. This does not affect the functionality in any way, but it helps the project a lot.  
[Support link](https://technohamster.taplink.ws)

---

## 🙏 Thanks

- **TTG Club** — for the icons used in the initiative table.
- To all contributors and users who report bugs and suggest ideas.

---

**Happy GMing! 🐉**