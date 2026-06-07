extends Control

@onready var game_tick_timer = $GameTickTimer
@onready var console = $Console

# Panel Referansları
@onready var budget_panel = $HBoxContainer/ContentArea/BudgetPanel
@onready var inventory_panel = $HBoxContainer/ContentArea/InventoryPanel
@onready var market_panel = $HBoxContainer/ContentArea/MarketPanel
@onready var manpower_panel = $HBoxContainer/ContentArea/ManpowerPanel
@onready var perks_panel = $HBoxContainer/ContentArea/PerksPanel
@onready var tech_panel = $HBoxContainer/ContentArea/TechPanel

@onready var node_details_panel = $HBoxContainer/ContentArea/NodeDetailsPanel

# Buton Referansları (Toggle durumlarını değiştirmek için)
@onready var budget_btn = $HBoxContainer/LeftMenuButtons/BudgetButton
@onready var inventory_btn = $HBoxContainer/LeftMenuButtons/InventoryButton
@onready var market_btn = $HBoxContainer/LeftMenuButtons/MarketButton
@onready var manpower_btn = $HBoxContainer/LeftMenuButtons/ManpowerButton
@onready var perks_btn = $HBoxContainer/LeftMenuButtons/PerksButton
@onready var tech_btn = $HBoxContainer/LeftMenuButtons/TechTreeButton

func _ready() -> void:
	game_tick_timer.timeout.connect(_on_game_tick_timer_timeout)
	game_tick_timer.start() # Timer'ı başlatmayı unutmayalım!
	
	# Sinyalleri doğrudan referanslar üzerinden bağlıyoruz
	budget_btn.pressed.connect(_on_budget_clicked)
	inventory_btn.pressed.connect(_on_inventory_clicked)
	market_btn.pressed.connect(_on_market_clicked)
	manpower_btn.pressed.connect(_on_manpower_clicked)
	perks_btn.pressed.connect(_on_perks_clicked)
	tech_btn.pressed.connect(_on_tech_clicked)
	
	SignalBus.city_selected.connect(_on_city_selected_from_map)
	
	close_all_panels()

func _on_game_tick_timer_timeout():
	if not GameManager.is_paused:
		GameManager.backend.step() 

		# TopBar'ı güncelle
		if has_node("TopBar"):
			$TopBar.update_topbar()
			
		# ZAMAN KONTROL PANELİNİ (TARİHİ) GÜNCELLE
		if has_node("TimeControls"):
			$TimeControls.update_time_display()
			
		# Console update
		console.update_console()
			
	var next_wait_time = 1.0 / float(GameManager.current_speed)
	if game_tick_timer.wait_time != next_wait_time:
		game_tick_timer.wait_time = next_wait_time

func close_all_panels():
	# Panelleri gizle
	budget_panel.visible = false
	inventory_panel.visible = false
	market_panel.visible = false
	manpower_panel.visible = false
	perks_panel.visible = false
	tech_panel.visible = false
	node_details_panel.visible = false
	
	# Butonların basılı kalma durumunu sıfırla
	budget_btn.button_pressed = false
	inventory_btn.button_pressed = false
	market_btn.button_pressed = false
	manpower_btn.button_pressed = false
	perks_btn.button_pressed = false
	tech_btn.button_pressed = false

func _on_budget_clicked():
	var was_open = budget_panel.visible
	close_all_panels()
	print("budget clicked")
	
	if not was_open:
		budget_panel.visible = true
		budget_btn.button_pressed = true
		# budget_panel.update_panel() # TODO

func _on_inventory_clicked():
	var was_open = inventory_panel.visible
	close_all_panels()
	print("inventory clicked")
	
	if not was_open:
		inventory_panel.visible = true
		inventory_btn.button_pressed = true
		# inventory_panel.update_panel()

func _on_market_clicked():
	var was_open = market_panel.visible
	close_all_panels()
	print("market clicked")
	
	if not was_open:
		market_panel.visible = true
		market_btn.button_pressed = true

func _on_manpower_clicked():
	var was_open = manpower_panel.visible
	close_all_panels()
	print("manpower clicked")
	
	if not was_open:
		manpower_panel.visible = true
		manpower_btn.button_pressed = true
	
func _on_perks_clicked():
	var was_open = perks_panel.visible
	close_all_panels()
	print("perks clicked")
	
	if not was_open:
		perks_panel.visible = true
		perks_btn.button_pressed = true
	
func _on_tech_clicked():
	var was_open = tech_panel.visible
	close_all_panels()
	print("tech clicked")
	
	if not was_open:
		tech_panel.visible = true
		tech_btn.button_pressed = true

func _on_city_selected_from_map(city_data: Dictionary) -> void:
	close_all_panels()
	
	# Kapatma işleminden sonra paneli aç ve veriyi gönder
	node_details_panel.visible = true
	node_details_panel.show_city_details(city_data)
