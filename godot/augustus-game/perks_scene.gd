extends Control

func _ready() -> void:
	update_panel()

func update_panel() -> void:
	var json_string = GameManager.backend.get_player_state()
	var json = JSON.new()
	var error = json.parse(json_string)
	
	if error == OK:
		var data = json.get_data()
		
		# 1. Oyun durumundan (Backend) oyuncunun sahip olduğu perk dizisini çekiyoruz
		var active_perks = data["components"]["PerkComponent"]["activePerks"]
		var item_list = $Panel/ScrollContainer/ItemList
		
		# 2. Eski listeyi temizliyoruz
		for child in item_list.get_children():
			child.queue_free()
			
		# 3. Yeni perkleri listeye diziyoruz
		for perk in active_perks:
			var perk_id = perk["id"]
			var perk_duration = float(perk["duration"])
			
			# YENİ: RegistryManager'dan perk'in sabit tanım dosyasını çekiyoruz
			var perk_def = RegistryManager.get_perk(perk_id)
			
			# Ana kapsayıcı (Alt alta dizmek için VBox kullanıyoruz ki açıklama sığsın)
			var row_container = VBoxContainer.new()
			# Perk içindeki yazılar arasında biraz boşluk bırakalım
			row_container.add_theme_constant_override("separation", 2)
			
			# --- 1. BAŞLIK VE SÜRE SATIRI (HBox - Yanyana) ---
			var header_row = HBoxContainer.new()
			
			var name_label = Label.new()
			# Tanımdan ismi alıp çeviriden geçiriyoruz (Tanım yoksa veya hata varsa sadece ID yazsın)
			if perk_def.has("name"):
				name_label.text = LocalizationManager.get_text(perk_def["name"])
			else:
				name_label.text = perk_id
				
			name_label.custom_minimum_size = Vector2(300, 0)
			name_label.add_theme_color_override("font_color", Color(0.9, 0.7, 0.2)) # Altınımsı havalı bir başlık rengi
			
			var duration_label = Label.new()
			if perk_duration < 0:
				duration_label.text = "Kalıcı"
			else:
				duration_label.text = "Süre: " + str(perk_duration) + " Tur"
				
			duration_label.custom_minimum_size = Vector2(100, 0)
			duration_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT
			
			header_row.add_child(name_label)
			header_row.add_child(duration_label)
			row_container.add_child(header_row)
			
			# --- 2. AÇIKLAMA (DESCRIPTION) ---
			if perk_def.has("description"):
				var desc_label = Label.new()
				desc_label.text = LocalizationManager.get_text(perk_def["description"])
				desc_label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART # Metin uzunsa alt satıra geçsin
				desc_label.add_theme_color_override("font_color", Color(0.7, 0.7, 0.7)) # Soluk gri (Lore metni hissi)
				row_container.add_child(desc_label)
				
			# --- 3. ETKİLER (EFFECTS) ---
			if perk_def.has("effects") and perk_def["effects"].size() > 0:
				var effects_label = Label.new()
				var effects_text = "Etkiler:\n"
				
				for effect in perk_def["effects"]:
					# İleride çeviri json'ına "effect_debt_interest_rate" gibi keyler ekleyebilirsin
					var eff_type = LocalizationManager.get_text("effect_" + effect["type"])
					
					# Eğer sözlükte karşılığını henüz yazmadıysan, bozuk görünmesin diye alt tireleri silip düzgün yazdıralım
					if eff_type == "effect_" + effect["type"]:
						eff_type = effect["type"].capitalize().replace("_", " ")
						
					# Örn: -0.10 olan veriyi -10% şeklinde formata sokuyoruz
					var eff_val = effect["value"] * 100
					var sign_str = "+" if eff_val > 0 else "" # Pozitif değerlerin başına + koy
					
					effects_text += "  • " + eff_type + ": " + sign_str + str(eff_val) + "%\n"
					
				effects_label.text = effects_text
				effects_label.add_theme_color_override("font_color", Color(0.4, 0.8, 0.4)) # Açık yeşil (Buff rengi)
				row_container.add_child(effects_label)
				
			# --- 4. AYRAÇ ---
			# İki perk arasında görsel olarak ince bir çizgi çeker, arayüzü çok düzenli gösterir
			var separator = HSeparator.new()
			row_container.add_child(separator)
			
			# Hazırladığımız bu devasa bloğu ana dikey listeye ekliyoruz
			item_list.add_child(row_container)
			
	else:
		print("json parse error in PerksPanel")
