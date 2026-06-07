extends Control

@onready var rows = $Panel/ScrollContainer/Rows
@onready var command_line = $Panel/HBoxContainer/CommandLine
@onready var send_button = $Panel/HBoxContainer/SendButton

func _ready() -> void:
	# text_submitted sinyalini doğrudan bağlayabiliriz, fonksiyon artık parametre kabul ediyor
	command_line.text_submitted.connect(_on_send_input)
	send_button.pressed.connect(_on_send_input)
	update_console()

func _unhandled_input(event):
	if event.is_action_pressed("toggle_console"):
		visible = !visible
		if visible:
			command_line.grab_focus() # Konsol açılınca direkt yazmaya başla diye ufak bir konfor bonusu

func update_console():
	var lines = GameManager.backend.read_console()
	if lines.is_empty():
		return

	# Önceki eski satırları temizlemek istersen burayı açabilirsin:
	# for child in rows.get_children():
	#     child.queue_free()

	for line in lines:
		var row = HBoxContainer.new()
		var label = Label.new()
		label.text = line

		row.add_child(label)
		rows.add_child(row)

# _new_text = "" diyerek opsiyonel parametre yaptık. 
# Enter'a basılırsa string gelir, butona basılırsa varsayılan olarak boş kalır.
func _on_send_input(_new_text: String = ""):
	var input = command_line.text.strip_edges() # Boşlukları temizle
	if input == "":
		return
		
	command_line.text = ""
	GameManager.backend.send_input(input)
	
	# Zamana yaymak için: Backend'in cevabı hazırlamasına 0.05 saniye şans veriyoruz
	await get_tree().create_timer(0.05).timeout
	update_console()
