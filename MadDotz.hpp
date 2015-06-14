# ifndef _MadDotz_hpp
# define _MadDotz_hpp

# ifdef __EMSCRIPTEN__
# include <emscripten.h>
# include <html5.h>
# endif

# include <SDL2/SDL.h>
# include <SDL2/SDL_image.h>
# include <stdlib.h>
# include <stdio.h>


# define FIGS (24)
# define FIGW (15)
# define FIGH (15)

# if 1
	# define DOTZ (32)
	# define RELS (4)
	# define W (800)
	# define H (480)

# else
	# define DOTZ (4)
	# define RELS (2)
	# define W 800
	# define H 600
	//# define STEPMODE
	//# define DUMPMODE
# endif


class Dot;

class MadDotz {

	protected:
				
		static SDL_Event event;

	public:

		static SDL_Window* mainWindow;
		static SDL_Renderer* renderer;
		static SDL_Texture* dotTexture;
		static SDL_Rect srcRect;
		static SDL_Rect dstRect;

		static Dot dots[DOTZ];
	
		static int windowWidth;
		static int windowHeight;
		static int dotWidth;
		static int dotHeight;
		static int windowWidthForDot;
		static int windowHeightForDot;
		
	public:
	
		# ifndef __EMSCRIPTEN__		
		static void about();
		# endif
		static void init();
		static void loop();
		static void eventHandler();
		static void renderFrame();
	
}; // class MadDotz


class Dot {

	public:
		int numero;
		int fig;
		int life;
		int x;
		int y;
		int nextX;
		int nextY;

		signed char relation[RELS];
		int speed;
		
	public:
		void init(int no);
		void respawn();
		void initFigure();
		void initPosition();
		void initRelation(int n);
		void initRelations();
		void smoothRelation(int n);
		void initSpeed();
		void proc();
		void paint();

}; // class Dot

# endif
