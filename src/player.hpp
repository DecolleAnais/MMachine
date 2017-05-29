#ifndef _MMACHINE_PLAYER_HPP_
#define _MMACHINE_PLAYER_HPP_

#include "terrain.hpp"
#include "controller.hpp"

#include "mat.h"
#include "vec.h"

class Terrain;

class Player {
  public :
    Player() ;

    /**
     * Placement initial du véhicule.
     * @param position la position ou placer le vehicule
     * @param direction la direction vers laquelle se dirige le vehicule
     */
    void spawn_at(const Point& position, const Vector& direction) ;

    /**
     * Statut du vehicule
     */
    bool active() ;


    /**
     * Activation du vehicule.
     */
    void activate() ;

    /**
     * Desactivation du vehicule.
     */
    void deactivate() ;

    /**
     * Matrice de transformation a utiliser pour l'affichage du vehicule
     */
    Transform transform() ;

    //parameters
    /**
     * Terrain a fournir sur lequel le vehicule se deplace.
     */
    void set_terrain(const Terrain* terrain) ;
    /**
     * Controles pour gerer le deplacement du vehicule
     */
    void set_controller(const Controller* controller) ;
    /**
     * Boite englobante pour gerer les collisions
     */
    void set_bounding_box(const Point p1, const Point p2);
    /**
     * Retourne la position en x du joueur.
     */
    float get_x();
    /**
     * Retourne la position en y du joueur.
     */
    float get_y();
    /**
     * Retourne la position en z du joueur.
     */
    float get_z();
    /**
     * Retourne la direction du joueur.
     */
    Vector getDirection();
    /**
     * Retourne la position du joueur.
     */
    Point getPosition();
    /**
     * Set l'autre joueur pour les collisions
     */
    void setOtherPlayer(Player& player);  
    std::vector<Point> getCornerPoints();  
    void step() ;
    bool isFallen();
    void setCurrentFallingDist(float currentFallingDist);
    
  private :

    //environment
    const Terrain* terrain_ ;
    void project(Point& candidate) ;
    Player* playerToCollide_;

    //control
    bool forward_ ;
    bool switchable_ ;
    const Controller* controller_ ;

    //physics
    bool active_ ;
    Vector speed_ ;
    int last_time_ ;
    bool collide(const Point& oldPos, const Vector& oldNorm, Point& newPos, Vector& newNorm);
    bool collideWithTerrain(const Point& oldPos, const Vector& oldNorm, Point& newPos, Vector& newNorm);
    bool collideWithPlayer(const Point& oldPos, const Vector& oldNorm, Point& newPos, Vector& newNorm);

    //elements
    Point position_ ;
    Vector direction_ ;
    Vector normal_ ;
    float sizeX_, sizeY_, sizeZ_;

    //parameters
    float acceleration_ ;
    float turning_angle_ ;
    float max_speed_ ;
    vec2 friction_ ;

    // falling
    bool fallen_;
    float fallingDist_;
} ;

#endif
