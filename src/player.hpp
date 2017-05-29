#ifndef _MMACHINE_PLAYER_HPP_
#define _MMACHINE_PLAYER_HPP_

#include "terrain.hpp"
#include "controller.hpp"
#include "objectsManager.hpp"

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
    /**
     * Set le conteneur d'objets pour les collisions
     */
    void setObjectsManager(ObjectsManager* objectsManager);
    /**
     * Retourne la liste des coins de la boite englobante du joueur
     */
    std::vector<Point> getCornerPoints();  
    /**
     * Update du joueur. Met a jour sa position, vitesse, direction, normale, et gère les collisions. 
     */
    void step() ;
    /**
     * True si le joueur a glissé trop longtemps dans une pente (considéré alors comme une chute). 
     */ 
    bool isFallen();
    /**
     * Met à jour la distance parcouru en glissade. 
     * Si le cumul de la distance passé en paramètre et celle stockée est trop grand, le joueur est considéré comme tombé. La variable fallen_ est set a True.
     * Si la valeur passée en paramètre est égale ou inférieure à 0, alors le joueur ne glisse ni ne tombe. la distance de chute cumulée est reset.
     */ 
    void setCurrentFallingDist(float currentFallingDist);
    
  private :

    //environment
    const Terrain* terrain_ ;
    void project(Point& candidate) ;
    Player* playerToCollide_;
    ObjectsManager* objectsManager_;

    //control
    bool forward_ ;
    bool switchable_ ;
    const Controller* controller_ ;

    //physics
    bool active_ ;
    Vector speed_ ;
    int last_time_ ;
    /**
     * Fonction de collision générale. Met à jour newPos et newDir en fonction des différentes collisions.
     */
    bool collide(const Point& oldPos, const Vector& oldNorm, Point& newPos, Vector& newNorm);
    /**
     * Gestion de la collision avec le terrain. Cette détection stoppe le joueur s'il essaye de gravir une pente trop raide.
     */
    bool collideWithTerrain(const Point& oldPos, const Vector& oldNorm, Point& newPos, Vector& newNorm);
    /**
     * Gestion de la collision entre joueurs. Cette détection calcule la pénétration du joueur adverse dans le véhicule et applique un knockback s'il y a collision.
     */
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
