extends Control

# Bu fonksiyon sahne oyuna (ekrana) girdiği saniye otomatik çalışır
func _ready():
	# Güncelleme fonksiyonumuzu çağıralım
	update_topbar()

# Verileri C++'tan çekip ekrana yazacak özel fonksiyonumuz
func update_topbar():
	# 1. GameManager Autoload'umuz üzerinden C++ backendine ulaşıp veriyi çekiyoruz
	var json_string = GameManager.backend.get_player_state()
	
	# 2. Gelen bu stringi Godot'nun okuyabileceği bir "Sözlük (Dictionary)" yapısına çeviriyoruz
	var json = JSON.new()
	var error = json.parse(json_string)
	
	if error == OK:
		var data = json.get_data() # data artık bir sözlük oldu!
		# print(data)
		# print("Ama topbarda: print(str(GameManager.backend.get_serialized_state())) yaptigimda:")
		# print(str(GameManager.backend.get_serialized_state()))
		# DisplayServer.clipboard_set(str(GameManager.backend.get_serialized_state()))
		DisplayServer.clipboard_set(str(GameManager.backend.get_economy_summary()))
		print("Veri panoya kopyalandı, bir yere yapıştırabilirsin!")
		# 3. Ekrandaki yazıları (Label) güncelliyoruz. 
		# NOT: "$" işareti Godot'da "bu isimdeki alt düğümü (child node) bul" demektir.
		
		# Kendi C++'ındaki JSON anahtarlarına göre buraları değiştirebilirsin ("name", "capital" vb.)
		$Panel/HBoxContainer/CompanyNameLabel.text = "Comp.: " + str(data["name"])
		$Panel/HBoxContainer/MoneyLabel.text = " $" + str(data["components"]["WalletComponent"]["balance"])
	else:
		print("C++'tan gelen JSON okunamadı! Hata!")
