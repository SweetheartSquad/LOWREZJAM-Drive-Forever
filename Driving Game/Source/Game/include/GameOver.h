#pragma once

#include <MY_Scene_Base.h>

class GameOver : public MY_Scene_Base{
public:
	bool done;

	GameOver(Game * _game, float _score);
	virtual void update(Step * _step) override;
};