extends Node

var dictionary = {}
var current_lang = "tr" # İleride ayarlardan değiştirilebilir

func _ready():
	load_dictionary(current_lang)

func load_dictionary(lang_code: String):
	var file_path = "res://core/data/locales/" + lang_code + ".json"
	
	# Dosya var mı kontrol et
	if not FileAccess.file_exists(file_path):
		print("Sözlük dosyası bulunamadı: ", file_path)
		return
		
	var file = FileAccess.open(file_path, FileAccess.READ)
	var json_string = file.get_as_text()
	
	var json = JSON.new()
	var error = json.parse(json_string)
	
	if error == OK:
		dictionary = json.get_data()
		print(lang_code, " sözlüğü başarıyla yüklendi!")
	else:
		print("Sözlük JSON'ı hatalı!")

# Diğer scriptlerin çağıracağı asıl fonksiyon
func get_text(id: String) -> String:
	if dictionary.has(id):
		return dictionary[id]
	return id # Eğer sözlükte karşılığı yoksa, bozulmaması için orijinal ID'yi döndür
