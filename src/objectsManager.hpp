#ifndef _MMACHINE_OBJECTS_MANAGER_HPP_
#define _MMACHINE_OBJECTS_MANAGER_HPP_

/**
 * \file objectsManager.hpp
 * \brief Gère les objets ajoutés sur le terrain et la collision avec les joueurs
 */
#include "mesh.h"
#include "mat.h"
#include "texture.h"

/**
 * \struct Obj
 * \brief Données d'un objet
 * Contient le mesh, la texture, le transform de l'objet et les points de la boîte englobante (2D)
 */
struct Obj {
	Mesh mesh;							/**< Mesh */
	GLuint texture;						/**< Texture */
	Transform transform;				/**< Transform appliqué à l'objet */
	float xmin;
	float xmax;
	float ymin;
	float ymax;
};

/**
 * \class ObjectsManager
 * \brief Gère les objets différents des joueurs, ajoutés sur le terrain
 * Crée les objets, leurs boîtes englobantes, les dessine
 */
class ObjectsManager {
public:
	/**
	 * \brief Constructeur
	 * Crée des objets avec leur mesh, texture et transformation
	 */
	ObjectsManager();
	/**
	 * \brief Gère les collisions avec le joueur
	 * S'il y a collision la position et la normale ne changent pas, sinon on met à jour avec les nouvelles position et normale
	 * \return true si collision, false sinon
	 * \param oldPos : ancienne position du joueur
	 * \param oldNorm : ancienne normale du joueur
	 * \param newPos : nouvelle position du joueur
	 * \param newNorm : nouvelle normale du joueur
	 */
	bool collideWithPlayer(const Point& oldPos, const Vector& oldNorm, Point& newPos, Vector& newNorm);
	/**
	 * \brief Dessin des objets
	 * Dessine tous les objets
	 * \param view : matrice vue
	 * \param projection : matrice projection
	 */
	void draw(const GLuint program, Transform view, Transform proj);
	/**
	 * \brief Libération des ressources
	 */
	void release();
private:
	std::vector<Obj> objects_;		/**< vector contenant tous les objets (Obj) */
	GLuint white_texture_;			/**< texture blanche nécessaire pour le dessin des objets */
	std::vector<GLuint> samplers_;	/**< samplers pour le dessin des textures */
};

#endif