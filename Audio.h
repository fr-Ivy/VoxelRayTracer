#pragma once
#include "miniaudio.h"

class Audio
{
public:
	Audio();
	~Audio();
	void Play();

	ma_sound music;
private:
	ma_engine engine;
};

