#pragma once

#include <MY_Scene_Main.h>
#include <RenderSurface.h>
#include <StandardFrameBuffer.h>
#include <RenderOptions.h>


#include <shader\ShaderComponentTexture.h>
#include <shader\ShaderComponentDiffuse.h>
#include <shader\ShaderComponentMVP.h>
#include <shader\ShaderComponentUvOffset.h>
#include <shader\ShaderComponentHsv.h>

#include <Easing.h>
#include <NumberUtils.h>
#include <PointLight.h>

#include <sweet\UI.h>

#include <MY_Game.h>
#include <GameOver.h>

MY_Scene_Main::MY_Scene_Main(Game * _game) :
	MY_Scene_Base(_game),
	screenSurfaceShader(new Shader("assets/RenderSurface_1", false, true)),
	screenSurface(new RenderSurface(screenSurfaceShader, true)),
	screenFBO(new StandardFrameBuffer(true)),
	turningAngle(0),
	speed(0),
	length(100),
	gap(3),
	health(3),
	distanceTravelled(0)
{
	// memory management
	screenSurface->incrementReferenceCount();
	screenSurfaceShader->incrementReferenceCount();
	screenFBO->incrementReferenceCount();
	
	screenSurface->setScaleMode(GL_NEAREST);
	screenSurface->uvEdgeMode = GL_CLAMP_TO_BORDER;

	// GAME

	road = new MeshEntity(MY_ResourceManager::globalAssets->getMesh("road")->meshes.at(0), baseShader);
	road->mesh->setScaleMode(GL_NEAREST);
	road->mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("road")->texture);
	
	childTransform->addChild(road)->scale(gap*0.75, 1, length)->translate(0, 0, length/2);


	Transform * treesL = new Transform();
	childTransform->addChild(treesL, false);
	Transform * treesR = new Transform();
	childTransform->addChild(treesR, false);

	int numTrees = 50;
	TriMesh * const treeMesh = MY_ResourceManager::globalAssets->getMesh("tree")->meshes.at(0);
	treeMesh->setScaleMode(GL_NEAREST);
	treeMesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("tree")->texture);
	for(unsigned long int i = 0; i < numTrees; ++i){
		MeshEntity * m = new MeshEntity(treeMesh, baseShader);
		treesL->addChild(m)->translate(0, 0, (float)i*length/numTrees)->scale(sweet::NumberUtils::randomFloat(0.25, 1.5));
		trees.push_back(m);
		
		m = new MeshEntity(treeMesh, baseShader);
		treesR->addChild(m)->translate(0, 0, (float)i*length/numTrees)->scale(sweet::NumberUtils::randomFloat(0.25, 1.5));
		trees.push_back(m);

	}

	TriMesh * const carMesh = MY_ResourceManager::globalAssets->getMesh("car")->meshes.at(0);
	carMesh->setScaleMode(GL_NEAREST);
	carMesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("tree")->texture);
	
	for(unsigned long int i = 0; i < 1; ++i){
		MeshEntity * obstacle = new MeshEntity(carMesh, baseShader);
		childTransform->addChild(obstacle)->translate(0, 0, length);
		obstacles.push_back(obstacle);
	}


	treesL->translate(-gap/2.f, 0, 0)->lookAt(glm::vec3(0,0,numTrees));
	treesR->translate(gap/2.f, 0, 0)->lookAt(glm::vec3(0,0,numTrees));


	// UI
	NodeUI * dash = new NodeUI(uiLayer->world);
	uiLayer->addChild(dash);
	dash->setRationalWidth(1.f, uiLayer);
	dash->setRationalHeight(1.f, uiLayer);
	dash->background->mesh->setScaleMode(GL_NEAREST);
	dash->background->mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("dash")->texture);

	wheel = new NodeUI(uiLayer->world);
	uiLayer->addChild(wheel);
	wheel->setRationalWidth(29/64.f, uiLayer);
	wheel->setRationalHeight(29/64.f, uiLayer);
	wheel->setMarginLeft(17/64.f);
	wheel->setMarginBottom(8/64.f);
	wheel->boxSizing = kCONTENT_BOX;
	wheel->background->mesh->setScaleMode(GL_NEAREST);
	wheel->background->mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("wheel")->texture);
	wheel->background->meshTransform->translate(glm::vec3(-0.5, -0.5, 0));

	
	speedUI = new NodeUI(uiLayer->world);
	uiLayer->addChild(speedUI);
	speedUI->setRationalHeight(1.f, uiLayer);
	speedUI->setRationalWidth(1.f, uiLayer);
	speedUI->background->mesh->setScaleMode(GL_NEAREST);
	updateSpeedUI();
	
	healthUI = new NodeUI(uiLayer->world);
	uiLayer->addChild(healthUI);
	healthUI->setRationalHeight(1.f, uiLayer);
	healthUI->setRationalWidth(1.f, uiLayer);
	healthUI->background->mesh->setScaleMode(GL_NEAREST);
	updateHealthUI();


	txt = new TextLabel(uiLayer->world, font, textShader);
	uiLayer->addChild(txt);
	txt->setRationalHeight(1.f, uiLayer);
	txt->setRationalWidth(1.f, uiLayer);
	txt->verticalAlignment = kMIDDLE;
	txt->setRenderMode(kTEXTURE);
	//txt->setMargin(2.f/64.f);
	//txt->setMarginBottom(32.f/64.f);

	waitTimeout = new Timeout(10.f, [this](sweet::Event * _event){
		txt->invalidate();
		txt->setText(getLine());
	});
	childTransform->addChild(waitTimeout, false);
	txt->setText("                Tutorial text or something.");

	NodeUI * dashMask = new NodeUI(uiLayer->world);
	uiLayer->addChild(dashMask);
	dashMask->setRationalWidth(1.f, uiLayer);
	dashMask->setRationalHeight(1.f, uiLayer);
	dashMask->background->mesh->setScaleMode(GL_NEAREST);
	dashMask->background->mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("dash-mask")->texture);

	sweet::setCursorMode(GLFW_CURSOR_DISABLED);


	gameCam = new PerspectiveCamera();
	gameCam->fieldOfView = 20;
	gameCam->yaw = -90;
	gameCam->pitch = -2.5;
	gameCam->interpolation = 1.f;
	gameCam->nearClip = 0.001f;
	gameCam->farClip = length;
	childTransform->addChild(gameCam)->translate(0, 0.25, 0);
	activeCamera = gameCam;
	cameras.push_back(gameCam);

	damageTimeout = new Timeout(0.5f, [this](sweet::Event * _event){
		gameCam->pitch = -2.5f;
	});
	damageTimeout->eventManager->addEventListener("progress", [this](sweet::Event * _event){
		float p = _event->getFloatData("progress");
		gameCam->pitch = Easing::easeOutElastic(p, -7.5, 5.f, 1.f);
	});
	childTransform->addChild(damageTimeout, false);

	PointLight * pl = new PointLight(glm::vec3(2), 0, 0.5f, -1);
	childTransform->addChild(pl)->translate(0,0.5,-0.5);
	lights.push_back(pl);
}

MY_Scene_Main::~MY_Scene_Main(){
	// memory management
	screenSurface->decrementAndDelete();
	screenSurfaceShader->decrementAndDelete();
	screenFBO->decrementAndDelete();
}


void MY_Scene_Main::update(Step * _step){
	// Screen shader update
	// Screen shaders are typically loaded from a file instead of built using components, so to update their uniforms
	// we need to use the OpenGL API calls
	screenSurfaceShader->bindShader(); // remember that we have to bind the shader before it can be updated
	GLint test = glGetUniformLocation(screenSurfaceShader->getProgramId(), "time");
	checkForGlError(0);
	if(test != -1){
		glUniform1f(test, _step->time);
		checkForGlError(0);
	}
	

	if(MY_Game::resized){
		font->resize(font->getSize());
		MY_Game::resized = false;
		txt->invalidate();
		txt->updateText();
		txt->invalidateLayout();
	}

	if(_step->cycles % 10 == 0){
		std::wstring s = txt->getText(false);
		if(s.length() > 0){
			std::wstring t = txt->getText(false);
			if(t.size() > 15 && t.at(15) != ' '){
				MY_ResourceManager::globalAssets->getAudio("boop")->sound->play();
			}
			txt->setText(t.substr(1));
		}else if(waitTimeout->complete || !waitTimeout->active){
			waitTimeout->targetSeconds = sweet::NumberUtils::randomFloat(5.f, 10.f);
			waitTimeout->restart();
		}
	}


	if(keyboard->keyJustDown(GLFW_KEY_L)){
		screenSurfaceShader->unload();
		screenSurfaceShader->loadFromFile(screenSurfaceShader->vertSource, screenSurfaceShader->fragSource);
		screenSurfaceShader->load();
	}

	// move obstacles
	
	for(auto o : obstacles){
		if(o->firstParent()->getTranslationVector().z < length){
			o->firstParent()->translate(0, 0, 0.25f);
		}
	}

	// player controls
	float turningAngleNew = Easing::linear((float)mouse->mouseX(false), 0.5, -1.f, sweet::getWindowWidth());
	float d = (turningAngleNew - turningAngle)*0.05f;
	if(glm::abs(d) > FLT_EPSILON){
		turningAngle += d;
	}
	wheel->background->childTransform->setOrientation(glm::angleAxis(turningAngle*60.f, glm::vec3(0,0,1)));
	
	gameCam->yaw = -90 + turningAngle;

	if(mouse->leftDown()){
		speed += 0.004;
	}else{
		speed -= 0.0035;
	}
	speed *= 0.99f;
	if(speed > FLT_EPSILON){
		for(auto t : trees){
			// move forward
			t->firstParent()->translate(0, 0, -speed);

			// cycle
			if(t->firstParent()->getTranslationVector().z < 0){
				t->firstParent()->translate(0, 0, length);
			}
		}
		for(auto o : obstacles){
			// move forward
			o->firstParent()->translate(0, 0, -speed);
			
			glm::vec3 v = o->firstParent()->getTranslationVector();
			if(v.z < 0){
				// check collision
				if(glm::abs(v.x - gameCam->firstParent()->getTranslationVector().x) < 0.3f){
					damage();
				}

				// move back
				o->firstParent()->translate((sweet::NumberUtils::randomBool() ? -1 : 1) * gap*0.125f, 0, sweet::NumberUtils::randomFloat(length, length*2), false);
			}
		}

		for(auto & v : road->mesh->vertices){
			v.v += speed*0.01;
		}
		road->mesh->dirty = true;

		gameCam->firstParent()->translate(turningAngle*speed*0.025f, 0, 0);
		glm::vec3 v = gameCam->firstParent()->getTranslationVector();

		if(v.x < -gap/2 * 0.75f){
			damage();
			v.x = -gap/2 * 0.7f;
			turningAngle = 5;
		}if(v.x > gap/2 * 0.75f){
			damage();
			v.x = gap/2 * 0.7f;
			turningAngle = -5;
		}
		gameCam->firstParent()->translate(v, false);
	}else{
		speed = 0;
	}
	distanceTravelled += speed;

	updateSpeedUI();
	

	if(health < 0){
		game->scenes["gameover"] = new GameOver(game, distanceTravelled);
		game->switchScene("gameover", true);
	}

	// Scene update
	uiLayer->resize(0, 64, 0, 64);
	MY_Scene_Base::update(_step);
}

void MY_Scene_Main::render(sweet::MatrixStack * _matrixStack, RenderOptions * _renderOptions){
	glm::uvec2 sd = sweet::getWindowDimensions();
	int max = glm::max(sd.x, sd.y);
	int min = glm::min(sd.x, sd.y);
	bool horz = sd.x == max;
	int offset = (max - min)/2;

	// keep our screen framebuffer up-to-date with the current viewport
	screenFBO->resize(64, 64);
	_renderOptions->setViewPort(0,0,64,64);
	_renderOptions->setClearColour(0,0,0,0);

	// bind our screen framebuffer
	FrameBufferInterface::pushFbo(screenFBO);
	// render the scene
	MY_Scene_Base::render(_matrixStack, _renderOptions);
	// unbind our screen framebuffer, rebinding the previously bound framebuffer
	// since we didn't have one bound before, this will be the default framebuffer (i.e. the one visible to the player)
	FrameBufferInterface::popFbo();

	// render our screen framebuffer using the standard render surface
	_renderOptions->setViewPort(horz ? offset : 0, horz ? 0 : offset, min, min);
	screenSurface->render(screenFBO->getTextureId());

	//uiLayer->render(_matrixStack, _renderOptions);
}

void MY_Scene_Main::load(){
	MY_Scene_Base::load();	

	screenSurface->load();
	screenFBO->load();
}

void MY_Scene_Main::unload(){
	screenFBO->unload();
	screenSurface->unload();

	MY_Scene_Base::unload();	
}


std::wstring MY_Scene_Main::getLine(){
	return lines.getPhrase();
}

void MY_Scene_Main::damage(){
	--health;
	MY_ResourceManager::globalAssets->getAudio("bam")->sound->play();
	damageTimeout->restart();
	updateHealthUI();
}

void MY_Scene_Main::updateHealthUI(){
	healthUI->background->mesh->replaceTextures(MY_ResourceManager::globalAssets->getTexture("health_" + std::to_string(health))->texture);
}

void MY_Scene_Main::updateSpeedUI(){
	static int curSpeed = -1;
	int s = (int)glm::clamp(speed * 4.f / 0.3f, 0.f, 4.f);
	if(s != curSpeed){
		curSpeed = s;
		speedUI->background->mesh->replaceTextures(MY_ResourceManager::globalAssets->getTexture("speed_" + std::to_string(curSpeed))->texture);
	}
}