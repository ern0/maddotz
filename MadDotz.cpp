# include "MadDotz.hpp"


# ifdef __EMSCRIPTEN__
	
	
	int getWindowWidth() {
		return EM_ASM_INT({ return window.innerWidth; },MadDotz::windowWidth);
	} // getWindowWidth()	


	int getWindowHeight() {
		return EM_ASM_INT({ return window.innerHeight; },MadDotz::windowHeight);												
	} // getWindowHeight()


	void hideHtmlWrapper() {
		EM_ASM(
			document.getElementById("controls").style.display = "none";
			document.getElementById("Layer_1").parentNode.style.display = "none";
			document.getElementsByClassName("emscripten")[0].style.display = "none";
			document.getElementById("output").style.display = "none";
			document.getElementById("canvas").parentNode.style.border = "none";					
		);
	} // hideHtmlWrapper()

# endif


	void die(const char* msg) {
		# ifdef __EMSCRIPTEN__
			EM_ASM(
				document.getElementById("output").style.display = "block";
			);
		# endif
		printf("ERROR: %s (SDL error: %s) \n",msg,SDL_GetError());
		exit(1);
	} // die()


	void quit() {
		SDL_Quit();
		exit(0);
	} // quit()	
	
	
	int seed = 1871;
	# define SEEDMAX (329483)
	int rnd() {
		seed += 991021;
		if (seed > SEEDMAX) seed -= SEEDMAX;
		return seed >> 4;
	} // rnd()

		

	SDL_Window* MadDotz::mainWindow;
	SDL_Renderer* MadDotz::renderer;
	SDL_Texture* MadDotz::dotTexture;
				
	SDL_Event MadDotz::event;
		
	int MadDotz::windowWidth;
	int MadDotz::windowHeight;
	int MadDotz::windowWidthForDot;
	int MadDotz::windowHeightForDot;

	SDL_Rect MadDotz::srcRect;
	SDL_Rect MadDotz::dstRect;
	Dot MadDotz::dots[DOTZ];


	# ifndef __EMSCRIPTEN__		
	void MadDotz::about() {
	
		printf(
			"MadDotz - a tiny particle demo - 2015.06.14 \n"
			"Idea by Attila Kosir (TheMad), he wrote a similar app cca. 25-30 years ago. \n"
			" Each dot of %d total has a relationship to %d of other ones: +/-100%%. \n"
			" When the relation factor is love (0..100%%), the dot will follow the other \n"
			" one with the speed of the factor. When the relation factor is hate (0..-100%%) \n"
			" the dot will run away as fast as the factor is. The dot's position in the next \n"
			" frame is changed by the sum of these relations. When a dot stops (e.g. lovers \n"
			" stick together) or hit the wall (e.g. wants to escape from the world), minor \n"
			" change applies to its feelings in order to keep it in the arena. \n"						
			"Check for web-based (Emscripten) version at http://linkbroker.hu/ \n"
		,DOTZ,RELS);
		
	} // about()
	# endif

	
	void MadDotz::init() {
		
		# ifdef __EMSCRIPTEN__
			hideHtmlWrapper();
			windowWidth = getWindowWidth();
			windowHeight = getWindowHeight();
		# else
			windowWidth = W;
			windowHeight = H;
		# endif
		
		windowWidthForDot = 256 * (windowWidth - FIGW);
		windowHeightForDot = 256 * (windowHeight - FIGH);

		if ( SDL_Init(SDL_INIT_VIDEO) == -1 ) die("sdl init failed");

		mainWindow = SDL_CreateWindow(
			"MadDotz",
			# ifdef DUMPMODE
				1000,
				50,
			# else
				SDL_WINDOWPOS_CENTERED,
				SDL_WINDOWPOS_CENTERED,
			# endif
			windowWidth,
			windowHeight,
			SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
		);
		if (mainWindow == NULL) die("window creation failed");

		renderer = SDL_CreateRenderer(mainWindow,-1,SDL_RENDERER_ACCELERATED);			
		if (renderer == NULL) {
			renderer = SDL_CreateRenderer(mainWindow,-1,SDL_RENDERER_SOFTWARE);			
			if (renderer == NULL) die("renderer init failed");			
		}
		SDL_SetRenderDrawColor(renderer,0xFF,0xFF,0xFF,0xFF);

		/// // these lines occurs error
		/// int imgFlags = IMG_INIT_PNG; 
		/// if (!(IMG_Init(imgFlags) & imgFlags)) die("png support init failed");
		
		SDL_Surface* dotLoadSurface = IMG_Load("res/dotz24.png");
		if (dotLoadSurface == NULL) dotLoadSurface = IMG_Load("/usr/share/MadDotz/dotz24.png");
		if (dotLoadSurface == NULL) die("image load failed"); 			
		dotTexture = SDL_CreateTextureFromSurface(renderer,dotLoadSurface);
		if (dotTexture == NULL) die("texture creation failed");
		SDL_FreeSurface(dotLoadSurface);	
		
		for (int n = 0; n < DOTZ; n++) dots[n].init(n);
		
		srcRect.w = FIGW;
		srcRect.h = FIGH;
		srcRect.x = 0;
		srcRect.y = 0;
		dstRect.w = FIGW;
		dstRect.h = FIGH; 			
		
	} // init()		
				
		
	void MadDotz::loop() {
	
		while (SDL_PollEvent(&event) != 0) {			
			eventHandler();
		} // while event

		# ifndef STEPMODE
			renderFrame();
		# endif
		
	} // loop()		
	
		
	void MadDotz::eventHandler() {
	
		switch (event.type) {
		
			case SDL_QUIT: {
				quit();
			} break;
			
			case SDL_KEYDOWN: {	

				switch (event.key.keysym.sym) {
					case 0x1B: {
						quit();
					} break;
					default: {
						# ifdef STEPMODE
							renderFrame();
						# endif
					} break;
				} // switch key
				
			} break;
		
		} // switch event type

	} // eventHandler()
	

	void MadDotz::renderFrame() {

		# ifdef DUMPMODE
			printf("\e[0;0H\e[2J");
		# endif
		for (int n = 0; n < DOTZ; n++) dots[n].proc();
		SDL_RenderClear(renderer);		
		for (int n = 0; n < DOTZ; n++) dots[n].paint();
		SDL_RenderPresent(renderer);
		
	} // renderFrame()			
			
			
	void Dot::init(int no) {

		numero = no;
		initFigure();
		initPosition();
		initRelations();
		initSpeed();
		life = 0;
		
	} // init()
	
	
	void Dot::respawn() {

		life++;

		int r = rnd();
		if (r % 1000 < 100) {
			initRelations();
		} else if (r % 1000 < 700) {
			for (int n = 0; n < RELS; n++) smoothRelation(n);
		} else {
			int n = rnd() % RELS;
			smoothRelation(n);
		}
		
		if (rnd() % 100 < 20) {
			initSpeed();
		} else {
			if (speed > 1) --speed;
		}
					
	} // respawn()
	
	
	void Dot::initFigure() {
		fig = rnd() % FIGS;		
	} // initFigure()
	
	
	void Dot::initPosition() {
	
		x = rnd() % MadDotz::windowWidthForDot;
		y = rnd() % MadDotz::windowHeightForDot;
		
		# ifdef DUMPMODE
			if (numero == 0) {
				x = 256 * 20;
				y = 256 * 20;
			}
		# endif
	
	} // initPosition()
	

	void Dot::initRelation(int n) {
		relation[n] = rnd() % 512 - 256;
	} // initRelation()
	
	
	void Dot::initRelations() {
		for (int n = 0; n < RELS; n++) initRelation(n);		
	} // initRelations()
	
	
	void Dot::smoothRelation(int n) {
		relation[n] = -relation[n] / 2;
	} // smoothRelation()
	
	
	void Dot::initSpeed() {	
		speed = 1 + (rnd() % 3);		
	} // initSpeed()
	
	
	void Dot::proc() {
	
		# ifdef DUMPMODE
			printf("id=%d life=%d x=%d y=%d speed=%d\n",numero,life,x / 256,y / 256,speed);
		# endif
		
		int sumMoveX = 0;
		int sumMoveY = 0;
	
		int index = 1 + numero;
		if (index == DOTZ) index = 0;
		for (int n = 0; n < RELS; n++) {			
			Dot& rel = MadDotz::dots[index++];
			int love = relation[n];

			int moveX = 0;
			int moveY = 0;
			
			int distX = rel.x - x;
			int distY = rel.y - y;
			
			# define CORRECTION (DOTZ / RELS);
			moveX = distX * love * speed / 32768 * CORRECTION;
			moveY = distY * love * speed / 32768 * CORRECTION;

			sumMoveX += moveX;
			sumMoveY += moveY;

			# ifdef DUMPMODE
				printf("  %d:",n);				
				printf(" love=%d",love);
				printf(" distX=%d",distX / 256);
				printf(" moveX=%d",moveX / 256);				
				printf(" distY=%d",distY / 256);
				printf(" moveY=%d",moveY / 256);
				printf("\n");
			# endif
		
		} // foreach dot
		
		# define SPEEDLIMIT (256 * 5)
		if (sumMoveX < -SPEEDLIMIT) sumMoveX = -SPEEDLIMIT;
		if (sumMoveX > SPEEDLIMIT) sumMoveX = SPEEDLIMIT;
		if (sumMoveY < -SPEEDLIMIT) sumMoveY = -SPEEDLIMIT;
		if (sumMoveY > SPEEDLIMIT) sumMoveY = SPEEDLIMIT;
		
		nextX = x + sumMoveX;
		nextY = y + sumMoveY;
		
		int vitality = ( sumMoveX > 0 ? sumMoveX : -sumMoveX );
		vitality += ( sumMoveY > 0 ? sumMoveY : -sumMoveY );
		if (vitality < 50) respawn();

		# ifdef DUMPMODE
			printf("  *vitality=%d sumMoveX=%d sumMoveY=%d nextX=%d nextY=%d \n",vitality,sumMoveX / 256,sumMoveY / 256,nextX / 256,nextY / 256);
		# endif

	} // proc()
			

	void Dot::paint() {		

		x = nextX;
		y = nextY;
		
		bool hit = false;

		if (x < 0) {
			x = 0;
			hit = true;
		}
		if (x > MadDotz::windowWidthForDot) {
			x = MadDotz::windowWidthForDot;	
			hit = true;
		}
		if (y < 0) {
			y = 0;
			hit = true;
		}
		if (y > MadDotz::windowHeightForDot) {
			y = MadDotz::windowHeightForDot;
			hit = true;
		}
		
		if (hit) respawn();
				
		MadDotz::srcRect.x = fig * FIGW;
		MadDotz::dstRect.x = x / 256;
		MadDotz::dstRect.y = y / 256;
		
		SDL_RenderCopy(
			MadDotz::renderer,
			MadDotz::dotTexture,
			&MadDotz::srcRect,
			&MadDotz::dstRect
		);
		
	} // paint()
	
	
	int main() { 

		MadDotz::init();
	
		# ifdef __EMSCRIPTEN__
			emscripten_set_main_loop(MadDotz::loop,0,1);
		# else
			# ifdef STEPMODE
				MadDotz::renderFrame();
			# endif
			MadDotz::about();
			while (true) {
				MadDotz::loop();
				SDL_Delay(20);
			} // forever
		# endif
	
	} // main()
