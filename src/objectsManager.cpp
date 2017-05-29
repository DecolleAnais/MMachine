#include "objectsManager.hpp"

#include "wavefront.h"
#include "draw.h"
#include "program.h"
#include "uniforms.h"

ObjectsManager::ObjectsManager() {
	Obj obj;
	Point pmin,pmax;
	white_texture_ = read_texture(0, "MMachine/data/textures/White_Texture.png");

	// samplers des textures
    samplers_.resize(2);

	glGenSamplers(1, &samplers_[0]);
        glSamplerParameteri(samplers_[0], GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glSamplerParameteri(samplers_[0], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(samplers_[0], GL_TEXTURE_WRAP_S, GL_REPEAT);
        glSamplerParameteri(samplers_[0], GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenSamplers(1, &samplers_[1]);
        glSamplerParameteri(samplers_[1], GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glSamplerParameteri(samplers_[1], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(samplers_[1], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glSamplerParameteri(samplers_[1], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	// mesh
	obj.mesh = read_mesh("MMachine/data/obj/Rock3.obj");
	// texture
	obj.texture = read_texture(0, "MMachine/data/textures/Rock3_Texture.png");
	obj.transform = Translation(67.0, 15.0, 15.0) * Scale(2.0,2.0,2.0);
	// points de la bounding box en 2D
	(obj.mesh).bounds(pmin, pmax);
	pmin = (obj.transform) (pmin);
	pmax = (obj.transform) (pmax);
	obj.xmin = pmin.x - 0.4;
	obj.xmax = pmax.x + 0.4;
	obj.ymin = pmin.y - 0.4;
	obj.ymax = pmax.y + 0.4;

	objects_.push_back(obj);

	// mesh
	obj.mesh = read_mesh("MMachine/data/obj/Log.obj");
	// texture
	obj.texture = read_texture(0, "MMachine/data/textures/Log_Texture.png");
	obj.transform = Translation(33.0, 99.6, 17.8) * Scale(0.2,0.2,0.2);
	// points de la bounding box en 2D
	(obj.mesh).bounds(pmin, pmax);
	pmin = (obj.transform) (pmin);
	pmax = (obj.transform) (pmax);
	obj.xmin = pmin.x - 0.4;
	obj.xmax = pmax.x + 0.4;
	obj.ymin = pmin.y - 0.4;
	obj.ymax = pmax.y + 0.4;

	objects_.push_back(obj);

	// mesh
	obj.mesh = read_mesh("MMachine/data/obj/Rock4.obj");
	// texture
	obj.texture = read_texture(0, "MMachine/data/textures/Rock4_Texture.png");
	obj.transform = Translation(145.0, 68.0, 16.0);
	// points de la bounding box en 2D
	(obj.mesh).bounds(pmin, pmax);
	pmin = (obj.transform) (pmin);
	pmax = (obj.transform) (pmax);
	obj.xmin = pmin.x - 0.4;
	obj.xmax = pmax.x + 0.4;
	obj.ymin = pmin.y - 0.4;
	obj.ymax = pmax.y + 0.4;

	objects_.push_back(obj);

	//nettoyage
	(obj.mesh).release();
}

// fonction de collision avec les joueurs
bool ObjectsManager::collideWithPlayer(const Point& oldPos, const Vector& oldNorm, Point& newPos, Vector& newNorm) {
	for(Obj o : objects_) {
		// si collision détectée, le joueur reste à sa position initiale
		if(newPos.x > o.xmin && newPos.x < o.xmax && newPos.y > o.ymin && newPos.y < o.ymax) {
			newPos = oldPos;
			newNorm = oldNorm;
			return true;
		}
	}
	return false;
}

void ObjectsManager::draw(const GLuint program, Transform view, Transform proj) {
	// texture blanche pour que les textures des objets soient visibles
	program_use_texture(program, "texture0", 0, white_texture_, samplers_[0]);
	for(Obj o : objects_) {
        // texture de l'objet
        program_use_texture(program, "texture1", 1, o.texture, samplers_[1]);
        
        Transform mv = view * o.transform;
		Transform mvp = proj * view * o.transform;
		// paramètres du program
		program_uniform(program, "mvMatrix", mv);        
		program_uniform(program, "normalMatrix", mv.normal());        
		program_uniform(program, "mvpMatrix", mvp);
		program_uniform(program, "modelMatrix", o.transform);
		program_uniform(program, "viewMatrix", view);
		program_uniform(program, "viewInvMatrix", view.inverse());

		(o.mesh).draw(program);
	}
}

void ObjectsManager::release() {
	glDeleteTextures(1, &white_texture_);
	for(Obj o : objects_) {
		(o.mesh).release();
		glDeleteTextures(1, &(o.texture));
	}
}