#ifndef _MMACHINE_TERRAIN_HPP_
#define _MMACHINE_TERRAIN_HPP_

/**
 * \file terrain.hpp
 * \brief Gestion du terrain
 */

#include "player.hpp"

#include "vec.h"
#include "mat.h"
#include "mesh.h"
#include "image.h"
#include "orbiter.h"

class Player;

/**
 * \class Terrain
 * \brief Gestion du terrain
 * Génère un terrain depuis une carte de hauteur, crée les checkpoints, dessine le terrain et gère les projections sur le terrain
 */
class Terrain {
  public :
    /** 
     * Fonction de projection sur le terrain
     * @param from le point d'ou provient le vehicule
     * @param to le point ou souhaite se rendre le vehicule
     * @param n la normale du vehicule
     *
     * Cette fonction doit modifier le point to et la normale du vehicule, pour
     * fournir au programme appelant la position a laquelle le vehicule est
     * parvenu, et la normale du terrain a ce point.
     */
    void project(const Point& from, Point& to, Vector& n) const ;

    /**
     * Fonction d'affichage du terrain
     * @param v la matrice view de la camera
     * @param p la matrice de projection de la camera
     */
    void draw(const Transform& v, const Transform& p) ;


} ;

/**
 * \class FlatTerrain
 * \brief Terrain plat de base
 */
class FlatTerrain : public Terrain {
  public :
    /**
     * \brief Constructeur
     * Génère un terrain plat allant de pmin à pmax
     * \param pmin : point minimum du terrain
     * \param pmax : point max du terrain
     */
    FlatTerrain(const Point& pmin, const Point& pmax) ;
    void project(const Point& from, Point& to, Vector& n) const ;
    void draw(const Transform& v, const Transform& p) ;

  private :
    Mesh mesh_ ; /**< Mesh du terrain */
} ;

/**
 * \struct Checkpoint
 * \brief Stocke les données d'un checkpoint 
 * Stocke la position du centre, la position d'un point du rayon, le rayon
 */
struct Checkpoint {
    Point center;       /**< Point au centre du checkpoint */
    Point radius_point; /**< Point sur le rayon */
    float radius;       /**< Rayon, calculé grâce aux 2 points */
};

/**
 * \class GeneratedTerrain
 * \brief Gestion du terrain généré à partir d'une carte de hauteur
 */
class GeneratedTerrain : public Terrain {
    public :
        /**
         * \brief Génération du terrain
         * Génère le terrain en lisant une image png en niveau de gris (carte de hauteurs).
         * Normalise et agrandit le terrain.
         * Crée le mesh du terrain et un mesh pour le socle sous le terrain (nécessaire pour les effets de lumière du shader).
         * Applique la fonction smooth au terrain pour le lisser.
         * \param pmin : point minimum du terrain (inutilisé pour l'instant)
         * \param pmax : point maximum du terrain (inutilisé pour l'instant)
         */
        GeneratedTerrain(const Point& pmin, const Point& pmax) ;
        /**
         * \brief Lissage du terrain
         * Pour chaque sommet, fait une moyenne de la hauteur avec les sommets voisins (carré de 9x9).
         * \param iterations : nombre d'itérations à effectuer
         */
        void smooth(std::vector< std::vector< Vector > >& vVertexData, const unsigned int iterations) ;
        /**
         * \brief Projette le joueur sur le terrain. 
         * \brief Note : !!! DEPRECATED !!! Utiliser project(const Point& from, Point& to, Vector& n, Player* player)
         */
        void project(const Point& from, Point& to, Vector& n) const ;
        /**
         * \brief Projette le joueur sur le terrain. 
         * \param from : Point d'origine
         * \param to : Point désiré de destination, modifié par la projection
         * \param n : Normale désirée, modifiée par la projection
         * \param player : Joueur subissant la projection
         */
        void project(const Point& from, Point& to, Vector& n, Player* player) const ;
        /**
         * \brief Dessin de base du terrain
         * \param v : matrice view
         * \param p : matrice projection
         */
        void draw(const Transform& v, const Transform& p) ;
        /**
         * \brief Dessin du terrain via un shader
         * \param shaders_program : shader à utiliser
         * \param model : matrice model
         * \param view : matrice view
         * \param proj : matrice projection
         */
        void draw(const GLuint& shaders_program, Transform model, Transform view, Transform proj) ;
        /**
         * \brief Dessine le socle du terrain via un shader
         * \param shaders_program : shader à utiliser
         * \param model : matrice model
         * \param view : matrice view
         * \param proj : matrice projection
         */
        void drawUnderBox(const GLuint& shaders_program, Transform model, Transform view, Transform proj) ;
        /**
         * \brief Initialise les checkpoints du terrain
         * Lit deux fichiers : un avec les coordonnées du centre des checkpoints, l'autre avec un second point sur le rayon max du checkpoint.
         * Normalise les positions des points et applique la transformation.
         * Déduit le rayon du checkpoint et l'ajoute à la liste de checkpoints.
         * \param transform : transformation à effectuer sur les checkpoints
         */
        void setCheckpoints(Transform transform) ;
        /**
         * \brief Checkpoints
         * \return un vector contenant les checkpoints
         */
        std::vector<Checkpoint> getCheckpoints() const;
        /**
         * \brief Libération des ressources
         */
        void release() ;

    private :
        /**
         * \brief Fonction de détection de collision d'un point avec un triangle, sans considération de l'axe z (projection sur le plan xy).
         * \param pos : Point à tester.
         * \param ia : indice du premier point du triangle
         * \param ib : indice du second point du triangle
         * \param ic : indice du troisième point du triangle
         * \return true si le point est dans le triangle, false sinon
         */
        bool collideWithTriangleGird(Point pos, int ia, int ib, int ic);
        /**
         * Retourne la hauteur d'un point dans un triangle en fonction de ses coordonnées xy.
         * \param pos : Point à tester.
         * \param ia : indice du premier point du triangle
         * \param ib : indice du second point du triangle
         * \param ic : indice du troisième point du triangle
         * \return moyenne pondérée de la hauteur des vertex (hauteur exacte du point)
         */
        float getHeight(Point pos, int ia, int ib, int ic);
        /**
         * Retourne la normale d'un point dans un triangle en fonction de ses coordonnées xy.
         * \param pos : Point à tester.
         * \param ia : indice du premier point du triangle
         * \param ib : indice du second point du triangle
         * \param ic : indice du troisième point du triangle
         * \return moyenne pondérée de la normale des vertex (normale exacte du point)
         */
        Vector getNormal(Point pos, int ia, int ib, int ic);

        Mesh mesh_ ;                            /**< Mesh du terrain */
        Mesh underBox_;                         /**< Mesh du socle du terrain */
        unsigned int height;                    /**< Hauteur/longueur du terrain */
        unsigned int width;                     /**< Largeur du terrain */
        // un point sur step est pris en compte dans la génération du terrain (depuis l'image png)
        unsigned int step;                      /**< Pas utilisé dans la lecture des pixels de l'image png */
        std::vector<Checkpoint> checkpoints_;   /**< Liste des checkpoints */
} ;


#endif
