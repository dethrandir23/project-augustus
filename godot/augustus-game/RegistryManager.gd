extends Node

# Verilerin tutulacağı devasa sözlüklerimiz
var companies = {}
var events = {}
var factories = {}
var items = {}
var perks = {}
var pipelines = {}
var technologies = {}
var tradenodes = {}

# JSON'dan okunan 'registry_type' stringlerini doğrudan bizim değişkenlere bağlayan harita
@onready var registry_map = {
	"COMPANY_DEFINITIONS": companies,
	"EVENT_DEFINITIONS": events,
	"FACTORY_DEFINITIONS": factories,
	"ITEM_DEFINITIONS": items,
	"PERK_DEFINITIONS": perks,
	"PIPELINE_DEFINITIONS": pipelines,
	"TECHNOLOGY_DEFINITIONS": technologies,
	"TRADENODE_DEFINITIONS": tradenodes
}

func _ready():
	# JSON dosyalarının bulunduğu yolu buraya yaz kanka.
	# Ben örnek olarak res://data/ koydum, kendi klasör yapına göre değiştirirsin.
	var data_folder = "res://core/data/common/" 
	load_all_json_files_in_directory(data_folder)

# Klasörü tarayıp içindeki tüm .json dosyalarını otomatik okuyan fonksiyon
func load_all_json_files_in_directory(path: String):
	var dir = DirAccess.open(path)
	if dir:
		dir.list_dir_begin()
		var file_name = dir.get_next()
		
		while file_name != "":
			if not dir.current_is_dir() and file_name.ends_with(".json"):
				parse_and_store_registry(path + "/" + file_name)
			file_name = dir.get_next()
		dir.list_dir_end()
		print("--- Tüm Registry verileri Godot hafızasına yüklendi! ---")
	else:
		push_error("Veri klasörüne ulaşılamadı: " + path)

# JSON'ı parçalayıp doğru sözlüğe ekleyen asıl işçi fonksiyon
func parse_and_store_registry(file_path: String):
	var file = FileAccess.open(file_path, FileAccess.READ)
	var json = JSON.new()
	var error = json.parse(file.get_as_text())
	
	if error == OK:
		var data = json.get_data()
		
		# Dosya bizim registry standartlarımıza uyuyor mu kontrolü
		if data.has("registry_type") and data.has("entries"):
			var type = data["registry_type"]
			
			if registry_map.has(type):
				var target_dict = registry_map[type] # Hedef sözlüğü seç (Örn: perks)
				
				# Bütün entry'leri ID'lerini anahtar (key) yaparak sözlüğe diz
				for entry in data["entries"]:
					if entry.has("id"):
						target_dict[entry["id"]] = entry
				
				print("[OK] ", file_path.get_file(), " yüklendi -> ", type)
			else:
				push_warning("Bilinmeyen registry_type bulundu: " + type + " (Dosya: " + file_path + ")")
		else:
			push_warning("Dosya Registry formatına uymuyor: " + file_path)
	else:
		push_error("JSON Parse Hatası: " + file_path)


# ==========================================
# ARAYÜZLERİN KULLANACAĞI GETTER FONKSİYONLARI
# ==========================================

# Eğer istenen ID bulunamazsa boş sözlük {} döndürürüz ki kod null hatası verip çökmesin.

func get_company(id: String) -> Dictionary:
	return companies.get(id, {})

func get_event(id: String) -> Dictionary:
	return events.get(id, {})

func get_factory(id: String) -> Dictionary:
	return factories.get(id, {})

func get_item(id: String) -> Dictionary:
	return items.get(id, {})

func get_perk(id: String) -> Dictionary:
	return perks.get(id, {})

func get_pipeline(id: String) -> Dictionary:
	return pipelines.get(id, {})

func get_technology(id: String) -> Dictionary:
	return technologies.get(id, {})

func get_tradenode(id: String) -> Dictionary:
	return tradenodes.get(id, {})
