extends Control

const STYLE_URLS := [
	"https://demotiles.maplibre.org/style.json",
	"https://tile.openstreetmap.jp/styles/osm-bright/style.json",
]

var map_node
var zoom_label: Label
var perf_label: Label   # shows last render time in ms


func _ready() -> void:
	_build_ui()
	_apply_initial_state()


func _build_ui() -> void:
	set_anchors_preset(Control.PRESET_FULL_RECT)

	var root := VBoxContainer.new()
	root.set_anchors_preset(Control.PRESET_FULL_RECT)
	root.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	root.size_flags_vertical   = Control.SIZE_EXPAND_FILL
	add_child(root)

	var toolbar := HBoxContainer.new()
	toolbar.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	root.add_child(toolbar)

	var style_picker := OptionButton.new()
	for style_url in STYLE_URLS:
		style_picker.add_item(style_url)
	style_picker.item_selected.connect(_on_style_selected)
	toolbar.add_child(style_picker)

	toolbar.add_child(_make_city_button("Paris",    48.8566,   2.3522,  10.0))
	toolbar.add_child(_make_city_button("New York", 40.7128, -74.0060,  10.0))
	toolbar.add_child(_make_city_button("Tokyo",    35.6895, 139.6917,  10.0))

	var bench_button := Button.new()
	bench_button.text = "Benchmark (10)"
	bench_button.pressed.connect(_run_benchmark)
	toolbar.add_child(bench_button)

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
	bearing_slider.max_value = 360.0
	bearing_slider.step = 1.0
	bearing_slider.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	bearing_slider.value_changed.connect(_on_bearing_changed)
	toolbar.add_child(bearing_slider)

	zoom_label = Label.new()
	zoom_label.text = "Zoom: 1"
	toolbar.add_child(zoom_label)

	# Performance label — always visible below the toolbar
	perf_label = Label.new()
	perf_label.text = "Render: — ms"
	perf_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT
	root.add_child(perf_label)

	map_node = ClassDB.instantiate("MapLibreMap")
	if map_node == null:
		push_error("failed to instantiate MapLibreMap")
		return
	map_node.custom_minimum_size = Vector2(800, 540)
	map_node.expand_mode  = TextureRect.EXPAND_IGNORE_SIZE
	map_node.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
	map_node.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	map_node.size_flags_vertical   = Control.SIZE_EXPAND_FILL
	root.add_child(map_node)


func _apply_initial_state() -> void:
	map_node.set_style_url(STYLE_URLS[0])
	map_node.fly_to(0.0, 0.0, 1.0)
	map_node.set_pitch(0.0)
	map_node.set_bearing(0.0)
	_refresh_labels()


func _make_city_button(label_text: String, lat: float, lon: float, zoom: float) -> Button:
	var button := Button.new()
	button.text = label_text
	button.pressed.connect(func() -> void:
		map_node.fly_to(lat, lon, zoom)
		_refresh_labels()
	)
	return button


func _on_style_selected(index: int) -> void:
	map_node.set_style_url(STYLE_URLS[index])
	_refresh_labels()


func _on_pitch_changed(value: float) -> void:
	map_node.set_pitch(value / 100.0 * 60.0)
	_refresh_labels()


func _on_bearing_changed(value: float) -> void:
	map_node.set_bearing(value)
	_refresh_labels()


func _refresh_labels() -> void:
	zoom_label.text = "Zoom: %d" % int(round(map_node.get_current_zoom()))
	perf_label.text = "Render: %d ms" % map_node.get_last_render_ms()


# ---------------------------------------------------------------------------
# Benchmark: fly_to Paris then collect tick times over 90 frames (~3 s).
# With Continuous mode each process frame calls tick() so we measure
# sustained FPS across a full fly_to animation arc.
# ---------------------------------------------------------------------------

const BENCH_FRAMES := 90   # ~3 s at 30 fps

func _run_benchmark() -> void:
	print("=== Benchmark start (%d frames) ===" % BENCH_FRAMES)
	map_node.fly_to(48.8566, 2.3522, 10.0)

	var times: Array[int] = []
	for _i in range(BENCH_FRAMES):
		await get_tree().process_frame
		times.append(map_node.get_last_render_ms())
		_refresh_labels()

	var total := 0
	var mn    := times[0]
	var mx    := times[0]
	for t in times:
		total += t
		mn = min(mn, t)
		mx = max(mx, t)
	var avg := float(total) / times.size()
	var fps := 1000.0 / avg if avg > 0 else 0.0

	var result := "=== Benchmark: %d frames | avg %.1f ms (%.1f fps) | min %d ms | max %d ms ===" % [
		times.size(), avg, fps, mn, mx
	]
	print(result)
	perf_label.text = result
