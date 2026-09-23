const fs = require('fs');
const path = require('path');
const zlib = require('zlib');
const SAVE_FILE = 'CCLocalLevels.dat';
const XOR_KEY = 11;
function getDefaultSavePaths() {
    const platform = process.platform;
    const home = process.env.HOME || process.env.USERPROFILE || '';
    if (platform === 'win32') {
        const localAppData = process.env.LOCALAPPDATA || path.join(home, 'AppData', 'Local');
        return [path.join(localAppData, 'GeometryDash', SAVE_FILE)];
    }
    if (platform === 'darwin') {
        return [path.join(home, 'Library', 'Application Support', 'GeometryDash', SAVE_FILE)];
    }
    return [
        path.join(home, '.steam', 'steam', 'steamapps', 'compatdata', '322170', 'pfx', 'drive_c',
            'users', 'steamuser', 'AppData', 'Local', 'GeometryDash', SAVE_FILE),
        path.join(home, '.local', 'share', 'Steam', 'steamapps', 'compatdata', '322170', 'pfx', 'drive_c',
            'users', 'steamuser', 'AppData', 'Local', 'GeometryDash', SAVE_FILE),
    ];
}
function findSaveFile(customPath) {
    if (customPath) {
        if (fs.existsSync(customPath)) return customPath;
        throw new Error(`Save file not found at: ${customPath}`);
    }
    for (const p of getDefaultSavePaths()) {
        if (fs.existsSync(p)) return p;
    }
    return null;
}
function decryptSaveFile(filePath) {
    const raw = fs.readFileSync(filePath);
    const xored = Buffer.alloc(raw.length);
    for (let i = 0; i < raw.length; i++) {
        xored[i] = raw[i] ^ XOR_KEY;
    }
    let b64 = xored.toString('ascii');
    b64 = b64.replace(/-/g, '+').replace(/_/g, '/');
    while (b64.length % 4 !== 0) b64 += '=';
    const compressed = Buffer.from(b64, 'base64');
    try {
        return zlib.gunzipSync(compressed).toString('utf-8');
    } catch (e) {
        try {
            return zlib.inflateSync(compressed).toString('utf-8');
        } catch (e2) {
            throw new Error('Failed to decompress save file - might be corrupted');
        }
    }
}
function encryptSaveFile(xmlData, outputPath) {
    const compressed = zlib.gzipSync(Buffer.from(xmlData, 'utf-8'));
    let b64 = compressed.toString('base64');
    b64 = b64.replace(/\+/g, '-').replace(/\//g, '_');
    const xored = Buffer.alloc(b64.length);
    for (let i = 0; i < b64.length; i++) {
        xored[i] = b64.charCodeAt(i) ^ XOR_KEY;
    }
    fs.writeFileSync(outputPath, xored);
}
function parseLevels(xml) {
    const levels = [];
    const levelBlocks = [];
    let searchFrom = 0;
    while (true) {
        const k2Pos = xml.indexOf('<k>k2</k>', searchFrom);
        if (k2Pos === -1) break;
        let depth = 0;
        let dictStart = k2Pos;
        while (dictStart > 0) {
            dictStart--;
            if (xml.substring(dictStart, dictStart + 3) === '<d>') {
                if (depth === 0) break;
                depth--;
            } else if (xml.substring(dictStart, dictStart + 4) === '</d>') {
                depth++;
            }
        }
        let dictEnd = k2Pos;
        depth = 1; 
        let pos = xml.indexOf('>', dictStart) + 1;
        while (pos < xml.length && depth > 0) {
            const nextOpen = xml.indexOf('<d>', pos);
            const nextClose = xml.indexOf('</d>', pos);
            if (nextClose === -1) break;
            if (nextOpen !== -1 && nextOpen < nextClose) {
                depth++;
                pos = nextOpen + 3;
            } else {
                depth--;
                if (depth === 0) {
                    dictEnd = nextClose + 4;
                }
                pos = nextClose + 4;
            }
        }
        levelBlocks.push(xml.substring(dictStart, dictEnd));
        searchFrom = dictEnd;
    }
    for (const block of levelBlocks) {
        const level = parseLevelDict(block);
        if (level) levels.push(level);
    }
    return levels;
}

function parseGmd(xml) {
    return [parseLevelDict(xml)];
}

function parseLevelDict(dictXml) {
    const kvs = {};
    const keyRegex = /<k>([^<]+)<\/k>/g;
    let match;
    const keys = [];
    while ((match = keyRegex.exec(dictXml)) !== null) {
        keys.push({ key: match[1], index: match.index + match[0].length });
    }
    for (let i = 0; i < keys.length; i++) {
        const key = keys[i].key;
        const valueStart = keys[i].index;
        const valueEnd = (i + 1 < keys.length) ? keys[i + 1].index - keys[i + 1].key.length - 7 : dictXml.length;
        const valueStr = dictXml.substring(valueStart, valueEnd).trim();
        const tagMatch = valueStr.match(/^<(\w+)>([\s\S]*?)<\/\1>/);
        if (tagMatch) {
            const tag = tagMatch[1];
            let val = tagMatch[2];
            if (val.startsWith('<![CDATA[') && val.endsWith(']]>')) {
                val = val.substring(9, val.length - 3);
            }
            if (tag === 's') kvs[key] = val;
            else if (tag === 'i') kvs[key] = parseInt(val, 10);
            else if (tag === 'r') kvs[key] = parseFloat(val);
            else kvs[key] = val;
        } else if (valueStr.startsWith('<t />') || valueStr.startsWith('<t/>')) {
            kvs[key] = true;
        } else if (valueStr.startsWith('<f />') || valueStr.startsWith('<f/>')) {
            kvs[key] = false;
        }
    }
    const name = kvs['k2'];
    if (!name) return null;
    const levelString = kvs['k4'] || '';
    const description = kvs['k3'] || '';
    let objectCount = kvs['k48'] || 0;
    if (objectCount === 0 && levelString) {
        const decoded = decodeLevelString(levelString);
        if (decoded && decoded.objects) {
            objectCount = (decoded.objects.match(/;/g) || []).length;
        }
    }
    
    const audioTrack = kvs['k8'] || 0;
    const songID = kvs['k45'] || 0; 
    const revision = kvs['k46'] || 0;
    return {
        name,
        description,
        levelString,
        objectCount,
        audioTrack,
        songID,
        revision,
    };
}
function decodeLevelString(encoded) {
    if (!encoded || encoded.length === 0) return { objects: '', settings: '' };
    try {
        if (encoded.startsWith('kS') || encoded.startsWith('1,')) {
            return splitLevelString(encoded);
        }
        let b64 = encoded.replace(/-/g, '+').replace(/_/g, '/');
        while (b64.length % 4 !== 0) b64 += '=';
        const compressed = Buffer.from(b64, 'base64');
        let decompressed;
        try {
            decompressed = zlib.gunzipSync(compressed).toString('utf-8');
        } catch {
            decompressed = zlib.inflateSync(compressed).toString('utf-8');
        }
        
        let result = splitLevelString(decompressed);
        
        if (result.objects.startsWith('UEsDBB')) {
            const buf = Buffer.from(result.objects, 'base64');
            if (buf.length >= 4 && buf.readUInt32LE(0) === 0x04034b50) {
                const compMethod = buf.readUInt16LE(8);
                const dataOffset = 30 + buf.readUInt16LE(26) + buf.readUInt16LE(28);
                const compressedData = buf.slice(dataOffset);
                if (compMethod === 8) {
                    try {
                        result.objects = zlib.inflateRawSync(compressedData).toString('utf-8');
                    } catch(e) {}
                } else if (compMethod === 0) {
                    result.objects = compressedData.toString('utf-8');
                }
            }
        }
        
        return result;
    } catch (e) {
        return { objects: '', settings: '' };
    }
}
function splitLevelString(raw) {
    const firstSemicolon = raw.indexOf(';');
    if (firstSemicolon === -1) {
        return { objects: raw, settings: '' };
    }
    const first = raw.substring(0, firstSemicolon);
    if (first.startsWith('kS')) {
        return {
            settings: first + ';',
            objects: raw.substring(firstSemicolon + 1),
        };
    }
    return { objects: raw, settings: '' };
}
function encodeLevelString(settings, objects) {
    let raw = '';
    if (settings) {
        raw = settings.endsWith(';') ? settings : settings + ';';
    }
    raw += objects;
    const compressed = zlib.gzipSync(Buffer.from(raw, 'utf-8'));
    let b64 = compressed.toString('base64');
    b64 = b64.replace(/\+/g, '-').replace(/\//g, '_').replace(/=+$/, '');
    return b64;
}
function exportToGmd(room, outDir, suffix = '_export') {
    let objectsData = room.compressedLevelData;
    if (objectsData.length >= 4 && objectsData.readUInt32LE(0) === 0x04034b50) {
        const compMethod = objectsData.readUInt16LE(8);
        const dataOffset = 30 + objectsData.readUInt16LE(26) + objectsData.readUInt16LE(28);
        const compressedData = objectsData.slice(dataOffset);
        if (compMethod === 8) {
            try { objectsData = zlib.inflateRawSync(compressedData); } catch(e) {}
        } else if (compMethod === 0) {
            objectsData = compressedData;
        }
    }
    const encoded = encodeLevelString(room.settings.saveString, objectsData.toString('utf8'));
    const gmd = `<plist version="1.0" gjver="2.0"><dict><k>k_0</k><s/><k>k_1</k><s></s><k>k2</k><s>${room.levelName}</s><k>k4</k><s>${encoded}</s></dict></plist>`;
    const safeName = room.levelName.replace(/[^a-zA-Z0-9_-]/g, '_');
    const filename = room.code ? `${room.code}_${safeName}${suffix}.gmd` : `${safeName}${suffix}.gmd`;
    const outFile = path.join(outDir, filename);
    fs.writeFileSync(outFile, gmd);
    return outFile;
}
module.exports = {
    getDefaultSavePaths,
    findSaveFile,
    decryptSaveFile,
    encryptSaveFile,
    parseLevels,
    parseGmd,
    decodeLevelString,
    encodeLevelString,
    exportToGmd
};
