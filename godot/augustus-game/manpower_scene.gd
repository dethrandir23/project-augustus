extends Control


# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	update_panel()


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	pass
 
func update_panel() -> void:
	var json_string = GameManager.backend.get_player_state()
	var json = JSON.new()
	var error = json.parse(json_string)
	if error == OK:
		var data = json.get_data()
		$Panel/VBoxContainer/ManpowerLabel.text = "Manpower: " + str(data["components"]["ManpowerPoolComponent"]["balance"])
	else:
		print("json parse error")
