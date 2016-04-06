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

#include <MY_Game.h>

MY_Scene_Main::MY_Scene_Main(Game * _game) :
	MY_Scene_Base(_game),
	screenSurfaceShader(new Shader("assets/RenderSurface_1", false, true)),
	screenSurface(new RenderSurface(screenSurfaceShader, true)),
	screenFBO(new StandardFrameBuffer(true)),
	turningAngle(0)
{
	// memory management
	screenSurface->incrementReferenceCount();
	screenSurfaceShader->incrementReferenceCount();
	screenFBO->incrementReferenceCount();
	
	screenSurface->setScaleMode(GL_NEAREST);
	screenSurface->uvEdgeMode = GL_CLAMP_TO_BORDER;

	// GAME
	
	Transform * treesL = new Transform();
	childTransform->addChild(treesL, false);
	Transform * treesR = new Transform();
	childTransform->addChild(treesR, false);

	int numTrees = 100;
	float gap = 4;
	TriMesh * const treeMesh = MY_ResourceManager::globalAssets->getMesh("tree")->meshes.at(0);
	treeMesh->setScaleMode(GL_NEAREST);
	treeMesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("tree")->texture);
	for(unsigned long int i = 0; i < numTrees; ++i){
		MeshEntity * m = new MeshEntity(treeMesh, baseShader);
		treesL->addChild(m)->translate(0, 0, i)->scale(sweet::NumberUtils::randomFloat(0.5, 1.5));
		
		m = new MeshEntity(treeMesh, baseShader);
		treesR->addChild(m)->translate(0, 0, i)->scale(sweet::NumberUtils::randomFloat(0.5, 1.5));
	}
	
	treesL->translate(-gap/2.f, 0, 0)->lookAt(glm::vec3(0,0,numTrees));
	treesR->translate(gap/2.f, 0, 0)->lookAt(glm::vec3(0,0,numTrees));



	PointLight * pl = new PointLight(glm::vec3(1), 0, 0.05f, -1);
	activeCamera->childTransform->addChild(pl,false);
	lights.push_back(pl);


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


	NodeUI * dashMask = new NodeUI(uiLayer->world);
	uiLayer->addChild(dashMask);
	dashMask->setRationalWidth(1.f, uiLayer);
	dashMask->setRationalHeight(1.f, uiLayer);
	dashMask->background->mesh->setScaleMode(GL_NEAREST);
	dashMask->background->mesh->pushTexture2D(MY_ResourceManager::globalAssets->getTexture("dash-mask")->texture);

	sweet::setCursorMode(GLFW_CURSOR_DISABLED);
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


	if(keyboard->keyJustDown(GLFW_KEY_L)){
		screenSurfaceShader->unload();
		screenSurfaceShader->loadFromFile(screenSurfaceShader->vertSource, screenSurfaceShader->fragSource);
		screenSurfaceShader->load();
	}

	// terrain
	//treeWheelL->rotate(1, 0, 1, 0, kOBJECT);
	//treeWheelR->rotate(-1, 0, 1, 0, kOBJECT);

	// player controls
	float turningAngleNew = 30.f - Easing::linear((float)mouse->mouseX(false), 0, 60, sweet::getWindowWidth());
	turningAngle += (turningAngleNew - turningAngle)*0.1f;
	wheel->background->childTransform->setOrientation(glm::angleAxis(turningAngle, glm::vec3(0,0,1)));
	
	

	if(mouse->leftDown()){
		activeCamera->firstParent()->translate(activeCamera->forwardVectorRotated * 0.1f);
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