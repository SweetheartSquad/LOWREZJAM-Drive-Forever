#pragma once

#include <MY_Scene_Base.h>
#include <Phrases.h>

class RenderSurface;
class StandardFrameBuffer;
class TextLabel;

class MY_Scene_Main : public MY_Scene_Base{
public:
	Shader * screenSurfaceShader;
	RenderSurface * screenSurface;
	StandardFrameBuffer * screenFBO;


	PerspectiveCamera * gameCam;
	float speed;
	float distanceTravelled;

	const float length, const gap;


	NodeUI * wheel, * healthUI, * speedUI;
	float turningAngle;
	int health;
	
	std::vector<MeshEntity *> trees;
	MeshEntity * road;
	std::vector<MeshEntity *> obstacles;

	Timeout * waitTimeout, * damageTimeout;
	TextLabel * txt;
	Phrases lines;


	virtual void update(Step * _step) override;
	virtual void render(sweet::MatrixStack * _matrixStack, RenderOptions * _renderOptions) override;

	virtual void unload() override;
	virtual void load() override;

	MY_Scene_Main(Game * _game);
	~MY_Scene_Main();

	std::wstring getLine();

	void damage();
	void updateHealthUI();
	void updateSpeedUI();
};