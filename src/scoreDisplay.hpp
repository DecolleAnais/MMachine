#ifndef _MMACHINE_SCORE_DISPLAY_HPP_
#define _MMACHINE_SCORE_DISPLAY_HPP_

#include "mesh.h"
#include "texture.h"

class ScoreDisplay {
  public :
  	ScoreDisplay(const unsigned int max_score);
  	void draw(const unsigned int score);
    void release();

private:
	std::vector<Mesh> meshs_;
    std::vector<GLuint> textures_;
} ;

#endif
