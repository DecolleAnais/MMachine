#ifndef _MMACHINE_SCORE_MANAGER_HPP_
#define _MMACHINE_SCORE_MANAGER_HPP_

#include "terrain.hpp"

#include "mesh.h"
#include "texture.h"
#include "text.h"

class ScoreManager {
  public :
  	ScoreManager(const unsigned int max_score, unsigned int score_player_1, std::vector<Checkpoint> checkpoints);
  	int getFirst(Point player_1_position, Point player_2_position);
  	void updateCheckpoints(Point player_1_position, Point player_2_position);
  	void updateScore(int first);
  	int getRoundWinner();
  	void resetRound();
  	bool end();
  	void draw();
  	void drawCongratulations();
    void release();

private:
	// display
	std::vector<Mesh> meshs_;
    std::vector<GLuint> textures_;
    // score storage
    unsigned int score_player_1_;
    unsigned int max_score_;
    // checkpoints to pass
    std::vector<Checkpoint> checkpoints_;
    int checkpoint_player_1_;
    int checkpoint_player_2_;
    // winner
    int round_winner_;
    // texte de score affiché
    Text text;
} ;

#endif
