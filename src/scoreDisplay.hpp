#ifndef _MMACHINE_SCORE_DISPLAY_HPP_
#define _MMACHINE_SCORE_DISPLAY_HPP_

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

class ScoreDisplay {
  public :
  	ScoreDisplay(SDL_Window* window);
  	void draw(unsigned int score);
    void release();

private:
	SDL_Renderer* renderer = NULL;
	SDL_Texture* texture = NULL;
	SDL_Rect rect;
} ;

#endif
