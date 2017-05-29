#ifndef _MMACHINE_TERRAIN_HPP_
#define _MMACHINE_TERRAIN_HPP_

/**
 * \file terrain.hpp
 * \brief Gestion du terrain
 */

#include "vec.h"
#include "mat.h"
#include "mesh.h"
#include "image.h"
#include "orbiter.h"

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
    virtual void project(const Point& from, Point& to, Vector& n) const = 0 ;

    /**
     * Fonction d'affichage du terrain
     * @param v la matrice view de la camera
     * @param p la matrice de projection de la camera
     */
    virtual void draw(const Transform& v, const Transform& p) = 0 ;


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

class GeneratedTerrain : public Terrain {
    public :
        GeneratedTerrain(const Point& pmin, const Point& pmax) ;
        void smooth(std::vector< std::vector< Vector > >& vVertexData, const unsigned int iterations) ;
        void project(const Point& from, Point& to, Vector& n) const ;
        void draw(const Transform& v, const Transform& p) ;
        void draw(const GLuint& shaders_program, Transform model, Transform view, Transform proj) ;
        void setCheckpoints(Transform transform) ;
        void drawCheckpoints(Transform model, Transform view, Transform proj);
        std::vector<Checkpoint> getCheckpoints() const;

        void release() ;

    private :
        Mesh mesh_ ;
        unsigned int height;
        unsigned int width;
        bool collideWithTriangleGird(Point pos, int ia, int ib, int ic);
        float getHeight(Point pos, int ia, int ib, int ic);
        Vector getNormal(Point pos, int ia, int ib, int ic);
        unsigned int step; // un point sur step est pris en compte dans la génération du terrain (depuis l'image png)
        std::vector<Checkpoint> checkpoints_;
        std::vector<Mesh> meshs_checkpoints_;
} ;


#endif
