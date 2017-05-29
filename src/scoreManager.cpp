#include "scoreManager.hpp"

#include "draw.h"
#include "window.h"

ScoreManager::ScoreManager(const unsigned int max_score, unsigned int score_player_1, std::vector<Checkpoint> checkpoints) {
	// initialisation du score
	max_score_ = max_score;
	score_player_1_ = score_player_1;
	round_winner_ = -1;
	text = create_text();

	// initialisation de la liste de checkpoints
	checkpoints_ = checkpoints;
	checkpoint_player_1_ = -1;
	checkpoint_player_2_ = -1;

	// initialisation des meshs pour l'affichage du score
	textures_.resize(2);
	textures_[0] = read_texture(0, "MMachine/data/textures/red_ball.png");
    textures_[1] = read_texture(1, "MMachine/data/textures/blue_ball.png");

    // taille des images
    float height = 20.0;
    float width = height * (float)window_width()/(float)window_height();
    // espacement entre les images
    float espacement = 0.07;
    float left_espacement = 0.01;
    float up_espacement = -0.01;

    meshs_.resize(max_score_);

    for(unsigned int i = 0;i < max_score_;i++) {
    	// construction d'un mesh rectangulaire à la ième place
    	Mesh mesh(GL_TRIANGLES);
    	unsigned int a,b,c,d;

    	// calcul du décalage en hauteur selon la place
    	float height_decalage = - 1 / height * i - espacement * i;

    	// création du mesh
		mesh.texcoord(0,1);
	    a = mesh.vertex(Point(-1 + left_espacement, 1.0 + up_espacement + height_decalage, -1));
	    mesh.texcoord(0, -height);
	    b = mesh.vertex(Point(-1 + left_espacement,-1.0 + up_espacement + height_decalage,-1));
	    mesh.texcoord(width, -height);
	    c = mesh.vertex(Point(1 + left_espacement,-1.0 + up_espacement + height_decalage,-1));
	    mesh.texcoord(width, 1);
	    d = mesh.vertex(Point(1 + left_espacement, 1.0 + up_espacement + height_decalage, -1));    	

	    mesh.triangle(a,b,d);
	    mesh.triangle(d,b,c);

	    meshs_[i] = mesh;
    }
    

}

void ScoreManager::updateCheckpoints(Point player_1_position, Point player_2_position) {
	// axe z à 0, pas de prise en compte de la hauteur
	player_1_position.z = 0;
	player_2_position.z = 0;
	// si le joueur est dans le rayon de son prochain checkpoint, on incrémente son numéro de checkpoint
	// joueur 1
	Checkpoint next = checkpoints_[ (checkpoint_player_1_ + 1) % checkpoints_.size() ];
	if(distance(player_1_position, next.center) < next.radius) {
		checkpoint_player_1_++;
	}
	// joueur 2
	next = checkpoints_[ (checkpoint_player_2_ + 1) % checkpoints_.size() ];
	if(distance(player_2_position, next.center) < next.radius) {
		checkpoint_player_2_++;
	}
}

int ScoreManager::getFirst(Point player_1_position, Point player_2_position) {
	// axe z à 0, pas de prise en compte de la hauteur
	player_1_position.z = 0;
	player_2_position.z = 0;
	// classement selon le nombre de checkpoints validés, maj du score
	if(checkpoint_player_1_ > checkpoint_player_2_) {
		return 0;	// joueur 1 premier
	}else if(checkpoint_player_1_ < checkpoint_player_2_) {
		return 1;	// joueur 2 premier
	}else {	// égalité en nombre de checkpoints
		// on vérifie quel est le joueur le plus proche du checkpoint suivant
		Checkpoint next = checkpoints_[ (checkpoint_player_1_ + 1) % checkpoints_.size() ];
		float distance_player_1 = distance(player_1_position, next.center);
		float distance_player_2 = distance(player_2_position, next.center);
		if(distance_player_1 > distance_player_2) {
			return 1; // joueur 2 gagnant
		} else if(distance_player_1 < distance_player_2){
			return 0; // joueur 1 gagnant
		}else {
			return -1; // pas de gagnant, égalité
		}
	}
}

void ScoreManager::updateScore(int first) {
	switch(first) {
		case 0 : score_player_1_++; round_winner_ = 0; break;
		case 1 : score_player_1_--; round_winner_ = 1; break;
		default : ;
	}
}

int ScoreManager::getRoundWinner() {
	return round_winner_;
}

int ScoreManager::getWinner() {
	if(score_player_1_ == max_score_) {
		return 0;
	}else {
		return 1;
	} 
}

unsigned int ScoreManager::getEcartCheckpoints() {
	return abs(checkpoint_player_1_ - checkpoint_player_2_);
}

void ScoreManager::resetRound() {
	checkpoint_player_1_ = -1;
	checkpoint_player_2_ = -1;
	round_winner_ = -1;
}

bool ScoreManager::end() {
	if(score_player_1_ == 0 || score_player_1_ == max_score_) {
		return true;
	}
	return false;
}

void ScoreManager::draw() {
	DrawParam param;
	unsigned int s = 0;
	// dessin des points du joueur 1
	while(s < score_player_1_) {
		param.texture(textures_[0]);
		param.alpha(0.9);
		param.draw(meshs_[s]);
		s++;
	}
	// dessin des points du joueur 2
	while(s < max_score_) {
		param.texture(textures_[1]);
		param.alpha(0.9);
		param.draw(meshs_[s]);
		s++;
	}
}

void ScoreManager::drawRoundWinner() {
	if(getRoundWinner() == 0) {
		default_color(text, Color(1.0, 0.0, 0.0, 1.0));
		print(text, 50, 0, "Player 1 wins this round !"); 
	}else {
		default_color(text, Color(0.0, 0.0, 1.0, 1.0));
		print(text, 50, 0, "Player 2 wins this round !"); 
	}
	::draw(text, window_width(), window_height()/2);
}

void ScoreManager::drawWinner() {
	if(getWinner() == 0) {
		default_color(text, Color(1.0, 0.0, 0.0, 1.0));
		print(text, 50, 0, "PLAYER 1 IS THE WINNER !!!"); 
	}else {
		default_color(text, Color(0.0, 0.0, 1.0, 1.0));
		print(text, 50, 0, "PLAYER 2 IS THE WINNER !!!"); 
	}
	::draw(text, window_width(), window_height()/2);
}

void ScoreManager::release() { 
    glDeleteTextures(1, &textures_[0]);   
    glDeleteTextures(1, &textures_[1]);
    for(Mesh m : meshs_) {
    	m.release();
    }
	release_text(text);
}