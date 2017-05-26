#include "scoreDisplay.hpp"

ScoreDisplay::ScoreDisplay(SDL_Window* window) {
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 0);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Surface* temp = IMG_Load("/../textures/red_ball.png");
    texture = SDL_CreateTextureFromSurface(renderer, temp);
    SDL_FreeSurface(temp);

    rect.x = 0;
	rect.y = 0;
	rect.w = 100;
	rect.h = 100;
	//'rect' defines the dimensions for the bottom-left of the window

}

void ScoreDisplay::draw(unsigned int score) {
	//SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    //SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, &rect);
	SDL_RenderPresent(renderer);
}

void ScoreDisplay::release() {
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
}