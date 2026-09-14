# Multiplayer Edit

<img src="logo.png" width="150" alt="Multiplayer Edit logo" />

Real-time collaborative level editing for Geometry Dash! Host a session, invite your friends with a room code, and build levels together in real-time.

## [Join the Official Discord Server!](https://discord.gg/mdsuxYu2YP)

## Features

- **Real-Time Collaborative Editing:** Host a session, share the room code, and build together.
- **Instant Synchronization:** Placement, deletion, rotation, scaling, and movement are updated across all connected clients.
- **Isolated Undo/Redo Stacks:** Action history is tracked per player, meaning your undo/redo actions won't overwrite other players' actions.
- **Player Cursors & Badges:** Track player movements live in the editor. Badges display next to player cursors showing the specific object they currently have selected.
- **Playtesting icon Sync:** Watch players playtest in the editor.
- **Smart Object Locking:** Automatically locks selected objects to prevent multiple players from editing the same objects, ensuring no race conditions or crashes.
- **Session HUD & Player List:** Keep track of room details with a player list and status overlay showing the active session code. You can also see player cube icons next to their names.
- **In-Game Chat:** Press `/` to quickly send a message in the editor, or use the full chat menu. Messages appear directly above your cursor.
- **View-Only Mode:** Lock players into view-only mode to prevent griefing, or set your room to default to view-only for public showcases.
- **Camera Jumping:** Click the view button next to a player's name in the player list to instantly jump to their camera position.
- **Dedicated Servers:** Host headless servers and allow players to connect directly via IP using the Servers menu.

## How to Use

### Installation

1. Make sure you have [Geode](https://geode-sdk.org/) installed.
2. Go to the [Releases page](https://github.com/xXoanon/MultiplayerEdit/releases) and download the latest `.geode` file.
3. Place the `.geode` file in your Geometry Dash `geode/mods/` folder.
4. Restart Geometry Dash.

### Using the Mod
1. Open any level in the editor.
2. Click the **Multiplayer Edit** button (in the pause menu if you're hosting, or on the "my levels" page if you're joining).
3. Host a room, type in a friend's room code to join them, use the room browser, or click the **Servers** button to join a dedicated server by IP.

## Building from source

You'll need the [Geode SDK and CLI](https://docs.geode-sdk.org/).

```sh
git clone https://github.com/xXoanon/MultiplayerEdit.git
cd MultiplayerEdit
geode build
```

To build for specific platforms (like Android, or Windows on Linux):
```sh
geode build --platform win
geode build --platform android64
```

## Dedicated Servers & Signaling

Multiplayer Edit supports two different types of servers:

### 1. P2P Signaling Server (Cloudflare Workers)
When players host a room in-game using a room code, the mod uses WebRTC to establish a direct Peer-to-Peer connection. To find each other, it uses a signaling server. By default, the mod connects to a free public signaling server (`https://multiplayer-edit.d050.workers.dev`), but you can host your own.
See the [servers/signaling/README.md](servers/signaling/README.md) file for setup instructions. You can update the **Signaling Server URL** setting in the mod settings to use your custom server.

### 2. Dedicated Servers (Node.js)
You can run a standalone, headless server that hosts a level 24/7 without needing Geometry Dash open or even installed. Players connect directly to it using an IP address. 
See the [servers/dedicated/README.md](servers/dedicated/README.md) file for setup and hosting instructions.

## Support Development

If you enjoy this mod or any of my other projects, consider supporting future development on [Patreon](https://www.patreon.com/c/d050/membership)!