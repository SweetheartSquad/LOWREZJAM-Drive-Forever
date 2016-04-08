#pragma once

#include <PhraseGenerator.h>

class Phrases : public PhraseGenerator{
public:
	Phrases();
	std::wstring getPhrase();
};