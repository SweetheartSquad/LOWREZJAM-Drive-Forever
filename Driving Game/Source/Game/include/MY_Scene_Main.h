#pragma once

#include <MY_Scene_Base.h>

class RenderSurface;
class StandardFrameBuffer;

class MY_Scene_Main : public MY_Scene_Base{
public:
	Shader * screenSurfaceShader;
	RenderSurface * screenSurface;
	StandardFrameBuffer * screenFBO;


	PerspectiveCamera * gameCam;
	float speed;

	const float length, const gap;


	NodeUI * wheel;
	float turningAngle;
	
	std::vector<MeshEntity *> trees;


	virtual void update(Step * _step) override;
	virtual void render(sweet::MatrixStack * _matrixStack, RenderOptions * _renderOptions) override;

	virtual void unload() override;
	virtual void load() override;

	MY_Scene_Main(Game * _game);
	~MY_Scene_Main();
};