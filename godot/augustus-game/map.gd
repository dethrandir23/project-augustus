extends Node2D

# Sahneyi önceden belleğe yüklüyoruz
const TradeNodeScene = preload("res://TradeNode.tscn")

@onready var trade_nodes_layer = $TradeNodesLayer
@onready var background = $Background

func _ready() -> void:
	background.texture = load("res://Assets/MockBackground.png")
	create_nodes()

func create_nodes() -> void:
	var json_string = GameManager.backend.get_serialized_state()
	var json = JSON.new()
	var error = json.parse(json_string)
	
	if error == OK:
		var data = json.get_data()
		
		if typeof(data) == TYPE_DICTIONARY and data.has("entities"):
			var entities = data["entities"]
			
			for e in entities:
				if e.has("type") and e["type"] == "trade_node":
					var node_instance = TradeNodeScene.instantiate()
					trade_nodes_layer.add_child(node_instance)
					
					if e.has("x") and e.has("y"):
						node_instance.position = Vector2(e["x"], e["y"])
					
					if node_instance.has_method("setup"):
						node_instance.setup(e)
		else:
			push_error("HATA: Gelen JSON içinde 'entities' keyi bulunamadı veya data Dictionary değil!")
	else:
		push_error("JSON Parse Hatası: " + json.get_error_message())
