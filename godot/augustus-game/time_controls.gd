extends Control

@onready var speed_buttons = [
	$Panel/VBoxContainer/HBoxContainer/Speed1,
	$Panel/VBoxContainer/HBoxContainer/Speed2,
	$Panel/VBoxContainer/HBoxContainer/Speed3,
	$Panel/VBoxContainer/HBoxContainer/Speed4,
	$Panel/VBoxContainer/HBoxContainer/Speed5
]
@onready var play_pause_btn = $Panel/VBoxContainer/HBoxContainer/PlayPauseButton
@onready var date_label = $Panel/VBoxContainer/DateLabel

func _ready():
	for i in range(speed_buttons.size()):
		speed_buttons[i].pressed.connect(_on_speed_button_pressed.bind(i + 1))
		
	play_pause_btn.pressed.connect(_on_play_pause_pressed)
	set_speed_ui(1)
	
	# Oyun ilk açıldığında tarihi bir kere ekrana yazdıralım
	update_time_display()

# _process(delta) TAMAMEN SİLİNDİ!

func update_time_display():
	var json_string = GameManager.backend.get_serialized_state()
	var json = JSON.new()
	var error = json.parse(json_string)
	
	if error == OK:
		var data = json.get_data()
		
		# null yerine .has() kullanmak Godot'da Dictionary için daha güvenlidir
		if data.has("date"): 
			date_label.text = str(data["date"])
		else:
			date_label.text = "Tarih Bulunamadı"
	else:
		print("C++'tan gelen JSON okunamadı! Hata!")

func _on_speed_button_pressed(speed: int):
	GameManager.current_speed = speed
	GameManager.is_paused = false
	play_pause_btn.text = "PAUSE"
	set_speed_ui(speed)

func set_speed_ui(target_speed: int):
	for i in range(speed_buttons.size()):
		if (i + 1) <= target_speed:
			speed_buttons[i].button_pressed = true
		else:
			speed_buttons[i].button_pressed = false

func _on_play_pause_pressed():
	GameManager.is_paused = !GameManager.is_paused
	
	if GameManager.is_paused:
		play_pause_btn.text = "PLAY"
		set_speed_ui(0)
	else:
		play_pause_btn.text = "PAUSE"
		set_speed_ui(GameManager.current_speed)
