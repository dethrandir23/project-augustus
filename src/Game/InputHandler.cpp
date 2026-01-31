#include "../../lib/nlohmann/json.hpp"
#include "InputHandler.h"
#include "GameManager.h"
#include "Gamestate.h"
#include "Factory.h"
#include "../Economy/Templates.h"

    bool InputHandler::handleInput(Gamestate &gamestate, const nlohmann::json &input) {
        if (!input.contains("type")) return false;
        if (!input.contains("payload")) return false;

        InputType inputType = stringToInputType(input.at("type").get<std::string>());
        switch (inputType) {
            case InputType::STEP_GAME:
                return handleStepGame(gamestate, input.at("payload"));
            case InputType::RESET_GAME:
                return handleResetGame(gamestate, input.at("payload"));
            case InputType::BUILD_FACTORY:
                return handleBuildFactory(gamestate, input.at("payload"));
            case InputType::RENAME_FACTORY:
                return handleRenameFactory(gamestate, input.at("payload"));
            case InputType::SCRAP_FACTORY:
                return handleScrapFactory(gamestate, input.at("payload"));
            case InputType::CHANGE_WORKFORCE:
                return handleChangeWorkforce(gamestate, input.at("payload"));
            case InputType::MARKET_BUY_ITEM:
                return handleMarketBuyItem(gamestate, input.at("payload"));
            case InputType::MARKET_SELL_ITEM:
                return handleMarketSellItem(gamestate, input.at("payload"));
            case InputType::ADD_MONEY:
                return handleAddMoney(gamestate, input.at("payload"));
            case InputType::ADD_ITEM:
                return handleAddItem(gamestate, input.at("payload"));
            default:
                return false;
        }
    }

    bool handleStepGame(Gamestate &gamestate, const nlohmann::json &payload) {
        size_t times = payload.at("times").get<size_t>();
        for (size_t i = 0; i < times; ++i) {
            GameManager::stepGamestate(gamestate);
        }
        return true;
    }

    bool handleBuildFactory(Gamestate &gamestate, const nlohmann::json &payload) {
        try {
        std::string tmplName = payload.at("templateName").get<std::string>();
        
        // Şablonu seç (Burası biraz if-else mecburen, ya da map yapabilirsin)
        FactoryTemplate tmpl;
        if (tmplName == "SAWMILL") tmpl = DefaultFactoryTemplates::SAWMILL;
        else if (tmplName == "BRICKWORKS") tmpl = DefaultFactoryTemplates::BRICKWORKS;
        else return false; // Bilinmeyen şablon

        // Oyuncuyu bul
        uuids::uuid playerId = gamestate.getPlayerCompanyId();
        Company* player = gamestate.getCompany(playerId);
        
        // ID Oluştur
        uuids::uuid factoryId = IdUtils::generateUuid();
        
        // Fabrikayı Yarat
        std::string name = payload.value("customName", tmplName);
        Factory newFactory(name, factoryId, playerId);
        
        // Pipeline'ları ekle
        for(const auto& pipe : tmpl.pipelines) {
            newFactory.addPipeline(pipe);
        }

        // Gamestate'e kaydet
        gamestate.addFactory(factoryId, newFactory);
        player->addFactory(factoryId);
        
        //std::cout << "Factory Created: " << name << std::endl;
        return true;

    } catch (const std::exception& e) {
        //std::cerr << "Error creating factory: " << e.what() << std::endl;
        return false;
    }
    }
    bool handleRenameFactory(Gamestate &gamestate, const nlohmann::json &payload) {
        try {
            
            uuids::uuid factoryId = uuids::from_string(payload.at("factoryId").get<std::string>());
            std::string newName = payload.at("newName").get<std::string>();
            Factory* factory = gamestate.getFactory(factoryId);
            if (factory) {
                factory->setName(newName);
                return true;
            } else {
                return false;
            }
        } catch (const std::exception& e) {
            //std::cerr << "Error renaming factory: " << e.what() << std::endl;
            return false;
        }
    }

    // not: templatelere scrap return itemlerini de ekle
    bool handleScrapFactory(Gamestate &gamestate, const nlohmann::json &payload);

    bool handleChangeWorkforce(Gamestate &gamestate, const nlohmann::json &payload);

    bool handleMarketBuyItem(Gamestate &gamestate, const nlohmann::json &payload);
    bool handleMarketSellItem(Gamestate &gamestate, const nlohmann::json &payload);

    bool handleAddMoney(Gamestate &gamestate, const nlohmann::json &payload) {
        try {
            uuids::uuid playerId = gamestate.getPlayerCompanyId();
            Company* player = gamestate.getCompany(playerId);
            if (!player) return false;
            size_t amount = payload.at("amount").get<size_t>();
            player->setCapital(player->getCapital() + amount);
            return true;
        } catch (const std::exception& e) {
            //std::cerr << "Error adding money: " << e.what() << std::endl;
            return false;
        }
    }
    bool handleAddItem(Gamestate &gamestate, const nlohmann::json &payload);
    