## Advanced Label

Custom made label class for use with Geometry Dash/Geode.

### Features
- Multi-font support: handles missing characters and uses fallback fonts
- Supports both single-line and multi-line text with word wrapping
- Emoji support (both static sprites and custom embedded nodes)
- Performance optimizations: has slightly better performance than CCLabelBMFont

### TODO
- Implement multi-emoji maps (to use several spritesheets for emojis)
- Use UTF-8 instead of UTF-32 to avoid extra memory usage (parsing emojis would be harder though)
- Add `TextArea`-like tags for coloring and animating text
