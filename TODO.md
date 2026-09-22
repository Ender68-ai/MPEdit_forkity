MPEdit Roadmap

«Multiplayer collaborative Geometry Dash editor
Current version: v0.7.0 — In Development»

---

v0.7.0 — Room System & Host Integration

Focus: Finish the core room/hosting flow and integrate it cleanly with the editor.

Networking

- [x] Multiplayer networking foundation
- [x] Signaling server
- [x] Room creation
- [x] Room joining
- [x] Player synchronization
- [x] Cursor synchronization
- [x] Text chat
- [x] View-only players
- [x] Player information/icons
- [x] Dedicated-server support
- [x] "SessionManager"
- [x] "P2PManager"
- [x] Binary protocol
- [x] "RemoteActionHandler"

Host Mode

- [x] Host mode
- [x] Local level list
- [x] "GJListLayer" integration
- [x] "RoomCreateLayer" foundation
- [ ] Finish room creation UI
- [ ] Host → editor transition
- [ ] Loading/connecting screen
- [ ] Connection error handling
- [ ] Clean session teardown
- [ ] Investigate networking/NAT edge cases

---

v0.8.0 — Complete UI

Focus: Finish the user interface and make MPEdit feel like a complete mod.

Multiplayer Menu

- [ ] Polished multiplayer menu
- [ ] Host button
- [ ] Join button
- [ ] Room browser
- [ ] Room code display
- [ ] Player count
- [ ] Private/public indicators
- [ ] Password handling
- [ ] Room refresh
- [ ] Loading states
- [ ] Connection states
- [ ] Error notifications

Room Creation

- [ ] Finish "RoomCreateLayer"
- [ ] Room name
- [ ] Password
- [ ] Player limit
- [ ] Private room toggle
- [ ] Default view-only toggle
- [ ] Create button
- [ ] Cancel/back button
- [ ] Proper layer transitions

In-Editor UI

- [ ] Player list
- [ ] Player colors
- [ ] Player icons
- [ ] View-only indicator
- [ ] Host indicator
- [ ] Connection indicator
- [ ] Chat UI
- [ ] Chat notifications
- [ ] Room code display
- [ ] Leave-room button/menu

General UI Polish

- [ ] Animations
- [ ] Transitions
- [ ] Consistent node IDs
- [ ] Proper layer visibility management
- [ ] Resolution scaling
- [ ] Keyboard/controller handling where applicable
- [ ] Remove popup-only assumptions from layer-based UI

---

v0.9.0 — Multiplayer Editor Polish

Focus: Make collaborative editing reliable and pleasant.

«MPEdit already functions as a multiplayer editor. v0.9.0 is about improving the existing system rather than building multiplayer from scratch.»

Editor Synchronization

- [ ] Verify all important editor actions synchronize correctly
- [ ] Object creation
- [ ] Object deletion
- [ ] Object movement
- [ ] Object rotation
- [ ] Object scaling
- [ ] Object color changes
- [ ] Object properties
- [ ] Triggers
- [ ] Groups
- [ ] Layers
- [ ] Level settings
- [ ] Undo/redo synchronization
- [ ] Multi-object selections
- [ ] Rapid-edit testing

Conflict Handling

- [ ] Determine behavior when multiple players edit the same object
- [ ] Prevent desynchronization from simultaneous edits
- [ ] Host authority where necessary
- [ ] Action ordering
- [ ] Duplicate action protection
- [ ] Malformed packet protection
- [ ] Reconnection synchronization

Chat

- [x] Text chat
- [ ] Polished chat window
- [ ] Player names
- [ ] Chat notifications
- [ ] Scrolling
- [ ] Message length limits
- [ ] Proper cleanup when leaving

Reliability

- [ ] Reconnect handling
- [ ] Timeout handling
- [ ] Signaling server unavailable handling
- [ ] Room disappeared handling
- [ ] Player kick handling
- [ ] Host disconnect handling
- [ ] Clean editor exit
- [ ] Prevent stuck sessions
- [ ] Stress testing

---

v1.0.0 — First Stable Release

Focus: Make MPEdit stable enough to be considered a complete release.

«If someone installs MPEdit v1.0, the entire core experience should just work.»

Stability

- [ ] Long-session testing
- [ ] Multi-player testing
- [ ] Large-level testing
- [ ] High-action-rate testing
- [ ] Packet-loss testing
- [ ] High-latency testing
- [ ] Reconnection testing
- [ ] Server failure testing
- [ ] Crash testing
- [ ] Desynchronization detection

User Experience

- [ ] Intuitive room flow
- [ ] Clear connection status
- [ ] Clear error messages
- [ ] Loading screens
- [ ] No unexplained freezes
- [ ] No orphaned UI
- [ ] No duplicate callbacks/listeners
- [ ] Clean session lifecycle

Editor

- [ ] All important editor operations synchronized
- [ ] Multiplayer player representation
- [ ] Fully functional view-only mode
- [ ] Host permissions
- [ ] Room settings
- [ ] Text chat
- [ ] Level selection
- [ ] Host → level → editor flow
- [ ] Editor → leave → multiplayer menu flow

Infrastructure

- [ ] Production signaling server
- [ ] Server monitoring/logging
- [ ] Appropriate timeout values
- [ ] Protocol version handling
- [ ] Compatibility checks
- [ ] Server error reporting
- [ ] Release builds
- [ ] Documentation
- [ ] Installation instructions

Release

- [ ] README
- [ ] Screenshots
- [ ] Feature list
- [ ] Known limitations
- [ ] Bug-report system
- [ ] Stable v1.0.0 release

---

v1.1.0 — Collaboration Expansion

Focus: Expand the collaboration features beyond the core v1.0 experience.

Player Management

- [ ] Host controls
- [ ] Kick player
- [ ] Transfer host
- [ ] Per-player permissions
- [ ] Per-player view-only
- [ ] Player status/activity indicators

Editor Improvements

- [ ] Improved conflict handling
- [ ] Improved selection visualization
- [ ] Remote player cursors
- [ ] Action history
- [ ] Synchronization optimizations
- [ ] Bandwidth optimizations

Rooms

- [ ] Improved room browser
- [ ] Room search
- [ ] Filters
- [ ] Room sorting
- [ ] Room favorites/bookmarks if useful

---

v1.5.0 — Git for the Editor

Focus: Turn MPEdit from a collaborative editor into a collaborative development environment.

«The goal: multiple people should be able to develop a level together without destroying each other's work.»

Version Control

- [ ] Level snapshots
- [ ] Commits
- [ ] Commit messages
- [ ] Commit history
- [ ] Branches
- [ ] Checkout
- [ ] Reset/revert
- [ ] Version comparison
- [ ] Restore previous versions

Collaboration

- [ ] Commit authors
- [ ] Commit timestamps
- [ ] Remote commits
- [ ] Push
- [ ] Pull
- [ ] Branch switching
- [ ] Merge
- [ ] Remote synchronization

Level Diffing

- [ ] Create a diffable representation of a level
- [ ] Object-level diffs
- [ ] Added objects
- [ ] Deleted objects
- [ ] Modified objects
- [ ] Level-setting changes
- [ ] Conflict detection
- [ ] Conflict resolution UI

Version Control UI

- [ ] Commit history panel
- [ ] Branch selector
- [ ] Commit interface
- [ ] Diff viewer
- [ ] Merge interface
- [ ] Conflict resolution interface
- [ ] Restore/revert interface

Long-Term Goal

MPEdit v1.0

«"We can edit together."»

MPEdit v1.5

«"We can develop a level together without destroying each other's work."»

---

Version Summary

Version| Focus
v0.7.0| Room system & host integration
v0.8.0| Complete UI
v0.9.0| Multiplayer editor polish
v1.0.0| First stable release
v1.1.0| Collaboration expansion
v1.5.0| Git/version control for levels