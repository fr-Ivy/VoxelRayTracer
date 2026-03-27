#include "template.h"
#include "Audio.h"

Audio::Audio()
{
	if (ma_engine_init(nullptr, &engine) != MA_SUCCESS)
	{
		printf("Engine pointer: %p\n", static_cast<void*>(&engine));
		printf("Engine init failed.\n");
		return;
	}
	else
	{
		printf("Engine init succeeded\n");
	}

	if (ma_sound_init_from_file(&engine, "assets/Sounds/Alegend_In_Flight(freetouse.com).mp3", 0, NULL, NULL, &music) != MA_SUCCESS)
	{
		printf("Failed to load music");
	}

}

Audio::~Audio()
{
}

void Audio::Play()
{
	ma_result theResult = ma_sound_start(&music);
	if (theResult != MA_SUCCESS)
	{
		printf("Failed to play music");
	}
}
