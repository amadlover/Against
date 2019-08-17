#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

	typedef enum _SoundType
	{
		IntroMusic
	} SoundType;

	void PlaySound (SoundType Sound);

#ifdef __cplusplus
}
#endif // __cplusplus
