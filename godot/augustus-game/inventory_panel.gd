extends Control

func _ready() -> void:
	update_panel()

func _process(delta: float) -> void:
	pass

func update_panel() -> void:
	var json_string = GameManager.backend.get_player_state()
	var json = JSON.new()
	var error = json.parse(json_string)
	
	if error == OK:
		var data = json.get_data()
		
		# 1. Envanter dizisini C++ JSON'ından çekiyoruz
		var items = data["components"]["Storage"]["items"]
		var item_list = $Panel/ScrollContainer/ItemList
		
		# 2. TEMİZLİK ZAMANI: Her turda aynı listeyi üst üste eklememesi için
		# önce içindeki eski satırları yok etmeliyiz.
		for child in item_list.get_children():
			child.queue_free()
			
		# 3. YENİ LİSTEYİ DİZİYORUZ
		for item in items:
			var item_id = item["id"]
			var item_amount = float(item["amount"]) # JSON'dan float geliyor
			
			# Ana satırımız (İleride butonları falan buna ekleyeceksin)
			var row = HBoxContainer.new()
			
			# Eşya İsmi Label'ı
			var name_label = Label.new()
			# TODO: İleride burada bir çeviri sözlüğü kullanacağız
			name_label.text = LocalizationManager.get_text(item_id)
			# İsimler düzgün hizalansın diye minimum bir genişlik veriyoruz
			name_label.custom_minimum_size = Vector2(250, 0) 
			
			# Miktar Label'ı
			var amount_label = Label.new()
			amount_label.text = str(item_amount) + " Adet"
			amount_label.custom_minimum_size = Vector2(100, 0)
			
			# Düğümleri satıra takıyoruz
			row.add_child(name_label)
			row.add_child(amount_label)
			
			# İLERİYE DÖNÜK EXPANDABLE YAPI (Örnek Buton):
			# İleride al/sat butonu eklemek istersen aynen şöyle yapacaksın:
			# var sell_btn = Button.new()
			# sell_btn.text = "Sat"
			# row.add_child(sell_btn)
			
			# En son, hazırladığımız bu satırı ekrandaki dikey listemize basıyoruz
			item_list.add_child(row)
			
	else:
		print("json parse error")
