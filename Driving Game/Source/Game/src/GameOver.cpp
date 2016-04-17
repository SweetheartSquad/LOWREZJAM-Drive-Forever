#pragma once

#include <GameOver.h>
#include <shader/ComponentShaderText.h>
#include <sweet/UI.h>

#include <MY_Scene_Menu.h>

GameOver::GameOver(Game * _game, float _score) :
	MY_Scene_Base(_game),
	done(false)
{
	{
	VerticalLinearLayout * vl = new VerticalLinearLayout(uiLayer->world);
	uiLayer->addChild(vl);
	vl->setRationalHeight(1.f, uiLayer);
	vl->setRationalWidth(1.f, uiLayer);
	vl->horizontalAlignment = kCENTER;
	vl->verticalAlignment = kMIDDLE;
	NodeUI * menu = new NodeUI(uiLayer->world);
	vl->addChild(menu);
	menu->setRationalHeight(1.f, vl);
	menu->setSquareWidth(1.f);
	menu->background->mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("ENDING")->texture);
	menu->background->mesh->setScaleMode(GL_NEAREST);
	}
	TextArea * txt = new TextArea(uiLayer->world, font, textShader);
	uiLayer->addChild(txt);
	txt->setRationalHeight(32.f/64.f, uiLayer);
	txt->setRationalWidth(1.f, uiLayer);
	txt->verticalAlignment = kMIDDLE;
	txt->horizontalAlignment = kCENTER;
	txt->setRenderMode(kTEXTURE);
	txt->setWrapMode(kWORD);

	txt->setText("Drove " + std::to_string((int)glm::floor(_score*0.01f)) + " km");

	Timeout * doneTimeout = new Timeout(0.5f, [this](sweet::Event * _event){
		done = true;
		NodeUI * n = new NodeUI(uiLayer->world);
		n->setRationalHeight(1.f, uiLayer);
		n->setRationalWidth(1.f, uiLayer);
		uiLayer->addChild(n);
		n->background->mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("CLICK")->texture);
		n->background->mesh->setScaleMode(GL_NEAREST);
	});
	doneTimeout->start();
	doneTimeout->name = "done timeout";
	childTransform->addChild(doneTimeout, false);
	sweet::setCursorMode(GLFW_CURSOR_NORMAL);
}

void GameOver::update(Step * _step){
	if(done){
		if(mouse->leftJustPressed()){
			dynamic_cast<MY_Scene_Menu *>(game->scenes["menu"])->readyTimeout->restart();
			dynamic_cast<MY_Scene_Menu *>(game->scenes["menu"])->ready = false;
			game->switchScene("menu", true);
		}
	}

	MY_Scene_Base::update(_step);
}