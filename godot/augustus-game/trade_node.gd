extends Node2D

var city_data : Dictionary
@onready var button = $Button

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	button.pressed.connect(_on_button_pressed)

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	pass
	
func setup(data: Dictionary) -> void:
	city_data = data
	if city_data.has("name"):
		button.text = str(city_data["name"])

func _on_button_pressed() -> void:
	# Sinyali global olarak fırlat
	SignalBus.city_selected.emit(city_data)
