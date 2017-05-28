#ifndef _MMACHINE_PARSER_HPP_
#define _MMACHINE_PARSER_HPP_

/**
 * \file parser.h
 * \brief Parser de fichiers contenant les listes de points des checkpoints du terrain
 */

#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>

/**
 * \class Parser
 * \brief Parse les fichiers contenant des coordoonées de points et les stocke dans un vector
 */
class Parser {
public:
	/**
	 * \brief Constructeur
	 * Lit le fichier passé en paramètre et stocke ses données dans le vector text
	 * \param file : fichier à parser
	 */
	Parser(char* file);
	/**
	 * \brief Nombre de lignes du fichier parsé
	 * \return le nombre de lignes parsées
	 */
	unsigned int getNbLines();
	/**
	 * \brief Récupère un élément du vector text contenant les données parsées
	 * \return la coordonnée stockée sous forme de float
	 * \param line : numéro de ligne de l'élément
	 * \param column : numéro de colonne de l'élément
	 */
	float get(unsigned int line, unsigned int column);


private:
	const int MAX_CHARS_PER_LINE = 10;			/**< Maximum de caractères lus par ligne */
	const int MAX_TOKENS_PER_LINE = 2;			/**< Maximum de mots lus par ligne */
	const char* const DELIMITER = " ";			/**< Séparateur de mots */
	std::vector< std::vector< float > > text_; 	/**< Vector text 2D contenant les éléments parsés */
};

#endif