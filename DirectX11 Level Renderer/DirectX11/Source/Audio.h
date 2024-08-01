class Audio
{
	GW::AUDIO::GAudio audio;
	GW::AUDIO::GMusic music;
	GW::AUDIO::GSound sound;

	public:
		Audio()
		{
			audio.Create();
			std::cout << "GAudio proxy created\n";

			music.Create("../Audio Files/Piano Loop.wav", audio, 0.1f);
			std::cout << "GMusic created\n";

			sound.Create("../Audio Files/message-incoming-2-199577.wav", audio, 0.08f);
			std::cout << "GSound created\n";

			music.Play(true); // plays and 'true' loops the music
			std::cout << "Music playing\n";
		}
		void PlaySoundEffect()
		{
			sound.Play();
			std::cout << "Sound effect played\n";
		}
		~Audio()
		{
			music.Stop();
			audio.StopMusic();
		}
};