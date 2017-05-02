#ifndef _MMACHINE_PLAYER_HPP_
#define _MMACHINE_PLAYER_HPP_

#include "terrain.hpp"
#include "controller.hpp"

#include "mat.h"
#include "vec.h"

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
     * Retourne la position en x du joueur.
     */
    float get_x();
    /**
     * Retourne la position en y du joueur.
     */
    float get_y();
    
  private :

    //environment
    const Terrain* terrain_ ;
    void project(Point& candidate) ;

    //control
    bool forward_ ;
    bool switchable_ ;
    const Controller* controller_ ;

    //physics
    bool active_ ;
    Vector speed_ ;
    int last_time_ ;
    void step() ;
    void collide();

    //elements
    Point position_ ;
    Vector direction_ ;
    Vector normal_ ;

    //parameters
    float acceleration_ ;
    float turning_angle_ ;
    float max_speed_ ;
    vec2 friction_ ;
} ;

#endif
