#ifndef _MMACHINE_PNG_UTILITIES_HPP_
#define _MMACHINE_PNG_UTILITIES_HPP_

/**
 * \file pngUtilities.hpp
 * \brief Permet de lire une image png et de récupérer ses valeurs
 */

#include "png.h"
#include "zlib.h"
#include <string>

/**
 * \class PngUtilities
 * \brief Lit une image png et stocke ses valeurs
 */
class PngUtilities {
	public :
		/**
		 * \brief Initialisation du lecteur de png
		 * Ouvre le fichier, récupère ses données (largeur, hauteur, valeurs) et les stocke dans un tableau
		 * \param path : fichier à lire
		 */
		void init(const char* path);
		/**
		 * \brief Fermeture du fichier en lecture
		 */
		void free();
		/**
		 * \brief Affichage sur la sortie standard des données lues pour chaque pixel de l'image
		 */
		void readRows() const;
		/**
		 * \brief Valeur du pixel
		 * \return la valeur du pixel aux coordoonées x,y
		 * \param x : coordonnée en x
		 * \param y : coordonnée en y
		 */
		unsigned int getValue(const unsigned int x, const unsigned int y) const;
		/**
		 * \brief Hauteur de l'image
		 * \return la hauteur de l'image
		 */
		unsigned int getHeight() const;
		/**
		 * \brief Largeur de l'image
		 * \return la largeur de l'image
		 */
		unsigned int getWidth() const;
		/**
		 * \brief Création d'un string contenant les paramètres de l'image
		 * Ecrit la largeur, la hauteur, le format en couleur et en bits de l'image
		 * \return un string contenant les informations des paramètres de l'image
		 */ 
		std::string toString() const;

	private :
		FILE *file;				/**< Fichier à lire */
		unsigned int width;		/**< Largeur de l'image */
		unsigned int height;	/**< Hauteur de l'image */
		png_byte color_type;	/**< Format de la couleur de l'image */
		png_byte bit_depth;		/**< Formats en bits de l'image */
		png_bytep *rows;		/**< Pointeur sur le tableau contenant les données lues */
};

#endif