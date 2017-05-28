#ifndef _MMACHINE_SCORE_MANAGER_HPP_
#define _MMACHINE_SCORE_MANAGER_HPP_

/**
 * \file scoreManager.hpp
 * \brief Gestion du score
 */

#include "terrain.hpp"

#include "mesh.h"
#include "texture.h"
#include "text.h"

/**
 * \class ScoreManager
 * \brief Gestion du score
 */
class ScoreManager {
  public :
  	/**
  	 * \brief Constructeur
  	 * Initialise le score maximum, le score du joueur 1, la liste des checkpoints, le gagnant de la manche et le texte à afficher.
  	 * Crée des meshs pour l'affichage du score.
  	 * \param max_score : score maximum à atteindre pour finir le jeu
  	 * \param score_player_1 : score du joueur 1 (le score du joueur 2 est déduit de la différence avec max_score)
  	 * \param checkpoints : liste des checkpoints à passer dans l'ordre
  	 */
  	ScoreManager(const unsigned int max_score, unsigned int score_player_1, std::vector<Checkpoint> checkpoints);
  	/**
  	 * \brief Joueur en première position
  	 * Calcule qui est premier entre les joueurs par rapport aux checkpoints validés et la distance au prochain checkpoint
  	 * \return -1 si égalité, 0 si le joueur 1 est premier, 1 si le joueur 2 est premier
  	 */
  	int getFirst(Point player_1_position, Point player_2_position);
  	/**
  	 * \brief Mise à jour des checkpoints validés
  	 * Incrémente le nombre de checkpoints validés par les joueurs si la position du joueur est dans le rayon de son prochain checkpoint (distance en 2D uniquement)
  	 * \param player_1_position : position du joueur 1
  	 * \param player_2_position : position du joueur 2
  	 */
  	void updateCheckpoints(Point player_1_position, Point player_2_position);
  	/**
  	 * \brief Mise à jour du score
  	 * Incrémente le score du joueur passé en paramètre et le désigne gagnant du round
  	 * \param first : numéro du joueur en première position
  	 */
  	void updateScore(int first);
  	/**
  	 * \brief Gagnant de la manche
  	 * \return le numéro du gagnant de la manche
  	 */
  	int getRoundWinner();
  	/**
  	 * \brief Gagnant du jeu
  	 * \return le numéro du gagnant du jeu qui a le score max
  	 */
  	int getWinner();
  	/**
  	 * \brief Nombre de checkpoints d'écart entre les 2 joueurs
  	 * Permet de détecter un trop grand écart dans le nombre de checkpoints validés
  	 * \return le nombre de checkpoints validés d'écart entre les joueurs
  	 */
  	unsigned int getEcartCheckpoints();
  	/**
  	 * \brief Remise à zéro des paramètres de la manche
  	 * Réinitialise le gagnant de la manche et le nombre de checkpoints validés des joueurs
  	 */
  	void resetRound();
  	/**
  	 * \brief Fin du jeu
  	 * Détecte si le score max a été atteint par un des joueurs
  	 * \return true si c'est la fin du jeu, false sinon
  	 */
  	bool end();
  	/**
  	 * \brief Dessin du score
  	 */
  	void draw();
  	/**
  	 * \brief Ecriture du gagnant de la manche
  	 */
  	void drawRoundWinner();
  	/**
  	 * \brief Ecriture du gagnant du jeu
  	 */
  	void drawWinner();
  	/**
  	 * Libération des ressources
  	 */
    void release();

private:
	// display
	std::vector<Mesh> meshs_;				/**< vector contenant les meshs pour l'affichage du score */
    std::vector<GLuint> textures_;			/**< vector contenant les textures pour l'affichage du score */
    // score storage
    unsigned int score_player_1_;			/**< score du joueur 1 */
    unsigned int max_score_;				/**< score maximum atteignable */
    // checkpoints to pass
    std::vector<Checkpoint> checkpoints_;	/**< liste des checkpoints */
    int checkpoint_player_1_;				/**< nombre de checkpoints validés par le joueur 1 */
    int checkpoint_player_2_;				/**< nombre de checkpoints validés par le joueur 2 */
    // winner
    int round_winner_;						/**< numéro du joueur gagnant de la manche */
    // texte de score affiché
    Text text;								/**< texte à afficher en cas de victoire de la manche ou du jeu */
} ;

#endif
