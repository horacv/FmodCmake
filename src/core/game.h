#ifndef GAME_HPP
#define GAME_HPP

class Game
{
	public:
		Game();

		void Initialize();
		void Run();
		void Terminate() const;
		[[nodiscard]] bool IsRunning() const;

	private:
		bool mIsRunning;

		float mWindowWidth;
		float mWindowHeight;

		void Update();
		void ProcessInput();
		void Start();
		void Render() const;
};
#endif
