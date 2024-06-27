class Audio
{
	GW::AUDIO::GAudio audio;
	GW::AUDIO::GMusic music;
	GW::AUDIO::GSound sound;

	public:
		Audio()
		{
			audio.Create();
			/*music.Create();
			sound.Create();*/
		}
};