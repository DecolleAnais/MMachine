#include "scoreDisplay.hpp"

#include "draw.h"
#include "window.h"

ScoreDisplay::ScoreDisplay(const unsigned int max_score) {
	textures_.resize(2);
	textures_[0] = read_texture(0, "MMachine/textures/red_ball.png");
    textures_[1] = read_texture(1, "MMachine/textures/blue_ball.png");

    // taille des images
    float height = 20.0;
    float width = height * (float)window_width()/(float)window_height();
    // espacement entre les images
    float espacement = 0.07;
    float left_espacement = 0.01;
    float up_espacement = -0.01;

    meshs_.resize(max_score);

    for(unsigned int i = 0;i < meshs_.size();i++) {
    	// construction d'un mesh rectangulaire à la ième place
    	Mesh mesh(GL_TRIANGLES);
    	unsigned int a,b,c,d;

    	// calcul du décalage en hauteur selon la place
    	float height_decalage = - 1 / height * i - espacement * i;

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

void ScoreDisplay::draw(const unsigned int score) {
	DrawParam param;
	unsigned int s = 0;
	while(s < score) {
		param.texture(textures_[0]);
		param.alpha(0.9);
		param.draw(meshs_[s]);
		s++;
	}
	while(s < meshs_.size()) {
		param.texture(textures_[1]);
		param.alpha(0.9);
		param.draw(meshs_[s]);
		s++;
	}
}

void ScoreDisplay::release() { 
    glDeleteTextures(1, &textures_[0]);   
    glDeleteTextures(1, &textures_[1]);
    for(Mesh m : meshs_) {
    	m.release();
    }
}