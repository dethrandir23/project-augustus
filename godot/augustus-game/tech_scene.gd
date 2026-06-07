extends Control

func _ready() -> void:
	update_panel()

func update_panel() -> void:
	var json_string = GameManager.backend.get_player_state()
	var json = JSON.new()
	var error = json.parse(json_string)
	
	if error == OK:
		var data = json.get_data()
		
		# 1. Bilinen teknolojileri JSON'dan çekiyoruz
		var known_techs = data["components"]["TechTreeComponent"]["knownTechnologies"]
		var item_list = $Panel/ScrollContainer/ItemList
		
		# 2. Eski listeyi temizle
		for child in item_list.get_children():
			child.queue_free()
			
		# 3. Teknolojileri listeye ekle
		for tech_id in known_techs:
			# Sabit veri yöneticimizden teknolojinin özelliklerini alıyoruz
			var tech_def = RegistryManager.get_technology(tech_id)
			
			var row_container = VBoxContainer.new()
			row_container.add_theme_constant_override("separation", 5)
			
			# --- 1. BAŞLIK VE SEVİYE (HBox - Yanyana) ---
			var header_row = HBoxContainer.new()
			
			var name_label = Label.new()
			if tech_def.has("name"):
				name_label.text = LocalizationManager.get_text(tech_def["name"])
			else:
				name_label.text = tech_id
				
			name_label.custom_minimum_size = Vector2(300, 0)
			# Bilim ve teknoloji menülerine yakışacak saykodelik/neon bir mavi
			name_label.add_theme_color_override("font_color", Color(0.2, 0.6, 0.9)) 
			
			var tier_label = Label.new()
			if tech_def.has("tier"):
				tier_label.text = "Seviye " + str(tech_def["tier"])
			
			tier_label.custom_minimum_size = Vector2(100, 0)
			tier_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT
			tier_label.add_theme_color_override("font_color", Color(0.6, 0.6, 0.6))
			
			header_row.add_child(name_label)
			header_row.add_child(tier_label)
			row_container.add_child(header_row)
			
			# --- 2. AÇILAN ÖZELLİKLER (UNLOCKS) ---
			if tech_def.has("unlocks") and tech_def["unlocks"].size() > 0:
				var unlocks_label = Label.new()
				var unlocks_text = "Açılanlar:\n"
				
				for unlock in tech_def["unlocks"]:
					# İleride sözlüğe "can_do_agriculture": "Tarım Yapabilir" eklersin
					unlocks_text += "  • " + LocalizationManager.get_text(unlock) + "\n"
					
				unlocks_label.text = unlocks_text
				unlocks_label.add_theme_color_override("font_color", Color(0.9, 0.6, 0.2)) # Turuncu
				row_container.add_child(unlocks_label)
				
			# --- 3. ETKİLER (EFFECTS) ---
			if tech_def.has("effects") and tech_def["effects"].size() > 0:
				var effects_label = Label.new()
				var effects_text = "Bonuslar:\n"
				
				for effect in tech_def["effects"]:
					var eff_type = LocalizationManager.get_text("effect_" + effect["type"])
					
					if eff_type == "effect_" + effect["type"]:
						eff_type = effect["type"].capitalize().replace("_", " ")
						
					var eff_val = effect["value"] * 100
					var sign_str = "+" if eff_val > 0 else ""
					
					effects_text += "  • " + eff_type + ": " + sign_str + str(eff_val) + "%\n"
					
				effects_label.text = effects_text
				effects_label.add_theme_color_override("font_color", Color(0.4, 0.8, 0.4)) # Buff Yeşili
				row_container.add_child(effects_label)
				
			# --- 4. AYRAÇ ---
			var separator = HSeparator.new()
			row_container.add_child(separator)
			
			item_list.add_child(row_container)
			
	else:
		print("json parse error in TechPanel")
