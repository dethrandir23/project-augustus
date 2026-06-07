extends Node

var backend: GodotAugustus

var current_speed: int = 1
var is_paused: bool = true

func _ready():
	backend = GodotAugustus.new()
	add_child(backend)
	
	# 1. Motoru başlat
	backend.init()
	
	# 2. Tüm JSON dosyalarını recursive okuyup engine'e yükle
	var ok = backend.load_data_directory("res://core/data/")
	print("Data loaded: ", ok)

	# 3. Senaryo başlat
	backend.start_scenario("debug_scenario")
	
	# 4. Oyuncu şirketini oluştur
	backend.set_player("My Company", "core_startup_company_001", false)
	
	# 5. İlk tick
	backend.step()
	
	print("Oyun hazir!")
