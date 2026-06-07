extends Camera2D

var zoom_speed: float = 0.1
var min_zoom: float = 0.5
var max_zoom: float = 3.0

var is_dragging: bool = false
var drag_start_mouse_pos: Vector2
var drag_start_camera_pos: Vector2

func _unhandled_input(event: InputEvent) -> void:
	# Fare tekerleği ile Zoom in/out
	if event is InputEventMouseButton:
		if event.button_index == MOUSE_BUTTON_WHEEL_UP and event.pressed:
			zoom += Vector2(zoom_speed, zoom_speed)
		elif event.button_index == MOUSE_BUTTON_WHEEL_DOWN and event.pressed:
			zoom -= Vector2(zoom_speed, zoom_speed)
			
		zoom.x = clamp(zoom.x, min_zoom, max_zoom)
		zoom.y = clamp(zoom.y, min_zoom, max_zoom)

		# Sürükleme (Pan) işlemi
		if event.button_index == MOUSE_BUTTON_LEFT:
			if event.pressed:
				is_dragging = true
				drag_start_mouse_pos = event.position
				drag_start_camera_pos = position
			else:
				is_dragging = false

	# Fare hareket ettikçe kameranın pozisyonunu güncelle
	if event is InputEventMouseMotion and is_dragging:
		var zoom_factor = 1.0 / zoom.x # Zoom yapıldığında sürükleme hızını dengeler
		var drag_vector = (drag_start_mouse_pos - event.position) * zoom_factor
		position = drag_start_camera_pos + drag_vector
