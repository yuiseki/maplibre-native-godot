extends Control

const STYLE_URLS := [
	"https://demotiles.maplibre.org/style.json",
	"https://tile.openstreetmap.jp/styles/osm-bright/style.json",
]

var map_node
var zoom_label: Label


func _ready() -> void:
	_build_ui()
	_apply_initial_state()


func _build_ui() -> void:
	set_anchors_preset(Control.PRESET_FULL_RECT)

	var root := VBoxContainer.new()
	root.set_anchors_preset(Control.PRESET_FULL_RECT)
	root.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	root.size_flags_vertical = Control.SIZE_EXPAND_FILL
	add_child(root)

	var toolbar := HBoxContainer.new()
	toolbar.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	root.add_child(toolbar)

	var style_picker := OptionButton.new()
	for style_url in STYLE_URLS:
		style_picker.add_item(style_url)
	style_picker.item_selected.connect(_on_style_selected)
	toolbar.add_child(style_picker)

	toolbar.add_child(_make_city_button("Paris", 48.8566, 2.3522, 10.0))
	toolbar.add_child(_make_city_button("New York", 40.7128, -74.0060, 10.0))
	toolbar.add_child(_make_city_button("Tokyo", 35.6895, 139.6917, 10.0))

	var pitch_text := Label.new()
	pitch_text.text = "Pitch:"
	toolbar.add_child(pitch_text)

	var pitch_slider := HSlider.new()
	pitch_slider.min_value = 0.0
	pitch_slider.max_value = 100.0
	pitch_slider.step = 1.0
	pitch_slider.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	pitch_slider.value_changed.connect(_on_pitch_changed)
	toolbar.add_child(pitch_slider)

	var bearing_text := Label.new()
	bearing_text.text = "Bearing:"
	toolbar.add_child(bearing_text)

	var bearing_slider := HSlider.new()
	bearing_slider.min_value = 0.0
	bearing_slider.max_value = 100.0
	bearing_slider.step = 1.0
	bearing_slider.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	bearing_slider.value_changed.connect(_on_bearing_changed)
	toolbar.add_child(bearing_slider)

	zoom_label = Label.new()
	zoom_label.text = "Zoom: 1"
	toolbar.add_child(zoom_label)

	map_node = ClassDB.instantiate("MapLibreMap")
	if map_node == null:
		push_error("failed to instantiate MapLibreMap")
		return
	map_node.custom_minimum_size = Vector2(800, 540)
	map_node.expand_mode = TextureRect.EXPAND_IGNORE_SIZE
	map_node.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
	map_node.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	map_node.size_flags_vertical = Control.SIZE_EXPAND_FILL
	root.add_child(map_node)


func _apply_initial_state() -> void:
	map_node.set_style_url(STYLE_URLS[0])
	map_node.fly_to(0.0, 0.0, 1.0)
	map_node.set_pitch(0.0)
	map_node.set_bearing(0.0)
	_refresh_zoom_label()


func _make_city_button(label_text: String, lat: float, lon: float, zoom: float) -> Button:
	var button := Button.new()
	button.text = label_text
	button.pressed.connect(func() -> void:
		map_node.fly_to(lat, lon, zoom)
		_refresh_zoom_label()
	)
	return button


func _on_style_selected(index: int) -> void:
	map_node.set_style_url(STYLE_URLS[index])


func _on_pitch_changed(value: float) -> void:
	map_node.set_pitch(value / 100.0 * 60.0)


func _on_bearing_changed(value: float) -> void:
	map_node.set_bearing(value / 100.0 * 360.0)


func _refresh_zoom_label() -> void:
	zoom_label.text = "Zoom: %d" % int(round(map_node.get_current_zoom()))
