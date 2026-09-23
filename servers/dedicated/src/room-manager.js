const crypto = require("crypto");
const fs = require("fs");
const path = require("path");
const { Room } = require("./room");
const saveReader = require("./save-reader");

class RoomManager {
  constructor() {
    this.rooms = new Map();
    this.signalingTimer = null;
    this.signalingUrl = "https://dewy-flea-9364.d050.deno.net";
    this.publicListing = false;
    this.manifestPath = path.join(process.cwd(), "rooms.json");
    this.autosaveInterval = 5;
    this.loadTokens();
    this.port = 7575;
  }

  start(port, publicListing) {
    this.port = port;
    this.publicListing = publicListing;
    const levelsDir = path.join(process.cwd(), "levels");
    if (!fs.existsSync(levelsDir)) {
      fs.mkdirSync(levelsDir, { recursive: true });
    }
    if (this.publicListing) {
      this._updateSignalingServer();
      this.signalingTimer = setInterval(
        () => this._updateSignalingServer(),
        60 * 1000,
      );
    }
  }

  stop() {
    clearInterval(this.signalingTimer);
    for (const room of this.rooms.values()) {
      room.destroy();
    }
    this.rooms.clear();
  }

  saveManifest() {
    try {
      const data = {
        version: 1,
        autosaveInterval: this.autosaveInterval !== undefined ? this.autosaveInterval : 5,
        rooms: {}
      };
      for (const room of this.rooms.values()) {
        data.rooms[room.code] = {
          code: room.code,
          levelName: room.levelName,
          ownerToken: room.ownerToken || null,
          password: room.password || "",
          maxPlayers: room.maxPlayers || 0,
          defaultViewOnly: !!room.settings.defaultViewOnly,
          levelFile: room.levelFile || null,
          audioTrack: room.settings.audioTrack || 0,
          songID: room.settings.songID || 0,
          createdAt: room.createdAt || Date.now(),
          lastSavedAt: room.lastSavedAt || Date.now(),
        };
      }
      fs.writeFileSync(this.manifestPath, JSON.stringify(data, null, 2), "utf8");
    } catch (e) {
      console.error("Failed to save rooms.json:", e.message);
    }
  }

  loadManifest() {
    try {
      if (fs.existsSync(this.manifestPath)) {
        const content = fs.readFileSync(this.manifestPath, "utf8").trim();
        if (!content) return null;
        return JSON.parse(content);
      }
    } catch (e) {
      console.error("Failed to load rooms.json:", e.message);
    }
    return null;
  }

  getSavedRoomsCount() {
    const manifest = this.loadManifest();
    if (manifest && manifest.rooms) {
      return Object.keys(manifest.rooms).length;
    }
    return 0;
  }

  restoreRooms() {
    const manifest = this.loadManifest();
    if (!manifest || !manifest.rooms) return [];
    if (manifest.autosaveInterval !== undefined) {
      this.autosaveInterval = manifest.autosaveInterval;
    }
    const levelsDir = path.join(process.cwd(), "levels");
    const restored = [];
    for (const [code, info] of Object.entries(manifest.rooms)) {
      let levelPath = null;
      if (info.levelFile) {
        const directPath = path.isAbsolute(info.levelFile) ? info.levelFile : path.join(levelsDir, path.basename(info.levelFile));
        if (fs.existsSync(directPath)) {
          levelPath = directPath;
        }
      }
      if (!levelPath && fs.existsSync(levelsDir)) {
        const files = fs.readdirSync(levelsDir);
        const match = files.find(f => f.startsWith(`${code}_`) && f.endsWith(".gmd"));
        if (match) {
          levelPath = path.join(levelsDir, match);
        }
      }
      if (!levelPath) {
        console.error(`  \x1b[31m[ERROR]\x1b[0m Cannot find level file for room ${code} ("${info.levelName}")`);
        continue;
      }
      try {
        const xml = fs.readFileSync(levelPath, "utf8");
        const parsed = saveReader.parseGmd(xml);
        if (!parsed || parsed.length === 0) continue;
        const level = parsed[0];
        level.name = info.levelName || level.name;
        level.audioTrack = info.audioTrack !== undefined ? info.audioTrack : level.audioTrack;
        level.songID = info.songID !== undefined ? info.songID : level.songID;
        const room = this.createRoomForLevel(
          level,
          info.maxPlayers || 0,
          info.password || "",
          !!info.defaultViewOnly,
          code
        );
        room.ownerToken = info.ownerToken || null;
        room.createdAt = info.createdAt || Date.now();
        room.lastSavedAt = info.lastSavedAt || Date.now();
        room.levelFile = path.basename(levelPath);
        restored.push(room);
      } catch (e) {
        console.error(`  \x1b[31m[ERROR]\x1b[0m Failed to restore room ${code}:`, e.message);
      }
    }
    this.saveManifest();
    return restored;
  }

  createRoom(levelName, levelData, settings, code = null) {
    const room = new Room(levelName, levelData, settings, code);
    room.onSnapshotSaved = (r) => {
      const suffix = r.isAutoSaving ? "_autosave" : "_save";
      r.isAutoSaving = false;
      this._saveRoomToDisk(r, suffix);
    };
    this.rooms.set(room.code, room);
    return room;
  }

  createRoomForLevel(level, maxPlayers, password, defaultViewOnly, code = null) {
    console.log(`  Decoding level data for "${level.name}"...`);
    const decoded = saveReader.decodeLevelString(level.levelString);
    const settings = {
      saveString: decoded.settings,
      audioTrack: level.audioTrack,
      songID: level.songID,
      levelLength: 0,
      levelName: level.name,
      password: password,
      defaultViewOnly: defaultViewOnly,
    };
    const objectCount = decoded.objects.split(";").filter(Boolean).length;
    const uuids = Array.from({ length: objectCount }, () =>
      crypto.randomUUID(),
    );
    const room = this.createRoom(
      level.name,
      { compressedBytes: Buffer.from(decoded.objects, "utf8"), uuids: uuids },
      settings,
      code,
    );
    room.maxPlayers = maxPlayers;
    room.password = password;
    this._saveRoomToDisk(room, "_save");
    this.saveManifest();
    return room;
  }

  loadTokens() {
    this.tokens = new Set();
    try {
      if (fs.existsSync("tokens.json")) {
        const data = JSON.parse(fs.readFileSync("tokens.json", "utf8"));
        data.forEach((t) => this.tokens.add(t));
      }
    } catch (e) {
      console.error("Failed to load tokens.json:", e.message);
    }
  }

  saveTokens() {
    try {
      fs.writeFileSync(
        "tokens.json",
        JSON.stringify(Array.from(this.tokens), null, 2),
      );
    } catch (e) {
      console.error("Failed to save tokens.json:", e.message);
    }
  }

  addToken(token) {
    this.tokens.add(token);
    this.saveTokens();
  }

  removeToken(token) {
    const res = this.tokens.delete(token);
    this.saveTokens();
    return res;
  }

  isValidToken(token) {
    if (this.tokens.size === 0) return true;
    return this.tokens.has(token);
  }

  getRoom(code) {
    return this.rooms.get(code.toUpperCase());
  }

  deleteRoom(code) {
    const upperCode = code.toUpperCase();
    const room = this.rooms.get(upperCode);
    if (!room) {
      return false;
    }
    room.destroy();
    this.rooms.delete(upperCode);
    this.saveManifest();
    return true;
  }

  getRoomList() {
    const list = [];
    for (const room of this.rooms.values()) {
      list.push({
        roomCode: room.code,
        roomName: room.levelName,
        playerCount: room.players.size,
        playerLimit: room.maxPlayers,
        isPrivate: room.isPrivate,
        hasPassword: !!room.password,
      });
    }
    return list;
  }

  _performAutoSave() {
    for (const room of this.rooms.values()) {
      if (room.players.size > 0 && room.dirty) {
        room.isAutoSaving = true;
        room.requestSnapshot();
      } else if (
        !room.dirty &&
        room.compressedLevelData &&
        room.compressedLevelData.length > 0
      ) {
        this._saveRoomToDisk(room, "_autosave");
      }
    }
  }

  _saveRoomToDisk(room, suffix = "_autosave") {
    try {
      const levelsDir = path.join(process.cwd(), "levels");
      const outFile = saveReader.exportToGmd(room, levelsDir, suffix);
      room.levelFile = path.basename(outFile);
      room.lastSavedAt = Date.now();
      this.saveManifest();
      console.log(
        `  \x1b[35m[SAVE]\x1b[0m Saved level to ${path.basename(outFile)}`,
      );
    } catch (e) {
      console.error(
        `  \x1b[31m[ERROR]\x1b[0m Failed to save level:`,
        e.message,
      );
    }
  }

  async _updateSignalingServer() {
    for (const room of this.rooms.values()) {
      if (room.isPrivate) continue;
      try {
        const body = JSON.stringify({
          hostName: "Dedicated Server",
          playerName: "Dedicated Server",
          roomName: room.levelName,
          description: `Join at ws://<YOUR_IP>:${this.port}`,
          playerLimit: room.maxPlayers,
          isPrivate: false,
          hasPassword: !!room.password,
          version: "Dedicated",
        });
      } catch (e) {}
    }
  }
}

module.exports = { RoomManager };
