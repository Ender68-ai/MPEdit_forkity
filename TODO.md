# MPEdit v0.8 UI + Globed Integration Master TODO

**Target:** v0.8 UI milestone, with v0.9.0 planned for approximately November 1  
**Additional requirement:** migrate and validate the mod against Geometry Dash 2.2091  
**Purpose:** a detailed backlog. This is not a requirement to complete every checkbox in one sitting.

## Working rules

- [ ] Keep tasks small enough to finish and test in one focused session.
- [ ] Define the expected result before editing code.
- [ ] Build after each coherent change.
- [ ] Test changes in Geometry Dash, not only by compiling.
- [ ] Commit each coherent, tested change.
- [ ] Keep UI presentation separate from session/networking logic.
- [ ] Preserve the core MPEdit experience when Globed is unavailable, if Globed support is intended to be optional.
- [ ] Prefer fixing crashes and broken behavior over cosmetic polish.
- [ ] Avoid broad rewrites unless a specific issue requires one.
- [ ] Track blocked tasks and their cause instead of silently abandoning them.
- [ ] Do not compare progress by commit count or implementation speed.
- [ ] Keep a short “next task” note at the end of each coding session.

## Phase 0 — Baseline and migration planning

### Repository baseline
- [ ] Confirm the active repository path.
- [ ] Confirm the current branch and working-tree status.
- [ ] Review uncommitted changes before starting.
- [ ] Create a backup branch or tag for the last known working build.
- [ ] Record the current MPEdit version and target Geode SDK.
- [ ] Record the current Geometry Dash version used for testing.
- [ ] Build the current project before changing the target version.
- [ ] Save the full build log if the build fails.
- [ ] Launch the current build and verify the basic host/join flow.
- [ ] Record known crashes and existing UI defects separately from new migration regressions.

### Geometry Dash 2.2091 migration plan
- [ ] Confirm the intended Geode SDK/bindings support Geometry Dash 2.2091.
- [ ] Read the SDK's migration notes and relevant breaking-change notes.
- [ ] Confirm the local game installation is actually version 2.2091.
- [ ] Keep the old game/build environment available until the migration is verified, if practical.
- [ ] Identify hooks that target vanilla classes or methods that may have changed.
- [ ] Identify direct includes of game headers that may have moved or changed.
- [ ] Identify calls relying on old signatures, enums, layouts, or object lifetimes.
- [ ] Identify mods/APIs used by MPEdit that may need compatible versions.
- [ ] Check Globed's target version and compatibility with the chosen SDK/game version.
- [ ] Write down migration blockers before attempting unrelated UI polish.

## Phase 1 — CMake, SDK, and 2.2091 compatibility

### Build configuration
- [ ] Point `GEODE_SDK` or the Geode SDK path at the intended compatible SDK.
- [ ] Verify the CMake minimum version and project configuration still configure cleanly.
- [ ] Verify the source glob includes all `.cpp` files.
- [ ] Keep source, header, and resource lists separate.
- [ ] Verify the resources glob includes the current UI assets.
- [ ] Verify include directories are correct after the repository move.
- [ ] Verify fetched dependency versions are compatible with the toolchain.
- [ ] Avoid mixing CMake `target_link_libraries` keyword and plain signatures for the same target.
- [ ] Confirm the correct target is passed to `setup_geode_mod`.
- [ ] Configure from a clean build directory if stale generated files cause confusion.
- [ ] Build with limited parallelism if memory is constrained.
- [ ] Fix build errors one at a time, starting with the first relevant error.
- [ ] Keep the complete error output for any unresolved SDK/API issue.

### Geode and game API migration
- [ ] Update Geode SDK/bindings to a version that supports Geometry Dash 2.2091.
- [ ] Follow the official SDK setup/migration instructions rather than guessing at version numbers.
- [ ] Rebuild and resolve changed declarations or method signatures.
- [ ] Check hook declarations against the 2.2091 bindings.
- [ ] Verify modified classes still exist and are hooked correctly.
- [ ] Verify hook callback signatures match the new bindings.
- [ ] Verify scene/layer transition APIs and types.
- [ ] Verify menu and touch handling behavior.
- [ ] Verify IDs used to find game nodes still exist in 2.2091.
- [ ] Verify level-browser and creator-layer assumptions.
- [ ] Verify any direct use of internal game fields or offsets.
- [ ] Verify no hook depends on a hard-coded memory address.
- [ ] Review any compiler warning about an invalid override or signature mismatch.
- [ ] Resolve compile errors without removing required functionality merely to make the build pass.
- [ ] Build the complete mod after API migration changes.
- [ ] Launch Geometry Dash 2.2091 with the mod installed.
- [ ] Confirm the mod loads without failing to load.
- [ ] Test the main menu and editor entry points.
- [ ] Test the host/join flow after migration.
- [ ] Test Globed integration after migration.
- [ ] Record any compatibility limitations that remain.

## Phase 2 — Shared UI foundation

### Audit
- [ ] Inventory existing MPEdit UI classes and screens.
- [ ] Map each screen's entry point and exit path.
- [ ] Identify reusable components: panels, buttons, labels, inputs, lists, and popups.
- [ ] Identify duplicated layout code.
- [ ] Identify callbacks/listeners registered by each screen.
- [ ] Check cleanup behavior when screens close or are destroyed.
- [ ] Identify which UI classes depend directly on SessionManager.
- [ ] Identify which UI classes depend directly on P2PManager.
- [ ] Keep networking decisions out of purely presentational components where possible.
- [ ] Build and run the project before refactoring shared components.

### Panel component
- [ ] Make `Panel` support a background-only mode.
- [ ] Treat an empty title as “do not create a title label.”
- [ ] Handle a null title safely if the API permits null input.
- [ ] Preserve existing titled-panel behavior.
- [ ] Check `CCScale9Sprite::create` for failure before using the result.
- [ ] Set the background anchor point explicitly.
- [ ] Set the background position explicitly.
- [ ] Ensure the panel's content size is applied to the scale-nine sprite.
- [ ] Define whether the supplied size represents the outer panel size.
- [ ] Position a title relative to the panel dimensions.
- [ ] Remove title offsets that assume a tall panel.
- [ ] Avoid creating unnecessary empty labels.
- [ ] Define a consistent panel color/style.
- [ ] Check label scale, alignment, and clipping.
- [ ] Check z-order between background, title, and content.
- [ ] Test short and tall panels.
- [ ] Test narrow and wide panels.
- [ ] Test every existing `Panel::create` call site.
- [ ] Build and test before changing unrelated UI.

### Layout conventions
- [ ] Define standard padding.
- [ ] Define standard button heights.
- [ ] Define standard label scales.
- [ ] Define spacing between controls.
- [ ] Decide which elements are anchored to screen edges.
- [ ] Use parent-relative positions for panel content.
- [ ] Use screen-relative positioning only for screen-level elements.
- [ ] Check anchor points on new nodes.
- [ ] Check coordinate spaces before placing child nodes.
- [ ] Check whether parent scaling affects child positions.
- [ ] Keep controls away from screen edges.
- [ ] Avoid overlapping interactive controls.
- [ ] Test different supported window sizes.
- [ ] Add layout helpers only when they reduce real duplication.

## Phase 3 — SessionStatusNode

### Initialization and ownership
- [ ] Ensure `SessionStatusNode::create()` handles initialization failure correctly.
- [ ] Verify `CCNode::init()` succeeds before creating children.
- [ ] Check every required sprite/label creation for null.
- [ ] Keep the status label as a single dynamic label.
- [ ] Create a background-only `Panel` for the status HUD.
- [ ] Store the panel/background in a member if it must be resized or recolored.
- [ ] Set the status node's anchor point explicitly.
- [ ] Decide whether its position is screen-relative or parent-relative.
- [ ] Position the label relative to the panel.
- [ ] Ensure the background is behind the label.
- [ ] Avoid creating two labels for the same status text.
- [ ] Avoid passing the current label text as a permanent panel title.

### Status and content
- [ ] Define text for every P2P state.
- [ ] Handle `Connected`.
- [ ] Handle `Connecting`.
- [ ] Handle `Reconnecting`.
- [ ] Handle `Disconnected`.
- [ ] Handle `Error`.
- [ ] Handle the no-active-session case.
- [ ] Decide whether the HUD hides or shows an idle state outside sessions.
- [ ] Display room name/code if useful and available.
- [ ] Display player count if useful and available.
- [ ] Keep error text short enough for the HUD.
- [ ] Do not display room passwords or other secrets.
- [ ] Use readable text colors with sufficient contrast.
- [ ] Ensure the color variable is assigned for every possible state.
- [ ] Provide a safe fallback for any unexpected state if the enum/API permits it.

### Updates and testing
- [ ] Cache session state only when the cache matches the displayed state.
- [ ] Avoid rebuilding the label when nothing has changed.
- [ ] Update the label when the connection state changes.
- [ ] Update the label when the room changes.
- [ ] Update the label when player count changes.
- [ ] Update the panel width when the displayed text changes.
- [ ] Add horizontal padding around the text.
- [ ] Clamp panel width to the available screen width.
- [ ] Confirm the update callback uses the correct signature for the SDK.
- [ ] Confirm scheduled updates stop when the node is destroyed.
- [ ] Test entering a session.
- [ ] Test leaving a session.
- [ ] Test connection failure.
- [ ] Test reconnecting.
- [ ] Test an error with a long message.
- [ ] Test repeated room changes.
- [ ] Test that the HUD does not persist into unrelated screens.
- [ ] Test the HUD on Geometry Dash 2.2091.

## Phase 4 — Multiplayer HUD

- [ ] Decide where the HUD should appear in the editor.
- [ ] Avoid covering important editor controls.
- [ ] Add a compact MPEdit label/icon.
- [ ] Add connection status.
- [ ] Add room name/code.
- [ ] Add player count.
- [ ] Add a collapsed state.
- [ ] Add an expanded state.
- [ ] Add a button to open session details.
- [ ] Add a button to open chat.
- [ ] Add a button to open room settings.
- [ ] Add a leave-session action.
- [ ] Confirm before leaving if the action could cause user work to be lost.
- [ ] Add labels/tooltips to unclear actions.
- [ ] Implement disabled states for unavailable actions.
- [ ] Synchronize HUD state with SessionManager.
- [ ] Synchronize connection information with P2PManager.
- [ ] Show the HUD only when appropriate.
- [ ] Hide it when no session is active.
- [ ] Restore it correctly after joining a new session.
- [ ] Check z-order with editor overlays.
- [ ] Test long room names.
- [ ] Test maximum player count.
- [ ] Test no remote players.
- [ ] Test host departure.
- [ ] Test remote-player departure.
- [ ] Test all HUD actions with mouse/touch input.

## Phase 5 — Player list

- [ ] Choose a panel, popup, or collapsible sidebar.
- [ ] Display the local player.
- [ ] Display remote players.
- [ ] Mark the host.
- [ ] Mark view-only players.
- [ ] Display editing state if that state is genuinely available.
- [ ] Display connection state if available.
- [ ] Use player colors only when they are sourced from real session data.
- [ ] Handle long usernames.
- [ ] Handle duplicate usernames.
- [ ] Handle an empty list.
- [ ] Handle a full room.
- [ ] Update when a player joins.
- [ ] Update when a player leaves.
- [ ] Update when permissions change.
- [ ] Avoid rebuilding every row on every update.
- [ ] Add scrolling if needed.
- [ ] Only display host controls when the local user has permission.
- [ ] Ensure non-hosts cannot invoke host-only actions through the UI.
- [ ] Test rapid join/leave events.
- [ ] Test leaving and immediately joining another room.
- [ ] Test with multiple clients on 2.2091.

## Phase 6 — Host and Join workflow

- [ ] Audit the Host screen.
- [ ] Audit the Join screen.
- [ ] Audit the local-level selector.
- [ ] Standardize panel sizes and button styles.
- [ ] Improve input labels and placeholders.
- [ ] Make privacy settings clear.
- [ ] Make default view-only behavior clear.
- [ ] Validate room names.
- [ ] Validate passcodes/passwords.
- [ ] Validate player limits.
- [ ] Prevent repeated submissions while requests are processing.
- [ ] Add visible loading states.
- [ ] Show hosting failures.
- [ ] Show joining failures.
- [ ] Show incorrect-password errors.
- [ ] Show room-full errors.
- [ ] Show room-not-found errors.
- [ ] Show timeout errors.
- [ ] Show server-unavailable errors.
- [ ] Provide a clear back/cancel action.
- [ ] Avoid leaving half-created sessions after failures.
- [ ] Remove callbacks/listeners when screens close.
- [ ] Prevent duplicate listener registration when reopening screens.
- [ ] Test Host → session.
- [ ] Test Join → session.
- [ ] Test cancel → previous screen.
- [ ] Test failure → retry.
- [ ] Test repeated screen opening.
- [ ] Test fresh configuration.
- [ ] Re-test the workflow on Geometry Dash 2.2091.

## Phase 7 — Chat UI

- [ ] Create a compact chat panel.
- [ ] Display incoming messages.
- [ ] Display sender names.
- [ ] Distinguish local and remote messages.
- [ ] Add a message input.
- [ ] Add a send button.
- [ ] Implement Enter-to-send only if consistent with the input component.
- [ ] Reject or ignore empty messages.
- [ ] Handle long messages.
- [ ] Limit displayed history to a reasonable amount.
- [ ] Add scrolling.
- [ ] Keep the latest message visible where appropriate.
- [ ] Add a collapsed state.
- [ ] Add an unread-message indicator.
- [ ] Avoid covering critical editor controls.
- [ ] Show errors when messages cannot be sent.
- [ ] Disable sending when disconnected if appropriate.
- [ ] Test rapid messages.
- [ ] Test long usernames.
- [ ] Test chat after reconnecting.
- [ ] Ensure closing the panel does not stop message handling.
- [ ] Test the chat on 2.2091 with multiple clients.

## Phase 8 — Notifications and error UX

- [ ] Add a player-joined notification.
- [ ] Add a player-left notification.
- [ ] Add a host-changed notification if supported.
- [ ] Add a connection-lost notification.
- [ ] Add a reconnecting notification.
- [ ] Add a session-ended notification.
- [ ] Add a permissions-changed notification.
- [ ] Add a room-creation failure notification.
- [ ] Add a room-join failure notification.
- [ ] Keep notifications concise.
- [ ] Prevent duplicate notifications for one event.
- [ ] Allow notifications to expire or be dismissed.
- [ ] Never show passwords or private room secrets.
- [ ] Ensure notifications do not block unrelated clicks.
- [ ] Test multiple events in quick succession.
- [ ] Verify notifications behave correctly after scene transitions.

## Phase 9 — Cursor and player presence

- [ ] Audit the existing remote-cursor implementation.
- [ ] Confirm cursor coordinates use the correct coordinate space.
- [ ] Add a player-name label to remote cursors.
- [ ] Associate each cursor with the correct player.
- [ ] Use consistent player colors if available.
- [ ] Keep labels readable against level objects.
- [ ] Avoid oversized cursor graphics.
- [ ] Handle cursors near screen edges.
- [ ] Hide cursors when players leave.
- [ ] Clear stale cursor data after disconnects.
- [ ] Avoid creating new nodes on every cursor update.
- [ ] Ensure overlays do not intercept editor input.
- [ ] Test with two clients.
- [ ] Test with several clients.
- [ ] Test different zoom levels.
- [ ] Test editor scrolling.
- [ ] Test reconnecting during editing.
- [ ] Verify behavior on 2.2091.

## Phase 10 — Globed integration discovery

### Compatibility and architecture
- [ ] Identify the exact Globed version intended for testing.
- [ ] Confirm whether the chosen Globed version supports the target Geode SDK/game version.
- [ ] Inspect Globed's existing menu/layer architecture.
- [ ] Identify the intended MPEdit entry point.
- [ ] Confirm which Globed classes can safely be modified.
- [ ] Keep Globed-specific code separate from core MPEdit UI.
- [ ] Decide whether Globed is optional or a hard dependency.
- [ ] Reflect that decision accurately in `mod.json`.
- [ ] Verify installed-versus-enabled behavior for the dependency.
- [ ] Verify the mod behaves correctly when Globed is disabled.
- [ ] Verify the intended fallback when Globed is not installed.
- [ ] Avoid assuming menu child IDs always exist.
- [ ] Avoid assuming a menu's layout never changes.
- [ ] Avoid hard-coded memory addresses or unsupported offsets.
- [ ] Check relevant Globed source/API documentation before relying on implementation details.
- [ ] Record any limitations or incompatibilities.

## Phase 11 — Globed integration entry point

### Button and menu
- [ ] Add an MPEdit entry point to the chosen Globed menu.
- [ ] Use the project's standard resource-loading convention.
- [ ] Verify the button sprite exists in the packaged mod.
- [ ] Check sprite creation for failure before using the pointer.
- [ ] Set an appropriate ID on the new button.
- [ ] Prevent duplicate buttons when a menu refreshes.
- [ ] Add the button only after the target menu exists.
- [ ] Avoid changing the layout of existing Globed controls unexpectedly.
- [ ] Check whether `updateLayout()` repositions existing children.
- [ ] Confirm the button does not overlap existing controls.
- [ ] Confirm the button's hitbox matches its visible sprite.
- [ ] Choose a suitable scale and position.
- [ ] Check z-order and touch priority.
- [ ] Add a tooltip or clear label if the icon is not self-explanatory.
- [ ] Handle a missing target menu gracefully.
- [ ] Log integration errors during development.

### Callback and navigation
- [ ] Confirm the button callback signature matches `CCMenuItemSpriteExtra::create`.
- [ ] Add a temporary log at the start of the callback.
- [ ] Verify one click invokes the callback once.
- [ ] Open the correct MPEdit screen.
- [ ] Use a transition API/type supported by the target SDK.
- [ ] Avoid creating redundant scenes if existing navigation can be reused.
- [ ] Verify Back returns to the expected screen.
- [ ] Verify the original Globed menu remains functional.
- [ ] Prevent duplicate MPEdit layers after repeated clicks.
- [ ] Handle MPEdit screen initialization failure safely.
- [ ] Ensure callbacks do not outlive their target layer.
- [ ] Remove temporary logs after the behavior is verified.

### Compatibility testing
- [ ] Test Globed enabled.
- [ ] Test Globed disabled.
- [ ] Test Globed absent if optional integration is intended.
- [ ] Test with Eclipse Menu enabled.
- [ ] Test with DevTools enabled.
- [ ] Test with other relevant menu-modifying mods enabled.
- [ ] Confirm existing Globed buttons still work.
- [ ] Confirm existing buttons are not moved unexpectedly.
- [ ] Confirm the MPEdit button does not trigger another button underneath it.
- [ ] Test repeated menu opening and closing.
- [ ] Test repeated transitions between Globed and MPEdit.
- [ ] Test after returning from the editor.
- [ ] Test on Geometry Dash 2.2091.
- [ ] Inspect crash logs if a click reaches an unexpected vanilla callback.
- [ ] Disable one potentially conflicting mod at a time when isolating a conflict.
- [ ] Do not attribute a crash to Globed without a reproducible test.

## Phase 12 — Integration architecture and lifecycle

- [ ] Keep the Globed hook in its own source file.
- [ ] Keep networking logic inside SessionManager/P2PManager.
- [ ] Keep presentation inside UI classes.
- [ ] Avoid circular dependencies between Globed UI and MPEdit core.
- [ ] Check pointers returned by node lookup before use.
- [ ] Check sprite/label/layer creation before dereferencing.
- [ ] Avoid retaining raw pointers after the owning layer is destroyed.
- [ ] Remove listeners when their owner is destroyed.
- [ ] Unschedule callbacks when their owner is destroyed.
- [ ] Prevent duplicate hook or callback registration.
- [ ] Ensure initialization failure does not leave partial UI behind.
- [ ] Ensure leaving a session clears session-specific UI state.
- [ ] Ensure reopening a menu does not reuse stale session data.
- [ ] Confirm the mod can compile without accidentally including unavailable Globed headers when optional support is intended.
- [ ] Verify dependency and include configuration for the chosen integration model.
- [ ] Build with the final intended integration configuration.

## Phase 13 — Visual polish

- [ ] Standardize panel colors.
- [ ] Standardize button styles.
- [ ] Standardize typography.
- [ ] Standardize spacing.
- [ ] Standardize input dimensions.
- [ ] Align labels and controls consistently.
- [ ] Check text contrast.
- [ ] Check long text and clipping.
- [ ] Check menu transitions.
- [ ] Add animations only where they clarify state changes.
- [ ] Keep animations short.
- [ ] Avoid blocking input during unnecessary animation.
- [ ] Check all panels against the game's visual style.
- [ ] Remove redundant labels.
- [ ] Remove unnecessary decorative elements.
- [ ] Check that important status information remains visible.
- [ ] Check small and large resolutions.
- [ ] Check UI after a scene transition.
- [ ] Check UI after host/join failures.
- [ ] Check UI after reconnecting.
- [ ] Do not start a full redesign during the final stabilization week.

## Phase 14 — Regression testing

### Core behavior
- [ ] Host a room.
- [ ] Join a room.
- [ ] Leave a room.
- [ ] Repeatedly host and leave.
- [ ] Repeatedly join and leave.
- [ ] Try an invalid room.
- [ ] Try a wrong passcode.
- [ ] Try a full room.
- [ ] Test a connection timeout.
- [ ] Test a disconnect.
- [ ] Test reconnecting.
- [ ] Test view-only permissions.
- [ ] Test multiple clients.
- [ ] Verify level edits still synchronize.
- [ ] Verify chat still works.
- [ ] Verify cursor sync still works.
- [ ] Verify UI state follows actual session state.

### UI and lifecycle
- [ ] Open and close every screen repeatedly.
- [ ] Navigate backward and forward through the UI.
- [ ] Check for duplicate panels.
- [ ] Check for duplicate callbacks.
- [ ] Check for stale player lists.
- [ ] Check for stale room codes.
- [ ] Check for stale error messages.
- [ ] Check for labels outside panel bounds.
- [ ] Check for overlapping buttons.
- [ ] Check that no invisible control intercepts clicks.
- [ ] Check that the HUD disappears when appropriate.
- [ ] Check that scheduled updates stop when nodes are destroyed.
- [ ] Check for crashes after repeated scene transitions.
- [ ] Check for crashes when leaving during a connection attempt.

### Mod compatibility
- [ ] Test with required dependencies enabled.
- [ ] Test with Globed enabled.
- [ ] Test with Globed disabled.
- [ ] Test without Globed if the dependency model allows it.
- [ ] Test with Eclipse Menu.
- [ ] Test with DevTools.
- [ ] Test with other relevant UI mods.
- [ ] Isolate conflicts by changing one variable at a time.
- [ ] Save logs for reproducible crashes.
- [ ] Verify the final test environment is Geometry Dash 2.2091.

## Phase 15 — Release packaging

- [ ] Freeze new features.
- [ ] Review the diff for accidental changes.
- [ ] Remove temporary experiments.
- [ ] Remove dead code.
- [ ] Remove debug-only UI.
- [ ] Check resource filenames and paths.
- [ ] Check resource packaging.
- [ ] Check `mod.json` metadata.
- [ ] Check dependency declarations.
- [ ] Check the target version/build configuration.
- [ ] Update the version only according to the project's release plan.
- [ ] Update the changelog.
- [ ] Build the Release configuration.
- [ ] Install the actual generated `.geode` package.
- [ ] Launch Geometry Dash 2.2091 from a clean start.
- [ ] Verify the mod loads.
- [ ] Verify the Host/Join flow.
- [ ] Verify SessionStatusNode and HUD.
- [ ] Verify player list and chat.
- [ ] Verify Globed entry point.
- [ ] Verify expected behavior when Globed is disabled.
- [ ] Test one complete two-client session.
- [ ] Review logs for startup errors.
- [ ] Record known issues.
- [ ] Tag or prepare the release commit.
- [ ] Publish only after the packaged build passes the test checklist.

## Suggested schedule

### Sep 24–28 — Unblock and establish the baseline
- [ ] Confirm the 2.2091 migration requirements.
- [ ] Fix the shared Panel empty-title behavior.
- [ ] Fix SessionStatusNode overlap and duplicate text.
- [ ] Build and test.
- [ ] Commit the changes.

### Sep 29–Oct 5 — HUD and player list
- [ ] Finish SessionStatusNode.
- [ ] Add room information and player count.
- [ ] Implement the basic player list.
- [ ] Wire the UI to real session data.
- [ ] Test with two clients.
- [ ] Commit each finished component.

### Oct 6–12 — Host/Join and chat
- [ ] Improve Host and Join screens.
- [ ] Add loading and error states.
- [ ] Improve chat presentation.
- [ ] Test failure/retry paths.
- [ ] Re-test on 2.2091.

### Oct 13–19 — Globed integration
- [ ] Confirm compatibility and dependency strategy.
- [ ] Implement the Globed entry point.
- [ ] Verify callbacks and navigation.
- [ ] Test with Globed enabled and disabled.
- [ ] Fix menu overlap and lifecycle issues.

### Oct 20–25 — Presence and visual polish
- [ ] Improve remote cursor labels.
- [ ] Add useful join/leave notifications.
- [ ] Polish spacing, colors, and transitions.
- [ ] Avoid major architectural rewrites.
- [ ] Finish outstanding high-priority UI tasks.

### Oct 26–28 — Stabilization
- [ ] Freeze new features.
- [ ] Run the regression checklist.
- [ ] Fix crashes, broken callbacks, and stale state.
- [ ] Test the packaged build on 2.2091.

### Oct 29–Nov 1 — Release preparation
- [ ] Finalize version metadata and changelog.
- [ ] Build Release.
- [ ] Test the generated `.geode` package.
- [ ] Verify the Globed integration.
- [ ] Fix only release-blocking bugs.
- [ ] Prepare the v0.9.0 release.

## Definition of done

A task is not complete merely because it compiles. For each feature:

- [ ] The code builds.
- [ ] The feature works in Geometry Dash.
- [ ] The relevant success path was tested.
- [ ] The relevant failure path was tested where applicable.
- [ ] The UI does not overlap or intercept unrelated controls.
- [ ] Callbacks/listeners have a safe lifecycle.
- [ ] The feature still works on Geometry Dash 2.2091.
- [ ] The change is committed.
- [ ] The next task is written down.

## Priority order if time gets tight

1. [ ] Geometry Dash 2.2091 migration and successful startup.
2. [ ] Fix Panel and SessionStatusNode.
3. [ ] Reliable session HUD.
4. [ ] Player list using real data.
5. [ ] Reliable Host/Join flow.
6. [ ] Globed entry point that doesn't break other menus.
7. [ ] Chat and useful notifications.
8. [ ] Cursor labels and cosmetic polish.
9. [ ] Nonessential animations and decoration.

**Reminder:** this is a backlog, not a promise to finish every checkbox by November 1. A stable, tested UI and a working 2.2091 migration are more valuable than a larger set of half-finished features.
"""

output_path = "/mnt/data/mpedit_v08_ui_globed_2091_todo.md"
pypandoc.convert_text(content, "md", format="md", outputfile=output_path, extra_args=["--standalone"])
print(f"Created: {output_path}")
import pypandoc

content = r"""# MPEdit v0.8 UI + Globed Integration Master TODO

**Target:** v0.8 UI milestone, with v0.9.0 planned for approximately November 1  
**Additional requirement:** migrate and validate the mod against Geometry Dash 2.2091  
**Purpose:** a detailed backlog. This is not a requirement to complete every checkbox in one sitting.

## Working rules

- [ ] Keep tasks small enough to finish and test in one focused session.
- [ ] Define the expected result before editing code.
- [ ] Build after each coherent change.
- [ ] Test changes in Geometry Dash, not only by compiling.
- [ ] Commit each coherent, tested change.
- [ ] Keep UI presentation separate from session/networking logic.
- [ ] Preserve the core MPEdit experience when Globed is unavailable, if Globed support is intended to be optional.
- [ ] Prefer fixing crashes and broken behavior over cosmetic polish.
- [ ] Avoid broad rewrites unless a specific issue requires one.
- [ ] Track blocked tasks and their cause instead of silently abandoning them.
- [ ] Do not compare progress by commit count or implementation speed.
- [ ] Keep a short “next task” note at the end of each coding session.

## Phase 0 — Baseline and migration planning

### Repository baseline
- [ ] Confirm the active repository path.
- [ ] Confirm the current branch and working-tree status.
- [ ] Review uncommitted changes before starting.
- [ ] Create a backup branch or tag for the last known working build.
- [ ] Record the current MPEdit version and target Geode SDK.
- [ ] Record the current Geometry Dash version used for testing.
- [ ] Build the current project before changing the target version.
- [ ] Save the full build log if the build fails.
- [ ] Launch the current build and verify the basic host/join flow.
- [ ] Record known crashes and existing UI defects separately from new migration regressions.

### Geometry Dash 2.2091 migration plan
- [ ] Confirm the intended Geode SDK/bindings support Geometry Dash 2.2091.
- [ ] Read the SDK's migration notes and relevant breaking-change notes.
- [ ] Confirm the local game installation is actually version 2.2091.
- [ ] Keep the old game/build environment available until the migration is verified, if practical.
- [ ] Identify hooks that target vanilla classes or methods that may have changed.
- [ ] Identify direct includes of game headers that may have moved or changed.
- [ ] Identify calls relying on old signatures, enums, layouts, or object lifetimes.
- [ ] Identify mods/APIs used by MPEdit that may need compatible versions.
- [ ] Check Globed's target version and compatibility with the chosen SDK/game version.
- [ ] Write down migration blockers before attempting unrelated UI polish.

## Phase 1 — CMake, SDK, and 2.2091 compatibility

### Build configuration
- [ ] Point `GEODE_SDK` or the Geode SDK path at the intended compatible SDK.
- [ ] Verify the CMake minimum version and project configuration still configure cleanly.
- [ ] Verify the source glob includes all `.cpp` files.
- [ ] Keep source, header, and resource lists separate.
- [ ] Verify the resources glob includes the current UI assets.
- [ ] Verify include directories are correct after the repository move.
- [ ] Verify fetched dependency versions are compatible with the toolchain.
- [ ] Avoid mixing CMake `target_link_libraries` keyword and plain signatures for the same target.
- [ ] Confirm the correct target is passed to `setup_geode_mod`.
- [ ] Configure from a clean build directory if stale generated files cause confusion.
- [ ] Build with limited parallelism if memory is constrained.
- [ ] Fix build errors one at a time, starting with the first relevant error.
- [ ] Keep the complete error output for any unresolved SDK/API issue.

### Geode and game API migration
- [ ] Update Geode SDK/bindings to a version that supports Geometry Dash 2.2091.
- [ ] Follow the official SDK setup/migration instructions rather than guessing at version numbers.
- [ ] Rebuild and resolve changed declarations or method signatures.
- [ ] Check hook declarations against the 2.2091 bindings.
- [ ] Verify modified classes still exist and are hooked correctly.
- [ ] Verify hook callback signatures match the new bindings.
- [ ] Verify scene/layer transition APIs and types.
- [ ] Verify menu and touch handling behavior.
- [ ] Verify IDs used to find game nodes still exist in 2.2091.
- [ ] Verify level-browser and creator-layer assumptions.
- [ ] Verify any direct use of internal game fields or offsets.
- [ ] Verify no hook depends on a hard-coded memory address.
- [ ] Review any compiler warning about an invalid override or signature mismatch.
- [ ] Resolve compile errors without removing required functionality merely to make the build pass.
- [ ] Build the complete mod after API migration changes.
- [ ] Launch Geometry Dash 2.2091 with the mod installed.
- [ ] Confirm the mod loads without failing to load.
- [ ] Test the main menu and editor entry points.
- [ ] Test the host/join flow after migration.
- [ ] Test Globed integration after migration.
- [ ] Record any compatibility limitations that remain.

## Phase 2 — Shared UI foundation

### Audit
- [ ] Inventory existing MPEdit UI classes and screens.
- [ ] Map each screen's entry point and exit path.
- [ ] Identify reusable components: panels, buttons, labels, inputs, lists, and popups.
- [ ] Identify duplicated layout code.
- [ ] Identify callbacks/listeners registered by each screen.
- [ ] Check cleanup behavior when screens close or are destroyed.
- [ ] Identify which UI classes depend directly on SessionManager.
- [ ] Identify which UI classes depend directly on P2PManager.
- [ ] Keep networking decisions out of purely presentational components where possible.
- [ ] Build and run the project before refactoring shared components.

### Panel component
- [ ] Make `Panel` support a background-only mode.
- [ ] Treat an empty title as “do not create a title label.”
- [ ] Handle a null title safely if the API permits null input.
- [ ] Preserve existing titled-panel behavior.
- [ ] Check `CCScale9Sprite::create` for failure before using the result.
- [ ] Set the background anchor point explicitly.
- [ ] Set the background position explicitly.
- [ ] Ensure the panel's content size is applied to the scale-nine sprite.
- [ ] Define whether the supplied size represents the outer panel size.
- [ ] Position a title relative to the panel dimensions.
- [ ] Remove title offsets that assume a tall panel.
- [ ] Avoid creating unnecessary empty labels.
- [ ] Define a consistent panel color/style.
- [ ] Check label scale, alignment, and clipping.
- [ ] Check z-order between background, title, and content.
- [ ] Test short and tall panels.
- [ ] Test narrow and wide panels.
- [ ] Test every existing `Panel::create` call site.
- [ ] Build and test before changing unrelated UI.

### Layout conventions
- [ ] Define standard padding.
- [ ] Define standard button heights.
- [ ] Define standard label scales.
- [ ] Define spacing between controls.
- [ ] Decide which elements are anchored to screen edges.
- [ ] Use parent-relative positions for panel content.
- [ ] Use screen-relative positioning only for screen-level elements.
- [ ] Check anchor points on new nodes.
- [ ] Check coordinate spaces before placing child nodes.
- [ ] Check whether parent scaling affects child positions.
- [ ] Keep controls away from screen edges.
- [ ] Avoid overlapping interactive controls.
- [ ] Test different supported window sizes.
- [ ] Add layout helpers only when they reduce real duplication.

## Phase 3 — SessionStatusNode

### Initialization and ownership
- [ ] Ensure `SessionStatusNode::create()` handles initialization failure correctly.
- [ ] Verify `CCNode::init()` succeeds before creating children.
- [ ] Check every required sprite/label creation for null.
- [ ] Keep the status label as a single dynamic label.
- [ ] Create a background-only `Panel` for the status HUD.
- [ ] Store the panel/background in a member if it must be resized or recolored.
- [ ] Set the status node's anchor point explicitly.
- [ ] Decide whether its position is screen-relative or parent-relative.
- [ ] Position the label relative to the panel.
- [ ] Ensure the background is behind the label.
- [ ] Avoid creating two labels for the same status text.
- [ ] Avoid passing the current label text as a permanent panel title.

### Status and content
- [ ] Define text for every P2P state.
- [ ] Handle `Connected`.
- [ ] Handle `Connecting`.
- [ ] Handle `Reconnecting`.
- [ ] Handle `Disconnected`.
- [ ] Handle `Error`.
- [ ] Handle the no-active-session case.
- [ ] Decide whether the HUD hides or shows an idle state outside sessions.
- [ ] Display room name/code if useful and available.
- [ ] Display player count if useful and available.
- [ ] Keep error text short enough for the HUD.
- [ ] Do not display room passwords or other secrets.
- [ ] Use readable text colors with sufficient contrast.
- [ ] Ensure the color variable is assigned for every possible state.
- [ ] Provide a safe fallback for any unexpected state if the enum/API permits it.

### Updates and testing
- [ ] Cache session state only when the cache matches the displayed state.
- [ ] Avoid rebuilding the label when nothing has changed.
- [ ] Update the label when the connection state changes.
- [ ] Update the label when the room changes.
- [ ] Update the label when player count changes.
- [ ] Update the panel width when the displayed text changes.
- [ ] Add horizontal padding around the text.
- [ ] Clamp panel width to the available screen width.
- [ ] Confirm the update callback uses the correct signature for the SDK.
- [ ] Confirm scheduled updates stop when the node is destroyed.
- [ ] Test entering a session.
- [ ] Test leaving a session.
- [ ] Test connection failure.
- [ ] Test reconnecting.
- [ ] Test an error with a long message.
- [ ] Test repeated room changes.
- [ ] Test that the HUD does not persist into unrelated screens.
- [ ] Test the HUD on Geometry Dash 2.2091.

## Phase 4 — Multiplayer HUD

- [ ] Decide where the HUD should appear in the editor.
- [ ] Avoid covering important editor controls.
- [ ] Add a compact MPEdit label/icon.
- [ ] Add connection status.
- [ ] Add room name/code.
- [ ] Add player count.
- [ ] Add a collapsed state.
- [ ] Add an expanded state.
- [ ] Add a button to open session details.
- [ ] Add a button to open chat.
- [ ] Add a button to open room settings.
- [ ] Add a leave-session action.
- [ ] Confirm before leaving if the action could cause user work to be lost.
- [ ] Add labels/tooltips to unclear actions.
- [ ] Implement disabled states for unavailable actions.
- [ ] Synchronize HUD state with SessionManager.
- [ ] Synchronize connection information with P2PManager.
- [ ] Show the HUD only when appropriate.
- [ ] Hide it when no session is active.
- [ ] Restore it correctly after joining a new session.
- [ ] Check z-order with editor overlays.
- [ ] Test long room names.
- [ ] Test maximum player count.
- [ ] Test no remote players.
- [ ] Test host departure.
- [ ] Test remote-player departure.
- [ ] Test all HUD actions with mouse/touch input.

## Phase 5 — Player list

- [ ] Choose a panel, popup, or collapsible sidebar.
- [ ] Display the local player.
- [ ] Display remote players.
- [ ] Mark the host.
- [ ] Mark view-only players.
- [ ] Display editing state if that state is genuinely available.
- [ ] Display connection state if available.
- [ ] Use player colors only when they are sourced from real session data.
- [ ] Handle long usernames.
- [ ] Handle duplicate usernames.
- [ ] Handle an empty list.
- [ ] Handle a full room.
- [ ] Update when a player joins.
- [ ] Update when a player leaves.
- [ ] Update when permissions change.
- [ ] Avoid rebuilding every row on every update.
- [ ] Add scrolling if needed.
- [ ] Only display host controls when the local user has permission.
- [ ] Ensure non-hosts cannot invoke host-only actions through the UI.
- [ ] Test rapid join/leave events.
- [ ] Test leaving and immediately joining another room.
- [ ] Test with multiple clients on 2.2091.

## Phase 6 — Host and Join workflow

- [ ] Audit the Host screen.
- [ ] Audit the Join screen.
- [ ] Audit the local-level selector.
- [ ] Standardize panel sizes and button styles.
- [ ] Improve input labels and placeholders.
- [ ] Make privacy settings clear.
- [ ] Make default view-only behavior clear.
- [ ] Validate room names.
- [ ] Validate passcodes/passwords.
- [ ] Validate player limits.
- [ ] Prevent repeated submissions while requests are processing.
- [ ] Add visible loading states.
- [ ] Show hosting failures.
- [ ] Show joining failures.
- [ ] Show incorrect-password errors.
- [ ] Show room-full errors.
- [ ] Show room-not-found errors.
- [ ] Show timeout errors.
- [ ] Show server-unavailable errors.
- [ ] Provide a clear back/cancel action.
- [ ] Avoid leaving half-created sessions after failures.
- [ ] Remove callbacks/listeners when screens close.
- [ ] Prevent duplicate listener registration when reopening screens.
- [ ] Test Host → session.
- [ ] Test Join → session.
- [ ] Test cancel → previous screen.
- [ ] Test failure → retry.
- [ ] Test repeated screen opening.
- [ ] Test fresh configuration.
- [ ] Re-test the workflow on Geometry Dash 2.2091.

## Phase 7 — Chat UI

- [ ] Create a compact chat panel.
- [ ] Display incoming messages.
- [ ] Display sender names.
- [ ] Distinguish local and remote messages.
- [ ] Add a message input.
- [ ] Add a send button.
- [ ] Implement Enter-to-send only if consistent with the input component.
- [ ] Reject or ignore empty messages.
- [ ] Handle long messages.
- [ ] Limit displayed history to a reasonable amount.
- [ ] Add scrolling.
- [ ] Keep the latest message visible where appropriate.
- [ ] Add a collapsed state.
- [ ] Add an unread-message indicator.
- [ ] Avoid covering critical editor controls.
- [ ] Show errors when messages cannot be sent.
- [ ] Disable sending when disconnected if appropriate.
- [ ] Test rapid messages.
- [ ] Test long usernames.
- [ ] Test chat after reconnecting.
- [ ] Ensure closing the panel does not stop message handling.
- [ ] Test the chat on 2.2091 with multiple clients.

## Phase 8 — Notifications and error UX

- [ ] Add a player-joined notification.
- [ ] Add a player-left notification.
- [ ] Add a host-changed notification if supported.
- [ ] Add a connection-lost notification.
- [ ] Add a reconnecting notification.
- [ ] Add a session-ended notification.
- [ ] Add a permissions-changed notification.
- [ ] Add a room-creation failure notification.
- [ ] Add a room-join failure notification.
- [ ] Keep notifications concise.
- [ ] Prevent duplicate notifications for one event.
- [ ] Allow notifications to expire or be dismissed.
- [ ] Never show passwords or private room secrets.
- [ ] Ensure notifications do not block unrelated clicks.
- [ ] Test multiple events in quick succession.
- [ ] Verify notifications behave correctly after scene transitions.

## Phase 9 — Cursor and player presence

- [ ] Audit the existing remote-cursor implementation.
- [ ] Confirm cursor coordinates use the correct coordinate space.
- [ ] Add a player-name label to remote cursors.
- [ ] Associate each cursor with the correct player.
- [ ] Use consistent player colors if available.
- [ ] Keep labels readable against level objects.
- [ ] Avoid oversized cursor graphics.
- [ ] Handle cursors near screen edges.
- [ ] Hide cursors when players leave.
- [ ] Clear stale cursor data after disconnects.
- [ ] Avoid creating new nodes on every cursor update.
- [ ] Ensure overlays do not intercept editor input.
- [ ] Test with two clients.
- [ ] Test with several clients.
- [ ] Test different zoom levels.
- [ ] Test editor scrolling.
- [ ] Test reconnecting during editing.
- [ ] Verify behavior on 2.2091.

## Phase 10 — Globed integration discovery

### Compatibility and architecture
- [ ] Identify the exact Globed version intended for testing.
- [ ] Confirm whether the chosen Globed version supports the target Geode SDK/game version.
- [ ] Inspect Globed's existing menu/layer architecture.
- [ ] Identify the intended MPEdit entry point.
- [ ] Confirm which Globed classes can safely be modified.
- [ ] Keep Globed-specific code separate from core MPEdit UI.
- [ ] Decide whether Globed is optional or a hard dependency.
- [ ] Reflect that decision accurately in `mod.json`.
- [ ] Verify installed-versus-enabled behavior for the dependency.
- [ ] Verify the mod behaves correctly when Globed is disabled.
- [ ] Verify the intended fallback when Globed is not installed.
- [ ] Avoid assuming menu child IDs always exist.
- [ ] Avoid assuming a menu's layout never changes.
- [ ] Avoid hard-coded memory addresses or unsupported offsets.
- [ ] Check relevant Globed source/API documentation before relying on implementation details.
- [ ] Record any limitations or incompatibilities.

## Phase 11 — Globed integration entry point

### Button and menu
- [ ] Add an MPEdit entry point to the chosen Globed menu.
- [ ] Use the project's standard resource-loading convention.
- [ ] Verify the button sprite exists in the packaged mod.
- [ ] Check sprite creation for failure before using the pointer.
- [ ] Set an appropriate ID on the new button.
- [ ] Prevent duplicate buttons when a menu refreshes.
- [ ] Add the button only after the target menu exists.
- [ ] Avoid changing the layout of existing Globed controls unexpectedly.
- [ ] Check whether `updateLayout()` repositions existing children.
- [ ] Confirm the button does not overlap existing controls.
- [ ] Confirm the button's hitbox matches its visible sprite.
- [ ] Choose a suitable scale and position.
- [ ] Check z-order and touch priority.
- [ ] Add a tooltip or clear label if the icon is not self-explanatory.
- [ ] Handle a missing target menu gracefully.
- [ ] Log integration errors during development.

### Callback and navigation
- [ ] Confirm the button callback signature matches `CCMenuItemSpriteExtra::create`.
- [ ] Add a temporary log at the start of the callback.
- [ ] Verify one click invokes the callback once.
- [ ] Open the correct MPEdit screen.
- [ ] Use a transition API/type supported by the target SDK.
- [ ] Avoid creating redundant scenes if existing navigation can be reused.
- [ ] Verify Back returns to the expected screen.
- [ ] Verify the original Globed menu remains functional.
- [ ] Prevent duplicate MPEdit layers after repeated clicks.
- [ ] Handle MPEdit screen initialization failure safely.
- [ ] Ensure callbacks do not outlive their target layer.
- [ ] Remove temporary logs after the behavior is verified.

### Compatibility testing
- [ ] Test Globed enabled.
- [ ] Test Globed disabled.
- [ ] Test Globed absent if optional integration is intended.
- [ ] Test with Eclipse Menu enabled.
- [ ] Test with DevTools enabled.
- [ ] Test with other relevant menu-modifying mods enabled.
- [ ] Confirm existing Globed buttons still work.
- [ ] Confirm existing buttons are not moved unexpectedly.
- [ ] Confirm the MPEdit button does not trigger another button underneath it.
- [ ] Test repeated menu opening and closing.
- [ ] Test repeated transitions between Globed and MPEdit.
- [ ] Test after returning from the editor.
- [ ] Test on Geometry Dash 2.2091.
- [ ] Inspect crash logs if a click reaches an unexpected vanilla callback.
- [ ] Disable one potentially conflicting mod at a time when isolating a conflict.
- [ ] Do not attribute a crash to Globed without a reproducible test.

## Phase 12 — Integration architecture and lifecycle

- [ ] Keep the Globed hook in its own source file.
- [ ] Keep networking logic inside SessionManager/P2PManager.
- [ ] Keep presentation inside UI classes.
- [ ] Avoid circular dependencies between Globed UI and MPEdit core.
- [ ] Check pointers returned by node lookup before use.
- [ ] Check sprite/label/layer creation before dereferencing.
- [ ] Avoid retaining raw pointers after the owning layer is destroyed.
- [ ] Remove listeners when their owner is destroyed.
- [ ] Unschedule callbacks when their owner is destroyed.
- [ ] Prevent duplicate hook or callback registration.
- [ ] Ensure initialization failure does not leave partial UI behind.
- [ ] Ensure leaving a session clears session-specific UI state.
- [ ] Ensure reopening a menu does not reuse stale session data.
- [ ] Confirm the mod can compile without accidentally including unavailable Globed headers when optional support is intended.
- [ ] Verify dependency and include configuration for the chosen integration model.
- [ ] Build with the final intended integration configuration.

## Phase 13 — Visual polish

- [ ] Standardize panel colors.
- [ ] Standardize button styles.
- [ ] Standardize typography.
- [ ] Standardize spacing.
- [ ] Standardize input dimensions.
- [ ] Align labels and controls consistently.
- [ ] Check text contrast.
- [ ] Check long text and clipping.
- [ ] Check menu transitions.
- [ ] Add animations only where they clarify state changes.
- [ ] Keep animations short.
- [ ] Avoid blocking input during unnecessary animation.
- [ ] Check all panels against the game's visual style.
- [ ] Remove redundant labels.
- [ ] Remove unnecessary decorative elements.
- [ ] Check that important status information remains visible.
- [ ] Check small and large resolutions.
- [ ] Check UI after a scene transition.
- [ ] Check UI after host/join failures.
- [ ] Check UI after reconnecting.
- [ ] Do not start a full redesign during the final stabilization week.

## Phase 14 — Regression testing

### Core behavior
- [ ] Host a room.
- [ ] Join a room.
- [ ] Leave a room.
- [ ] Repeatedly host and leave.
- [ ] Repeatedly join and leave.
- [ ] Try an invalid room.
- [ ] Try a wrong passcode.
- [ ] Try a full room.
- [ ] Test a connection timeout.
- [ ] Test a disconnect.
- [ ] Test reconnecting.
- [ ] Test view-only permissions.
- [ ] Test multiple clients.
- [ ] Verify level edits still synchronize.
- [ ] Verify chat still works.
- [ ] Verify cursor sync still works.
- [ ] Verify UI state follows actual session state.

### UI and lifecycle
- [ ] Open and close every screen repeatedly.
- [ ] Navigate backward and forward through the UI.
- [ ] Check for duplicate panels.
- [ ] Check for duplicate callbacks.
- [ ] Check for stale player lists.
- [ ] Check for stale room codes.
- [ ] Check for stale error messages.
- [ ] Check for labels outside panel bounds.
- [ ] Check for overlapping buttons.
- [ ] Check that no invisible control intercepts clicks.
- [ ] Check that the HUD disappears when appropriate.
- [ ] Check that scheduled updates stop when nodes are destroyed.
- [ ] Check for crashes after repeated scene transitions.
- [ ] Check for crashes when leaving during a connection attempt.

### Mod compatibility
- [ ] Test with required dependencies enabled.
- [ ] Test with Globed enabled.
- [ ] Test with Globed disabled.
- [ ] Test without Globed if the dependency model allows it.
- [ ] Test with Eclipse Menu.
- [ ] Test with DevTools.
- [ ] Test with other relevant UI mods.
- [ ] Isolate conflicts by changing one variable at a time.
- [ ] Save logs for reproducible crashes.
- [ ] Verify the final test environment is Geometry Dash 2.2091.

## Phase 15 — Release packaging

- [ ] Freeze new features.
- [ ] Review the diff for accidental changes.
- [ ] Remove temporary experiments.
- [ ] Remove dead code.
- [ ] Remove debug-only UI.
- [ ] Check resource filenames and paths.
- [ ] Check resource packaging.
- [ ] Check `mod.json` metadata.
- [ ] Check dependency declarations.
- [ ] Check the target version/build configuration.
- [ ] Update the version only according to the project's release plan.
- [ ] Update the changelog.
- [ ] Build the Release configuration.
- [ ] Install the actual generated `.geode` package.
- [ ] Launch Geometry Dash 2.2091 from a clean start.
- [ ] Verify the mod loads.
- [ ] Verify the Host/Join flow.
- [ ] Verify SessionStatusNode and HUD.
- [ ] Verify player list and chat.
- [ ] Verify Globed entry point.
- [ ] Verify expected behavior when Globed is disabled.
- [ ] Test one complete two-client session.
- [ ] Review logs for startup errors.
- [ ] Record known issues.
- [ ] Tag or prepare the release commit.
- [ ] Publish only after the packaged build passes the test checklist.

## Suggested schedule

### Sep 24–28 — Unblock and establish the baseline
- [ ] Confirm the 2.2091 migration requirements.
- [ ] Fix the shared Panel empty-title behavior.
- [ ] Fix SessionStatusNode overlap and duplicate text.
- [ ] Build and test.
- [ ] Commit the changes.

### Sep 29–Oct 5 — HUD and player list
- [ ] Finish SessionStatusNode.
- [ ] Add room information and player count.
- [ ] Implement the basic player list.
- [ ] Wire the UI to real session data.
- [ ] Test with two clients.
- [ ] Commit each finished component.

### Oct 6–12 — Host/Join and chat
- [ ] Improve Host and Join screens.
- [ ] Add loading and error states.
- [ ] Improve chat presentation.
- [ ] Test failure/retry paths.
- [ ] Re-test on 2.2091.

### Oct 13–19 — Globed integration
- [ ] Confirm compatibility and dependency strategy.
- [ ] Implement the Globed entry point.
- [ ] Verify callbacks and navigation.
- [ ] Test with Globed enabled and disabled.
- [ ] Fix menu overlap and lifecycle issues.

### Oct 20–25 — Presence and visual polish
- [ ] Improve remote cursor labels.
- [ ] Add useful join/leave notifications.
- [ ] Polish spacing, colors, and transitions.
- [ ] Avoid major architectural rewrites.
- [ ] Finish outstanding high-priority UI tasks.

### Oct 26–28 — Stabilization
- [ ] Freeze new features.
- [ ] Run the regression checklist.
- [ ] Fix crashes, broken callbacks, and stale state.
- [ ] Test the packaged build on 2.2091.

### Oct 29–Nov 1 — Release preparation
- [ ] Finalize version metadata and changelog.
- [ ] Build Release.
- [ ] Test the generated `.geode` package.
- [ ] Verify the Globed integration.
- [ ] Fix only release-blocking bugs.
- [ ] Prepare the v0.9.0 release.

## Definition of done

A task is not complete merely because it compiles. For each feature:

- [ ] The code builds.
- [ ] The feature works in Geometry Dash.
- [ ] The relevant success path was tested.
- [ ] The relevant failure path was tested where applicable.
- [ ] The UI does not overlap or intercept unrelated controls.
- [ ] Callbacks/listeners have a safe lifecycle.
- [ ] The feature still works on Geometry Dash 2.2091.
- [ ] The change is committed.
- [ ] The next task is written down.

## Priority order if time gets tight

1. [ ] Geometry Dash 2.2091 migration and successful startup.
2. [ ] Fix Panel and SessionStatusNode.
3. [ ] Reliable session HUD.
4. [ ] Player list using real data.
5. [ ] Reliable Host/Join flow.
6. [ ] Globed entry point that doesn't break other menus.
7. [ ] Chat and useful notifications.
8. [ ] Cursor labels and cosmetic polish.
9. [ ] Nonessential animations and decoration.

**Reminder:** this is a backlog, not a promise to finish every checkbox by November 1. A stable, tested UI and a working 2.2091 migration are more valuable than a larger set of half-finished features.
