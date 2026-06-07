extends Control

@onready var close_button = $Panel/Button
@onready var vbox = $Panel/ScrollContainer/VBoxContainer

func _ready() -> void:
	close_button.pressed.connect(_on_close)
	visible = false

func show_city_details(city_data: Dictionary):
	# 1. Önceki şehirden kalan yazıları temizle
	for child in vbox.get_children():
		child.queue_free()
		
	# 2. Şehir adını en üste büyük yaz
	add_ui_row("Şehir: " + str(city_data.get("name", "Bilinmiyor")))
	add_ui_row("--------------------")
	
	# 3. JSON Component'larının içine dal ve verileri bas
	if city_data.has("components"):
		var comps = city_data["components"]
		
		# Demografik veriler
		if comps.has("DemographicsComponent"):
			var demo = comps["DemographicsComponent"]
			add_ui_row("Nüfus: " + str(demo.get("population", 0)))
			add_ui_row("Mutluluk: " + str(demo.get("happiness", 0.0)).pad_decimals(2))
			
		# Ekonomi verileri
		if comps.has("WalletComponent"):
			var wallet = comps["WalletComponent"]
			add_ui_row("Hazine: " + str(wallet.get("balance", 0)))
			
		# Depo verileri (Array içinde döngü)
		if comps.has("Storage"):
			add_ui_row("--- DEPO ---")
			var items = comps["Storage"].get("items", [])
			if items.is_empty():
				add_ui_row("Depo boş.")
			else:
				for item in items:
					# Örn: "core_wood_001 : 100"
					add_ui_row(str(item.get("id", "Unknown")) + " : " + str(item.get("amount", 0)))
	
	# Paneli ekranda göster
	visible = true

# Dinamik olarak Label oluşturup VBoxContainer'a ekleyen yardımcı fonksiyon
func add_ui_row(text: String):
	var label = Label.new()
	label.text = text
	
	# Font boyutunu veya rengini istersen burada ayarlayabilirsin
	# label.add_theme_font_size_override("font_size", 14) 
	
	vbox.add_child(label)

func _on_close():
	visible = false
